/**
  ******************************************************************************
  * @file    di_test.c
  * @brief   Non-blocking Digital Inputs 1..12 test engine for Inputs_Screen.
  ******************************************************************************
  */

#include "di_test.h"
#include "eol_report.h"
#include "gpio_config.h"
#include "uart_config.h"
#include "stm32u5xx_hal.h"

#include <stdio.h>
#include <string.h>

/* ======================================================================== */
/*  Timing / Limits                                                          */
/* ======================================================================== */
#define DI_TEST_CHANNEL_COUNT        12u
#define DI_TEST_HIGH_HOLD_MS        100u
#define DI_TEST_LOW_HOLD_MS         100u
#define DI_TEST_REPLY_TIMEOUT_MS   3000u
#define DI_TEST_TX_TIMEOUT_MS       100u
#define DI_TEST_MAX_ATTEMPTS          2u /* initial + 1 retry */
#define DI_TEST_RX_BUF_LEN           16u

typedef enum
{
    DI_SUB_IDLE = 0,
    DI_SUB_SEND_TEST_REQ,
    DI_SUB_DRIVE_HIGH,
    DI_SUB_HOLD_HIGH,
    DI_SUB_DRIVE_LOW,
    DI_SUB_HOLD_LOW,
    DI_SUB_WAIT_REPLY,
    DI_SUB_EVALUATE,
    DI_SUB_DONE
} di_sub_state_t;

typedef enum
{
    REPLY_PARSE_PENDING = 0,
    REPLY_PARSE_OK,
    REPLY_PARSE_FAIL,
    REPLY_PARSE_INVALID
} reply_parse_status_t;

typedef struct
{
    di_sub_state_t       subState;
    uint8_t              channel; /* 1..12 */
    uint8_t              attempt; /* 1..DI_TEST_MAX_ATTEMPTS */
    uint32_t             timestamp;

    uint8_t              rxLen;
    char                 rxBuf[DI_TEST_RX_BUF_LEN];
    bool                 txError;
    bool                 replyTimedOut;
    reply_parse_status_t replyStatus;

    di_test_progress_t   progress;
    bool                 hasProgress;

    di_test_result_t     result;
    bool                 hasResult;
} di_test_ctx_t;

static di_test_ctx_t ctx;

/* ======================================================================== */
/*  Helpers                                                                  */
/* ======================================================================== */

static void clearAllDiOutputsLow(void)
{
    uint8_t i;
    for (i = 1u; i <= DI_TEST_CHANNEL_COUNT; i++)
    {
        GPIO_Config_WriteDI(i, GPIO_PIN_RESET);
    }
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
    ctx.result.failureKind = DI_TEST_FAIL_NONE;
    ctx.hasResult = true;
}

static void postResultFail(uint8_t channel, di_test_failure_kind_t kind,
                           eol_error_code_t errorCode)
{
    memset(&ctx.result, 0, sizeof(ctx.result));
    ctx.result.passed = false;
    ctx.result.failedChannel = channel;
    ctx.result.failureKind = kind;
    ctx.result.errorCode = errorCode;
    strncpy(ctx.result.rawReply, ctx.rxBuf, sizeof(ctx.result.rawReply) - 1u);
    ctx.result.rawReply[sizeof(ctx.result.rawReply) - 1u] = '\0';
    ctx.hasResult = true;
}

static void recordDigitalPass(uint8_t channel)
{
    char testName[20];

    (void)snprintf(testName, sizeof(testName), "Digital Input %u", (unsigned)channel);
    (void)EOL_Report_AddRow(EOL_REPORT_ROW_PASS, testName, "High then Low", "High then Low");
}

static void recordDigitalFail(uint8_t channel, di_test_failure_kind_t kind)
{
    char testName[20];
    const char* actual = "Invalid reply";

    if (kind == DI_TEST_FAIL_NO_RESPONSE)
    {
        actual = "No response";
    }
    else if (kind == DI_TEST_FAIL_FAIL_TOKEN)
    {
        actual = "Invalid transition";
    }

    (void)snprintf(testName, sizeof(testName), "Digital Input %u", (unsigned)channel);
    (void)EOL_Report_AddRow(EOL_REPORT_ROW_FAIL, testName, "High then Low", actual);
    EOL_Report_FinalizeFail();
}

