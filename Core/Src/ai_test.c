/**
  ******************************************************************************
  * @file    ai_test.c
  * @brief   Non-blocking Analog Inputs 1..4 test engine for Inputs_Screen.
  ******************************************************************************
  */

#include "ai_test.h"
#include "eol_format.h"
#include "eol_report.h"
#include "gpio_config.h"
#include "uart_config.h"
#include "stm32u5xx_hal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ======================================================================== */
/*  Timing / Limits                                                          */
/* ======================================================================== */
#define AI_TEST_CHANNEL_COUNT           4u
#define AI_TEST_LEVEL_COUNT             3u
#define AI_TEST_PRE_ENABLE_SETTLE_MS   50u
#define AI_TEST_OUTPUT_SETTLE_MS      250u
#define AI_TEST_REPLY_TIMEOUT_MS     3000u
#define AI_TEST_TX_TIMEOUT_MS         100u
#define AI_TEST_MAX_ATTEMPTS            2u /* initial + 1 retry */
#define AI_TEST_RX_BUF_LEN             48u
#define AI_TEST_RX_QUIET_MS            10u
#define AI_TEST_NEAR_ZERO_CLAMP_V   0.015f

typedef enum
{
    AI_SUB_IDLE = 0,
    AI_SUB_PREP_POINT,
    AI_SUB_WAIT_PRE_ENABLE_SETTLE,
    AI_SUB_ENABLE_OUTPUT,
    AI_SUB_WAIT_OUTPUT_SETTLE,
    AI_SUB_SEND_SAMPLE_REQ,
    AI_SUB_WAIT_REPLY,
    AI_SUB_EVALUATE,
    AI_SUB_DONE
} ai_sub_state_t;

typedef enum
{
    AI_REPLY_PARSE_PENDING = 0,
    AI_REPLY_PARSE_VALID,
    AI_REPLY_PARSE_INVALID
} ai_reply_parse_status_t;

typedef struct
{
    ai_sub_state_t         subState;
    uint8_t                channel; /* 1..4 */
    ai_test_level_t        level;
    uint8_t                attempt; /* 1..AI_TEST_MAX_ATTEMPTS */
    uint32_t               timestamp;
    uint32_t               lastRxByteTick;

    char                   rxBuf[AI_TEST_RX_BUF_LEN];
    uint8_t                rxLen;
    bool                   txError;
    bool                   replyTimedOut;
    ai_reply_parse_status_t replyParseStatus;
    float                  parsedAdcVolts;

    ai_test_progress_t     progress;
    bool                   hasProgress;

    ai_test_result_t       result;
    bool                   hasResult;
} ai_test_ctx_t;

static ai_test_ctx_t ctx;

/* ======================================================================== */
/*  Tables / conversions                                                     */
/* ======================================================================== */

static const uint16_t s_dacSliderByLevelCurrent[AI_TEST_LEVEL_COUNT] = { 62u, 196u, 330u };
static const uint16_t s_dacSliderByLevelVoltage[AI_TEST_LEVEL_COUNT] = { 0u, 165u, 330u };
static const char * const s_levelName[AI_TEST_LEVEL_COUNT] = { "LOW", "MID", "HIGH" };
static const char * const s_levelNameDisplay[AI_TEST_LEVEL_COUNT] = { "Low", "Mid", "High" };

static ai_test_channel_kind_t channelKind(uint8_t channel)
{
    return (channel <= 2u) ? AI_TEST_KIND_CURRENT : AI_TEST_KIND_VOLTAGE;
}

static float expectedEngineering(ai_test_channel_kind_t kind, ai_test_level_t level)
{
    if (kind == AI_TEST_KIND_CURRENT)
    {
        static const float currentExpected[AI_TEST_LEVEL_COUNT] = { 4.0f, 12.0f, 20.0f };
        return currentExpected[(uint8_t)level];
    }
    else
    {
        static const float voltageExpected[AI_TEST_LEVEL_COUNT] = { 0.0f, 5.0f, 10.0f };
        return voltageExpected[(uint8_t)level];
    }
}

static float toleranceEngineering(ai_test_channel_kind_t kind)
{
    return (kind == AI_TEST_KIND_CURRENT) ? 2.0f : 1.0f;
}