static void resetReplyContext(void)
{
    ctx.rxLen = 0u;
    memset(ctx.rxBuf, 0, sizeof(ctx.rxBuf));
    ctx.txError = false;
    ctx.replyTimedOut = false;
    ctx.replyStatus = REPLY_PARSE_PENDING;
}

static void UART4_ClearRxStaleState(void)
{
    UART4_ClearRxBuffer();
    __HAL_UART_CLEAR_FLAG(&huart4, UART_CLEAR_PEF | UART_CLEAR_FEF |
                                   UART_CLEAR_NEF | UART_CLEAR_OREF);
    __HAL_UART_FLUSH_DRREGISTER(&huart4);
    huart4.ErrorCode = HAL_UART_ERROR_NONE;
}

static void buildDiTokens(uint8_t channel, char *reqBuf, size_t reqBufLen,
                          char *okBuf, size_t okBufLen,
                          char *failBuf, size_t failBufLen)
{
    if ((reqBuf != 0) && (reqBufLen > 0u))
    {
        (void)snprintf(reqBuf, reqBufLen, "DI%uTEST\n", (unsigned)channel);
    }
    if ((okBuf != 0) && (okBufLen > 0u))
    {
        (void)snprintf(okBuf, okBufLen, "DI%uOK", (unsigned)channel);
    }
    if ((failBuf != 0) && (failBufLen > 0u))
    {
        (void)snprintf(failBuf, failBufLen, "DI%uFAIL", (unsigned)channel);
    }
}

static bool isPrefixOfExpected(const char *rx, uint8_t rxLen, const char *token)
{
    const size_t tokenLen = strlen(token);
    if ((size_t)rxLen > tokenLen)
    {
        return false;
    }
    return (memcmp(rx, token, rxLen) == 0);
}

static void pollReplyBytes(void)
{
    uint8_t byteIn = 0u;

    while (ctx.rxLen < (DI_TEST_RX_BUF_LEN - 1u))
    {
        if (!UART4_ReadByte(&byteIn))
        {
            break;
        }

        /* Ignore ASCII whitespace; ESP32 replies are bare tokens. */
        if ((byteIn == '\r') || (byteIn == '\n') || (byteIn == '\t') || (byteIn == ' '))
        {
            continue;
        }

        if ((byteIn < 32u) || (byteIn > 126u))
        {
            /* Non-printable garbage => invalid reply. */
            ctx.replyStatus = REPLY_PARSE_INVALID;
            break;
        }

        ctx.rxBuf[ctx.rxLen++] = (char)byteIn;
        ctx.rxBuf[ctx.rxLen] = '\0';
    }
}

static reply_parse_status_t classifyReplyForChannel(uint8_t channel)
{
    char okToken[12];
    char failToken[14];

    buildDiTokens(channel, 0, 0u,
                  okToken, sizeof(okToken),
                  failToken, sizeof(failToken));

    if (ctx.rxLen == 0u)
    {
        return REPLY_PARSE_PENDING;
    }

    if (strcmp(ctx.rxBuf, okToken) == 0)
    {
        return REPLY_PARSE_OK;
    }

    if (strcmp(ctx.rxBuf, failToken) == 0)
    {
        return REPLY_PARSE_FAIL;
    }

    if (isPrefixOfExpected(ctx.rxBuf, ctx.rxLen, okToken) ||
        isPrefixOfExpected(ctx.rxBuf, ctx.rxLen, failToken))
    {
        return REPLY_PARSE_PENDING;
    }

    return REPLY_PARSE_INVALID;
}

/* ======================================================================== */
/*  Public API                                                               */
/* ======================================================================== */

void DITest_Start(void)
{
    memset(&ctx, 0, sizeof(ctx));

    clearAllDiOutputsLow();

    ctx.channel = 1u;
    ctx.attempt = 1u;
    ctx.subState = DI_SUB_SEND_TEST_REQ;
    ctx.timestamp = HAL_GetTick();
}