static float adcToEngineering(ai_test_channel_kind_t kind, float adcVolts)
{
    if (kind == AI_TEST_KIND_CURRENT)
    {
        /* Effective 150 ohm calibration from measured fixture behavior. */
        return adcVolts * (1000.0f / 150.0f);
    }
    else
    {
        /* Divider 8.2k / 2.0k => Vdut = Vadc * ((8.2 + 2.0)/2.0) = Vadc * 5.1 */
        return adcVolts * 5.1f;
    }
}

static uint16_t sliderSetpointForChannel(uint8_t channel, ai_test_level_t level)
{
    const uint8_t levelIndex = (uint8_t)level;

    if (channelKind(channel) == AI_TEST_KIND_CURRENT)
    {
        return s_dacSliderByLevelCurrent[levelIndex];
    }

    return s_dacSliderByLevelVoltage[levelIndex];
}

static void buildAnalogReportName(uint8_t channel, ai_test_level_t level, char* buf, size_t len)
{
    (void)snprintf(buf, len, "Analog Input %u - %s",
                   (unsigned)channel,
                   s_levelNameDisplay[(uint8_t)level]);
}

static void buildEngineeringExpected(ai_test_channel_kind_t kind,
                                     float expectedEng,
                                     float tolEng,
                                     char* buf,
                                     size_t len)
{
    (void)EOL_FormatAnalogExpected(kind == AI_TEST_KIND_CURRENT, expectedEng, tolEng, buf, len);
}

static void buildEngineeringActual(ai_test_channel_kind_t kind,
                                   float measuredEng,
                                   char* buf,
                                   size_t len)
{
    (void)EOL_FormatAnalogActual(kind == AI_TEST_KIND_CURRENT, measuredEng, buf, len);
}

static void recordAnalogPass(uint8_t channel,
                             ai_test_level_t level,
                             ai_test_channel_kind_t kind,
                             float measuredEng,
                             float expectedEng,
                             float tolEng)
{
    char testName[24];
    char expectedText[24];
    char actualText[16];

    buildAnalogReportName(channel, level, testName, sizeof(testName));
    buildEngineeringExpected(kind, expectedEng, tolEng, expectedText, sizeof(expectedText));
    buildEngineeringActual(kind, measuredEng, actualText, sizeof(actualText));
    (void)EOL_Report_AddRow(EOL_REPORT_ROW_PASS, testName, expectedText, actualText);
}

static void recordAnalogFail(uint8_t channel,
                             ai_test_level_t level,
                             ai_test_channel_kind_t kind,
                             ai_test_failure_kind_t failKind,
                             float measuredEng,
                             float expectedEng,
                             float tolEng)
{
    char testName[24];
    char expectedText[24];
    char actualText[16];

    buildAnalogReportName(channel, level, testName, sizeof(testName));
    buildEngineeringExpected(kind, expectedEng, tolEng, expectedText, sizeof(expectedText));

    if (failKind == AI_TEST_FAIL_OUT_OF_RANGE)
    {
        buildEngineeringActual(kind, measuredEng, actualText, sizeof(actualText));
    }
    else if (failKind == AI_TEST_FAIL_NO_RESPONSE)
    {
        (void)snprintf(actualText, sizeof(actualText), "No response");
    }
    else
    {
        (void)snprintf(actualText, sizeof(actualText), "Invalid reply");
    }

    (void)EOL_Report_AddRow(EOL_REPORT_ROW_FAIL, testName, expectedText, actualText);
    EOL_Report_FinalizeFail();
}

/* ======================================================================== */
/*  Helpers                                                                  */
/* ======================================================================== */

static void safeAnalogIdle(void)
{
    GPIO_Config_WriteDacEn(GPIO_PIN_RESET);
    DAC_SetFromSlider(0u);
}

static void postProgress(uint8_t channelPassed)
{
    ctx.progress.channelPassed = channelPassed;
    ctx.hasProgress = true;
}

static void postResultPass(void)
{
    memset(&ctx.result, 0, sizeof(ctx.result));
    ctx.result.passed = true;
    ctx.result.errorCode = EOL_ERR_NONE;
    ctx.result.failureKind = AI_TEST_FAIL_NONE;
    ctx.hasResult = true;
}