void DITest_Tick(void)
{
    char reqToken[12];
    di_test_failure_kind_t failKind;
    eol_error_code_t failCode;

    switch (ctx.subState)
    {
    case DI_SUB_SEND_TEST_REQ:
        resetReplyContext();
        UART4_ClearRxStaleState();

        buildDiTokens(ctx.channel, reqToken, sizeof(reqToken), 0, 0u, 0, 0u);

        if (UART4_Transmit((const uint8_t *)reqToken, (uint16_t)strlen(reqToken), DI_TEST_TX_TIMEOUT_MS) != HAL_OK)
        {
            ctx.txError = true;
            ctx.subState = DI_SUB_EVALUATE;
            break;
        }

        ctx.subState = DI_SUB_DRIVE_HIGH;
        break;

    case DI_SUB_DRIVE_HIGH:
        GPIO_Config_WriteDI(ctx.channel, GPIO_PIN_SET);
        ctx.timestamp = HAL_GetTick();
        ctx.subState = DI_SUB_HOLD_HIGH;
        break;

    case DI_SUB_HOLD_HIGH:
        if ((HAL_GetTick() - ctx.timestamp) >= DI_TEST_HIGH_HOLD_MS)
        {
            ctx.subState = DI_SUB_DRIVE_LOW;
        }
        break;

    case DI_SUB_DRIVE_LOW:
        GPIO_Config_WriteDI(ctx.channel, GPIO_PIN_RESET);
        ctx.timestamp = HAL_GetTick();
        ctx.subState = DI_SUB_HOLD_LOW;
        break;

    case DI_SUB_HOLD_LOW:
        if ((HAL_GetTick() - ctx.timestamp) >= DI_TEST_LOW_HOLD_MS)
        {
            ctx.timestamp = HAL_GetTick();
            ctx.subState = DI_SUB_WAIT_REPLY;
        }
        break;

    case DI_SUB_WAIT_REPLY:
        if (ctx.replyStatus != REPLY_PARSE_INVALID)
        {
            pollReplyBytes();
        }

        if (ctx.replyStatus != REPLY_PARSE_INVALID)
        {
            ctx.replyStatus = classifyReplyForChannel(ctx.channel);
        }

        if (ctx.replyStatus != REPLY_PARSE_PENDING)
        {
            ctx.subState = DI_SUB_EVALUATE;
        }
        else if ((HAL_GetTick() - ctx.timestamp) >= DI_TEST_REPLY_TIMEOUT_MS)
        {
            ctx.replyTimedOut = true;
            ctx.subState = DI_SUB_EVALUATE;
        }
        break;

    case DI_SUB_EVALUATE:
        if (ctx.replyStatus == REPLY_PARSE_OK)
        {
            recordDigitalPass(ctx.channel);
            postProgress(ctx.channel);

            if (ctx.channel >= DI_TEST_CHANNEL_COUNT)
            {
                clearAllDiOutputsLow();
                postResultPass();
                ctx.subState = DI_SUB_DONE;
            }
            else
            {
                ctx.channel++;
                ctx.attempt = 1u;
                ctx.subState = DI_SUB_SEND_TEST_REQ;
            }
            break;
        }

        if (ctx.replyStatus == REPLY_PARSE_FAIL)
        {
            failKind = DI_TEST_FAIL_FAIL_TOKEN;
            failCode = EOL_ERR_DI_FAIL_TOKEN;
        }
        else if (ctx.txError || (ctx.replyTimedOut && (ctx.rxLen == 0u)))
        {
            failKind = DI_TEST_FAIL_NO_RESPONSE;
            failCode = EOL_ERR_DI_NO_RESPONSE;
        }
        else
        {
            failKind = DI_TEST_FAIL_INVALID_REPLY;
            failCode = EOL_ERR_DI_INVALID_REPLY;
        }

        GPIO_Config_WriteDI(ctx.channel, GPIO_PIN_RESET);

        if (ctx.attempt < DI_TEST_MAX_ATTEMPTS)
        {
            ctx.attempt++;
            ctx.subState = DI_SUB_SEND_TEST_REQ;
        }
        else
        {
            clearAllDiOutputsLow();
            recordDigitalFail(ctx.channel, failKind);
            postResultFail(ctx.channel, failKind, failCode);
            ctx.subState = DI_SUB_DONE;
        }
        break;

    case DI_SUB_IDLE:
    case DI_SUB_DONE:
    default:
        break;
    }
}

bool DITest_HasProgressEvent(void)
{
    return ctx.hasProgress;
}

di_test_progress_t DITest_GetProgressEvent(void)
{
    ctx.hasProgress = false;
    return ctx.progress;
}

bool DITest_HasResult(void)
{
    return ctx.hasResult;
}

di_test_result_t DITest_GetResult(void)
{
    ctx.hasResult = false;
    return ctx.result;
}