static void postResultFail(ai_test_failure_kind_t failKind, eol_error_code_t errorCode,
                           float measuredAdcVolts, float measuredEngineering,
                           float expectedEng, float tolEng)
{
    memset(&ctx.result, 0, sizeof(ctx.result));
    ctx.result.passed = false;
    ctx.result.failedChannel = ctx.channel;
    ctx.result.failedLevel = ctx.level;
    ctx.result.kind = channelKind(ctx.channel);
    ctx.result.failureKind = failKind;
    ctx.result.errorCode = errorCode;
    ctx.result.measuredAdcVolts = measuredAdcVolts;
    ctx.result.measuredEngineering = measuredEngineering;
    ctx.result.expectedEngineering = expectedEng;
    ctx.result.toleranceEngineering = tolEng;
    ctx.hasResult = true;
}

static void UART4_ClearRxStaleState(void)
{
    UART4_ClearRxBuffer();
    __HAL_UART_CLEAR_FLAG(&huart4, UART_CLEAR_PEF | UART_CLEAR_FEF |
                                   UART_CLEAR_NEF | UART_CLEAR_OREF);
    __HAL_UART_FLUSH_DRREGISTER(&huart4);
    huart4.ErrorCode = HAL_UART_ERROR_NONE;
}

static void resetReplyContext(void)
{
    ctx.rxLen = 0u;
    memset(ctx.rxBuf, 0, sizeof(ctx.rxBuf));
    ctx.txError = false;
    ctx.replyTimedOut = false;
    ctx.replyParseStatus = AI_REPLY_PARSE_PENDING;
    ctx.parsedAdcVolts = 0.0f;
    ctx.lastRxByteTick = HAL_GetTick();
}

static void buildSampleReqToken(uint8_t channel, ai_test_level_t level, char *buf, size_t len)
{
    (void)snprintf(buf, len, "AI%u_%s_REQ\n", (unsigned)channel, s_levelName[(uint8_t)level]);
}

static void buildSampleReplyPrefix(uint8_t channel, ai_test_level_t level, char *buf, size_t len)
{
    (void)snprintf(buf, len, "OK_AI%u_%s:", (unsigned)channel, s_levelName[(uint8_t)level]);
}

static void pollReplyBytes(void)
{
    uint8_t byteIn = 0u;

    while (ctx.rxLen < (AI_TEST_RX_BUF_LEN - 1u))
    {
        if (!UART4_ReadByte(&byteIn))
        {
            break;
        }

        if ((byteIn == '\r') || (byteIn == '\n') || (byteIn == '\t') || (byteIn == ' '))
        {
            continue;
        }

        if ((byteIn < 32u) || (byteIn > 126u))
        {
            ctx.replyParseStatus = AI_REPLY_PARSE_INVALID;
            break;
        }

        ctx.rxBuf[ctx.rxLen++] = (char)byteIn;
        ctx.rxBuf[ctx.rxLen] = '\0';
        ctx.lastRxByteTick = HAL_GetTick();
    }
}

static ai_reply_parse_status_t tryParseCurrentReply(float *adcVoltsOut)
{
    char prefix[20];
    const char *valueStr;
    char *endPtr;
    float volts;
    size_t prefixLen;
    size_t valueLen;

    buildSampleReplyPrefix(ctx.channel, ctx.level, prefix, sizeof(prefix));
    prefixLen = strlen(prefix);

    if (ctx.rxLen < prefixLen)
    {
        if (memcmp(ctx.rxBuf, prefix, ctx.rxLen) == 0)
        {
            return AI_REPLY_PARSE_PENDING;
        }
        return AI_REPLY_PARSE_INVALID;
    }

    if (memcmp(ctx.rxBuf, prefix, prefixLen) != 0)
    {
        return AI_REPLY_PARSE_INVALID;
    }

    valueStr = &ctx.rxBuf[prefixLen];
    valueLen = strlen(valueStr);

    if (valueLen < 5u)
    {
        return AI_REPLY_PARSE_PENDING;
    }
    if (valueLen > 12u)
    {
        return AI_REPLY_PARSE_INVALID;
    }

    volts = strtof(valueStr, &endPtr);
    if ((endPtr == valueStr) || (*endPtr != '\0'))
    {
        return AI_REPLY_PARSE_INVALID;
    }

    if ((volts < -0.05f) || (volts > 3.5f))
    {
        return AI_REPLY_PARSE_INVALID;
    }

    *adcVoltsOut = volts;
    return AI_REPLY_PARSE_VALID;
}

/* ======================================================================== */
/*  Public API                                                               */
/* ======================================================================== */

void AITest_Start(void)
{
    memset(&ctx, 0, sizeof(ctx));

    safeAnalogIdle();

    ctx.channel = 1u;
    ctx.level = AI_TEST_LEVEL_LOW;
    ctx.attempt = 1u;
    ctx.subState = AI_SUB_PREP_POINT;
    ctx.timestamp = HAL_GetTick();
}

void AITest_Tick(void)
{
    ai_test_channel_kind_t kind;
    float expectedEng;
    float tolEng;
    float measuredEng;
    float adcVoltsEval;
    float lowerBound;
    float upperBound;
    char reqToken[20];

    switch (ctx.subState)
    {
    case AI_SUB_PREP_POINT:
        GPIO_Config_WriteDacEn(GPIO_PIN_RESET);
        GPIO_Config_SetMuxChannel((uint8_t)(ctx.channel - 1u));
        DAC_SetFromSlider(sliderSetpointForChannel(ctx.channel, ctx.level));
        ctx.timestamp = HAL_GetTick();
        ctx.subState = AI_SUB_WAIT_PRE_ENABLE_SETTLE;
        break;

    case AI_SUB_WAIT_PRE_ENABLE_SETTLE:
        if ((HAL_GetTick() - ctx.timestamp) >= AI_TEST_PRE_ENABLE_SETTLE_MS)
        {
            ctx.subState = AI_SUB_ENABLE_OUTPUT;
        }
        break;

    case AI_SUB_ENABLE_OUTPUT:
        GPIO_Config_WriteDacEn(GPIO_PIN_SET);
        ctx.timestamp = HAL_GetTick();
        ctx.subState = AI_SUB_WAIT_OUTPUT_SETTLE;
        break;

    case AI_SUB_WAIT_OUTPUT_SETTLE:
        if ((HAL_GetTick() - ctx.timestamp) >= AI_TEST_OUTPUT_SETTLE_MS)
        {
            ctx.subState = AI_SUB_SEND_SAMPLE_REQ;
        }
        break;

    case AI_SUB_SEND_SAMPLE_REQ:
        resetReplyContext();
        UART4_ClearRxStaleState();
        buildSampleReqToken(ctx.channel, ctx.level, reqToken, sizeof(reqToken));

        if (UART4_Transmit((const uint8_t *)reqToken, (uint16_t)strlen(reqToken), AI_TEST_TX_TIMEOUT_MS) != HAL_OK)
        {
            ctx.txError = true;
            ctx.subState = AI_SUB_EVALUATE;
            break;
        }

        ctx.timestamp = HAL_GetTick();
        ctx.subState = AI_SUB_WAIT_REPLY;
        break;

    case AI_SUB_WAIT_REPLY:
        if (ctx.replyParseStatus != AI_REPLY_PARSE_INVALID)
        {
            pollReplyBytes();
        }

        if ((ctx.replyParseStatus == AI_REPLY_PARSE_PENDING) &&
            (ctx.rxLen > 0u) &&
            ((HAL_GetTick() - ctx.lastRxByteTick) >= AI_TEST_RX_QUIET_MS))
        {
            ctx.replyParseStatus = tryParseCurrentReply(&ctx.parsedAdcVolts);
        }

        if (ctx.replyParseStatus != AI_REPLY_PARSE_PENDING)
        {
            ctx.subState = AI_SUB_EVALUATE;
        }
        else if ((HAL_GetTick() - ctx.timestamp) >= AI_TEST_REPLY_TIMEOUT_MS)
        {
            /* Final parse attempt before declaring timeout. */
            if (ctx.rxLen > 0u)
            {
                ctx.replyParseStatus = tryParseCurrentReply(&ctx.parsedAdcVolts);
            }
            ctx.replyTimedOut = true;
            ctx.subState = AI_SUB_EVALUATE;
        }
        break;

    case AI_SUB_EVALUATE:
        kind = channelKind(ctx.channel);
        expectedEng = expectedEngineering(kind, ctx.level);
        tolEng = toleranceEngineering(kind);

        if (ctx.replyParseStatus == AI_REPLY_PARSE_VALID)
        {
            adcVoltsEval = ctx.parsedAdcVolts;
            if ((adcVoltsEval > -AI_TEST_NEAR_ZERO_CLAMP_V) && (adcVoltsEval < AI_TEST_NEAR_ZERO_CLAMP_V))
            {
                adcVoltsEval = 0.0f;
            }

            measuredEng = adcToEngineering(kind, adcVoltsEval);
            lowerBound = expectedEng - tolEng;
            if (lowerBound < 0.0f)
            {
                lowerBound = 0.0f;
            }
            upperBound = expectedEng + tolEng;

            if ((measuredEng >= lowerBound) && (measuredEng <= upperBound))
            {
                recordAnalogPass(ctx.channel, ctx.level, kind, measuredEng, expectedEng, tolEng);

                if (ctx.level == AI_TEST_LEVEL_HIGH)
                {
                    postProgress(ctx.channel);

                    if (ctx.channel >= AI_TEST_CHANNEL_COUNT)
                    {
                        safeAnalogIdle();
                        postResultPass();
                        ctx.subState = AI_SUB_DONE;
                    }
                    else
                    {
                        ctx.channel++;
                        ctx.level = AI_TEST_LEVEL_LOW;
                        ctx.attempt = 1u;
                        ctx.subState = AI_SUB_PREP_POINT;
                    }
                }
                else
                {
                    ctx.level = (ai_test_level_t)((uint8_t)ctx.level + 1u);
                    ctx.attempt = 1u;
                    ctx.subState = AI_SUB_PREP_POINT;
                }
                break;
            }

            /* Out-of-range */
            GPIO_Config_WriteDacEn(GPIO_PIN_RESET);
            if (ctx.attempt < AI_TEST_MAX_ATTEMPTS)
            {
                ctx.attempt++;
                ctx.subState = AI_SUB_PREP_POINT;
            }
            else
            {
                safeAnalogIdle();
                recordAnalogFail(ctx.channel, ctx.level, kind, AI_TEST_FAIL_OUT_OF_RANGE,
                                 measuredEng, expectedEng, tolEng);
                postResultFail(AI_TEST_FAIL_OUT_OF_RANGE, EOL_ERR_AI_OUT_OF_RANGE,
                               adcVoltsEval, measuredEng, expectedEng, tolEng);
                ctx.subState = AI_SUB_DONE;
            }
            break;
        }

        GPIO_Config_WriteDacEn(GPIO_PIN_RESET);

        if (ctx.attempt < AI_TEST_MAX_ATTEMPTS)
        {
            ctx.attempt++;
            ctx.subState = AI_SUB_PREP_POINT;
            break;
        }

        if (ctx.txError || (ctx.replyTimedOut && (ctx.rxLen == 0u)))
        {
            safeAnalogIdle();
            recordAnalogFail(ctx.channel, ctx.level, kind, AI_TEST_FAIL_NO_RESPONSE,
                             0.0f, expectedEng, tolEng);
            postResultFail(AI_TEST_FAIL_NO_RESPONSE, EOL_ERR_AI_NO_RESPONSE,
                           0.0f, 0.0f, expectedEng, tolEng);
        }
        else
        {
            safeAnalogIdle();
            recordAnalogFail(ctx.channel, ctx.level, kind, AI_TEST_FAIL_INVALID_REPLY,
                             0.0f, expectedEng, tolEng);
            postResultFail(AI_TEST_FAIL_INVALID_REPLY, EOL_ERR_AI_INVALID_REPLY,
                           0.0f, 0.0f, expectedEng, tolEng);
        }
        ctx.subState = AI_SUB_DONE;
        break;

    case AI_SUB_IDLE:
    case AI_SUB_DONE:
    default:
        break;
    }
}

bool AITest_HasProgressEvent(void)
{
    return ctx.hasProgress;
}

ai_test_progress_t AITest_GetProgressEvent(void)
{
    ctx.hasProgress = false;
    return ctx.progress;
}

bool AITest_HasResult(void)
{
    return ctx.hasResult;
}

ai_test_result_t AITest_GetResult(void)
{
    ctx.hasResult = false;
    return ctx.result;
}
