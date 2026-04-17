/**
  ******************************************************************************
  * @file    output_test.c
  * @brief   Non-blocking Outputs test engine for Outputs_Screen.
  *******************************************************************************
  */

#include "output_test.h"
#include "eol_report.h"
#include "gpio_config.h"
#include "uart_config.h"
#include "stm32u5xx_hal.h"

#include <stdio.h>
#include <string.h>

/* ======================================================================== */
/*  Timing / Limits                                                         */
/* ======================================================================== */
#define OUTPUT_TEST_REPLY_TIMEOUT_MS   3000u
#define OUTPUT_TEST_TX_TIMEOUT_MS       100u
#define OUTPUT_TEST_RELAY_SETTLE_MS     100u
#define OUTPUT_TEST_MAX_ATTEMPTS          2u /* initial + 1 retry */
#define OUTPUT_TEST_RX_BUF_LEN           16u

typedef enum
{
    OUTPUT_SUB_IDLE = 0,
    OUTPUT_SUB_AUTO_PASS_DO2,
    OUTPUT_SUB_SEND_COMMAND,
    OUTPUT_SUB_WAIT_REPLY,
    OUTPUT_SUB_WAIT_RELAY_SETTLE,
    OUTPUT_SUB_VERIFY_STATE,
    OUTPUT_SUB_DONE
} output_sub_state_t;

typedef enum
{
    OUTPUT_REPLY_PARSE_PENDING = 0,
    OUTPUT_REPLY_PARSE_VALID,
    OUTPUT_REPLY_PARSE_INVALID
} output_reply_parse_status_t;

typedef struct
{
    output_sub_state_t          subState;
    output_test_phase_t         phase;
    uint8_t                     attempt;
    uint32_t                    timestamp;

    uint8_t                     rxLen;
    char                        rxBuf[OUTPUT_TEST_RX_BUF_LEN];
    bool                        txError;
    bool                        replyTimedOut;
    output_reply_parse_status_t replyStatus;

    output_test_progress_t      progress;
    bool                        hasProgress;

    output_test_result_t        result;
    bool                        hasResult;
} output_test_ctx_t;

static output_test_ctx_t ctx;

/* ======================================================================== */
/*  Helpers                                                                 */
/* ======================================================================== */

static void postProgress(output_test_item_t itemPassed)
{
    ctx.progress.itemPassed = itemPassed;
    ctx.hasProgress = true;
}

static void buildCommandToken(output_test_phase_t phase, char* buf, size_t len)
{
    const char* token = "";

    switch (phase)
    {
    case OUTPUT_TEST_PHASE_K1_ON:  token = "K1_ON";  break;
    case OUTPUT_TEST_PHASE_K1_OFF: token = "K1_OFF"; break;
    case OUTPUT_TEST_PHASE_K2_ON:  token = "K2_ON";  break;
    case OUTPUT_TEST_PHASE_K2_OFF: token = "K2_OFF"; break;
    default:                       token = "";       break;
    }

    (void)snprintf(buf, len, "%s", token);
}

static void buildAckToken(output_test_phase_t phase, char* buf, size_t len)
{
    const char* token = "";

    switch (phase)
    {
    case OUTPUT_TEST_PHASE_K1_ON:  token = "OK_K1_ON";  break;
    case OUTPUT_TEST_PHASE_K1_OFF: token = "OK_K1_OFF"; break;
    case OUTPUT_TEST_PHASE_K2_ON:  token = "OK_K2_ON";  break;
    case OUTPUT_TEST_PHASE_K2_OFF: token = "OK_K2_OFF"; break;
    default:                       token = "";          break;
    }

    (void)snprintf(buf, len, "%s", token);
}

static void buildLogicalSummary(output_test_phase_t phase,
                                bool noClosed,
                                bool ncClosed,
                                char* buf,
                                size_t len)
{
    (void)phase;

    /* Keep summaries short enough to fit the narrow Fault screen columns. */
    (void)snprintf(buf, len, "NO %s\nNC %s",
                   noClosed ? "Closed" : "Open",
                   ncClosed ? "Closed" : "Open");
}

static void buildReportLogicalSummary(output_test_phase_t phase,
                                      bool noClosed,
                                      bool ncClosed,
                                      char* buf,
                                      size_t len)
{
    const char* prefix = ((phase == OUTPUT_TEST_PHASE_K2_ON) ||
                          (phase == OUTPUT_TEST_PHASE_K2_OFF)) ? "K2" : "K1";

    (void)snprintf(buf, len, "%sNO=%s %sNC=%s",
                   prefix,
                   noClosed ? "Closed" : "Open",
                   prefix,
                   ncClosed ? "Closed" : "Open");
}

static const char* phaseName(output_test_phase_t phase)
{
    switch (phase)
    {
    case OUTPUT_TEST_PHASE_K1_ON:  return "Relay K1 ON";
    case OUTPUT_TEST_PHASE_K1_OFF: return "Relay K1 OFF";
    case OUTPUT_TEST_PHASE_K2_ON:  return "Relay K2 ON";
    case OUTPUT_TEST_PHASE_K2_OFF: return "Relay K2 OFF";
    default:                       return "Relay Output";
    }
}

static output_test_item_t phasePassItem(output_test_phase_t phase)
{
    switch (phase)
    {
    case OUTPUT_TEST_PHASE_K1_ON:  return OUTPUT_TEST_ITEM_K1_NO;
    case OUTPUT_TEST_PHASE_K1_OFF: return OUTPUT_TEST_ITEM_K1_NC;
    case OUTPUT_TEST_PHASE_K2_ON:  return OUTPUT_TEST_ITEM_K2_NO;
    case OUTPUT_TEST_PHASE_K2_OFF: return OUTPUT_TEST_ITEM_K2_NC;
    default:                       return OUTPUT_TEST_ITEM_DO1;
    }
}

static output_test_phase_t nextPhase(output_test_phase_t phase)
{
    switch (phase)
    {
    case OUTPUT_TEST_PHASE_K1_ON:  return OUTPUT_TEST_PHASE_K1_OFF;
    case OUTPUT_TEST_PHASE_K1_OFF: return OUTPUT_TEST_PHASE_K2_ON;
    case OUTPUT_TEST_PHASE_K2_ON:  return OUTPUT_TEST_PHASE_K2_OFF;
    default:                       return OUTPUT_TEST_PHASE_NONE;
    }
}

static void expectedContactStates(output_test_phase_t phase, bool* noClosed, bool* ncClosed)
{
    const bool relayOn = (phase == OUTPUT_TEST_PHASE_K1_ON) ||
                         (phase == OUTPUT_TEST_PHASE_K2_ON);

    *noClosed = relayOn;
    *ncClosed = !relayOn;
}

static bool readContactClosed(uint8_t inputIndex, bool* closedOut)
{
    if ((closedOut == 0) || (inputIndex >= 4u))
    {
        return false;
    }

    /* Optocoupler collector nodes idle HIGH via pull-up and pull LOW when conducting. */
    *closedOut = (GPIO_Config_ReadInput(inputIndex) == GPIO_PIN_RESET);
    return true;
}

static bool readPhaseContacts(output_test_phase_t phase, bool* noClosedOut, bool* ncClosedOut)
{
    uint8_t noIndex = 0u;
    uint8_t ncIndex = 1u;

    if ((noClosedOut == 0) || (ncClosedOut == 0))
    {
        return false;
    }

    if ((phase == OUTPUT_TEST_PHASE_K2_ON) || (phase == OUTPUT_TEST_PHASE_K2_OFF))
    {
        noIndex = 2u;
        ncIndex = 3u;
    }

    if (!readContactClosed(noIndex, noClosedOut))
    {
        return false;
    }

    if (!readContactClosed(ncIndex, ncClosedOut))
    {
        return false;
    }

    return true;
}

static void buildExpectedSummaryForPhase(output_test_phase_t phase, char* buf, size_t len)
{
    bool noClosed = false;
    bool ncClosed = false;

    expectedContactStates(phase, &noClosed, &ncClosed);
    buildReportLogicalSummary(phase, noClosed, ncClosed, buf, len);
}

static void safeOutputsOff(void)
{
    static const char k1Off[] = "K1_OFF";
    static const char k2Off[] = "K2_OFF";

    UART4_ClearRxBuffer();
    (void)UART4_Transmit((const uint8_t*)k1Off, (uint16_t)strlen(k1Off), OUTPUT_TEST_TX_TIMEOUT_MS);
    (void)UART4_Transmit((const uint8_t*)k2Off, (uint16_t)strlen(k2Off), OUTPUT_TEST_TX_TIMEOUT_MS);
    UART4_ClearRxBuffer();
}

static void postResultPass(void)
{
    memset(&ctx.result, 0, sizeof(ctx.result));
    ctx.result.passed = true;
    ctx.result.errorCode = EOL_ERR_NONE;
    ctx.result.failureKind = OUTPUT_TEST_FAIL_NONE;
    ctx.hasResult = true;
}

static void postResultFail(output_test_failure_kind_t failKind,
                           eol_error_code_t errorCode,
                           const char* expectedState,
                           const char* actualState)
{
    memset(&ctx.result, 0, sizeof(ctx.result));
    ctx.result.passed = false;
    ctx.result.failedPhase = ctx.phase;
    ctx.result.failureKind = failKind;
    ctx.result.errorCode = errorCode;
    strncpy(ctx.result.rawReply, ctx.rxBuf, sizeof(ctx.result.rawReply) - 1u);
    ctx.result.rawReply[sizeof(ctx.result.rawReply) - 1u] = '\0';

    if (expectedState != 0)
    {
        strncpy(ctx.result.expectedState, expectedState, sizeof(ctx.result.expectedState) - 1u);
        ctx.result.expectedState[sizeof(ctx.result.expectedState) - 1u] = '\0';
    }

    if (actualState != 0)
    {
        strncpy(ctx.result.actualState, actualState, sizeof(ctx.result.actualState) - 1u);
        ctx.result.actualState[sizeof(ctx.result.actualState) - 1u] = '\0';
    }

    safeOutputsOff();
    ctx.hasResult = true;
}

static void resetReplyContext(void)
{
    ctx.rxLen = 0u;
    memset(ctx.rxBuf, 0, sizeof(ctx.rxBuf));
    ctx.txError = false;
    ctx.replyTimedOut = false;
    ctx.replyStatus = OUTPUT_REPLY_PARSE_PENDING;
}

static void UART4_ClearRxStaleState(void)
{
    UART4_ClearRxBuffer();
    __HAL_UART_CLEAR_FLAG(&huart4, UART_CLEAR_PEF | UART_CLEAR_FEF |
                                   UART_CLEAR_NEF | UART_CLEAR_OREF);
    __HAL_UART_FLUSH_DRREGISTER(&huart4);
    huart4.ErrorCode = HAL_UART_ERROR_NONE;
}

static bool isPrefixOfExpected(const char* rx, uint8_t rxLen, const char* token)
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

    while (ctx.rxLen < (OUTPUT_TEST_RX_BUF_LEN - 1u))
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
            ctx.replyStatus = OUTPUT_REPLY_PARSE_INVALID;
            break;
        }

        ctx.rxBuf[ctx.rxLen++] = (char)byteIn;
        ctx.rxBuf[ctx.rxLen] = '\0';
    }
}

static output_reply_parse_status_t classifyReply(output_test_phase_t phase)
{
    char ackToken[16];

    buildAckToken(phase, ackToken, sizeof(ackToken));

    if (ctx.rxLen == 0u)
    {
        return OUTPUT_REPLY_PARSE_PENDING;
    }

    if (strcmp(ctx.rxBuf, ackToken) == 0)
    {
        return OUTPUT_REPLY_PARSE_VALID;
    }

    if (isPrefixOfExpected(ctx.rxBuf, ctx.rxLen, ackToken))
    {
        return OUTPUT_REPLY_PARSE_PENDING;
    }

    return OUTPUT_REPLY_PARSE_INVALID;
}

static void handleVerifiedPhasePass(void)
{
    const output_test_item_t item = phasePassItem(ctx.phase);
    const output_test_phase_t next = nextPhase(ctx.phase);

    postProgress(item);

    if (next == OUTPUT_TEST_PHASE_NONE)
    {
        postResultPass();
        ctx.subState = OUTPUT_SUB_DONE;
        return;
    }

    ctx.phase = next;
    ctx.attempt = 1u;
    ctx.subState = OUTPUT_SUB_SEND_COMMAND;
}

static void recordAutoPassRows(void)
{
    (void)EOL_Report_AddRow(EOL_REPORT_ROW_PASS, "Digital Output 1", "Temporary skip", "Auto-pass (no DO board)");
    (void)EOL_Report_AddRow(EOL_REPORT_ROW_PASS, "Digital Output 2", "Temporary skip", "Auto-pass (no DO board)");
}

static void recordRelayPass(output_test_phase_t phase)
{
    char expectedSummary[32];

    buildExpectedSummaryForPhase(phase, expectedSummary, sizeof(expectedSummary));
    (void)EOL_Report_AddRow(EOL_REPORT_ROW_PASS, phaseName(phase), expectedSummary, expectedSummary);
}

static void recordRelayFail(output_test_phase_t phase,
                            output_test_failure_kind_t failKind,
                            const char* actualState)
{
    char expectedSummary[32];
    const char* actualText = actualState;

    buildExpectedSummaryForPhase(phase, expectedSummary, sizeof(expectedSummary));

    if (actualText == 0)
    {
        actualText = (failKind == OUTPUT_TEST_FAIL_INVALID_REPLY) ? "Invalid reply" : "No response";
    }

    (void)EOL_Report_AddRow(EOL_REPORT_ROW_FAIL, phaseName(phase), expectedSummary, actualText);
    EOL_Report_FinalizeFail();
}

/* ======================================================================== */
/*  Public API                                                              */
/* ======================================================================== */

void OutputTest_Start(void)
{
    memset(&ctx, 0, sizeof(ctx));

    UART4_ClearRxStaleState();
    ctx.phase = OUTPUT_TEST_PHASE_K1_ON;
    ctx.attempt = 1u;
    recordAutoPassRows();
    postProgress(OUTPUT_TEST_ITEM_DO1);
    ctx.subState = OUTPUT_SUB_AUTO_PASS_DO2;
}

void OutputTest_Tick(void)
{
    bool noClosed = false;
    bool ncClosed = false;
    bool expectedNoClosed = false;
    bool expectedNcClosed = false;
    char expectedSummary[32];
    char actualSummary[32];
    char actualReportSummary[32];
    char commandToken[12];

    switch (ctx.subState)
    {
    case OUTPUT_SUB_IDLE:
    case OUTPUT_SUB_DONE:
        break;

    case OUTPUT_SUB_AUTO_PASS_DO2:
        if (!ctx.hasProgress)
        {
            postProgress(OUTPUT_TEST_ITEM_DO2);
            ctx.subState = OUTPUT_SUB_SEND_COMMAND;
        }
        break;

    case OUTPUT_SUB_SEND_COMMAND:
        buildCommandToken(ctx.phase, commandToken, sizeof(commandToken));
        UART4_ClearRxStaleState();
        resetReplyContext();

        if (UART4_Transmit((const uint8_t*)commandToken,
                           (uint16_t)strlen(commandToken),
                           OUTPUT_TEST_TX_TIMEOUT_MS) != HAL_OK)
        {
            ctx.txError = true;
        }

        ctx.timestamp = HAL_GetTick();
        ctx.subState = OUTPUT_SUB_WAIT_REPLY;
        break;

    case OUTPUT_SUB_WAIT_REPLY:
        pollReplyBytes();

        if (ctx.replyStatus != OUTPUT_REPLY_PARSE_INVALID)
        {
            ctx.replyStatus = classifyReply(ctx.phase);
        }

        if (ctx.replyStatus == OUTPUT_REPLY_PARSE_VALID)
        {
            ctx.timestamp = HAL_GetTick();
            ctx.subState = OUTPUT_SUB_WAIT_RELAY_SETTLE;
        }
        else if (ctx.replyStatus == OUTPUT_REPLY_PARSE_INVALID)
        {
            if (ctx.attempt < OUTPUT_TEST_MAX_ATTEMPTS)
            {
                ctx.attempt++;
                ctx.subState = OUTPUT_SUB_SEND_COMMAND;
            }
            else
            {
                recordRelayFail(ctx.phase, OUTPUT_TEST_FAIL_INVALID_REPLY, 0);
                postResultFail(OUTPUT_TEST_FAIL_INVALID_REPLY,
                               EOL_ERR_OUTPUT_INVALID_REPLY,
                               0,
                               0);
                ctx.subState = OUTPUT_SUB_DONE;
            }
        }
        else if (ctx.txError || ((HAL_GetTick() - ctx.timestamp) >= OUTPUT_TEST_REPLY_TIMEOUT_MS))
        {
            if (ctx.attempt < OUTPUT_TEST_MAX_ATTEMPTS)
            {
                ctx.attempt++;
                ctx.subState = OUTPUT_SUB_SEND_COMMAND;
            }
            else
            {
                recordRelayFail(ctx.phase, OUTPUT_TEST_FAIL_NO_RESPONSE, 0);
                postResultFail(OUTPUT_TEST_FAIL_NO_RESPONSE,
                               EOL_ERR_OUTPUT_NO_RESPONSE,
                               0,
                               0);
                ctx.subState = OUTPUT_SUB_DONE;
            }
        }
        break;

    case OUTPUT_SUB_WAIT_RELAY_SETTLE:
        if ((HAL_GetTick() - ctx.timestamp) >= OUTPUT_TEST_RELAY_SETTLE_MS)
        {
            ctx.subState = OUTPUT_SUB_VERIFY_STATE;
        }
        break;

    case OUTPUT_SUB_VERIFY_STATE:
        expectedContactStates(ctx.phase, &expectedNoClosed, &expectedNcClosed);
        (void)readPhaseContacts(ctx.phase, &noClosed, &ncClosed);

        if ((noClosed == expectedNoClosed) && (ncClosed == expectedNcClosed))
        {
            recordRelayPass(ctx.phase);
            handleVerifiedPhasePass();
        }
        else
        {
            buildLogicalSummary(ctx.phase,
                                expectedNoClosed,
                                expectedNcClosed,
                                expectedSummary,
                                sizeof(expectedSummary));
            buildLogicalSummary(ctx.phase,
                                noClosed,
                                ncClosed,
                                actualSummary,
                                sizeof(actualSummary));
            buildReportLogicalSummary(ctx.phase,
                                      noClosed,
                                      ncClosed,
                                      actualReportSummary,
                                      sizeof(actualReportSummary));
            recordRelayFail(ctx.phase, OUTPUT_TEST_FAIL_STATE_MISMATCH, actualReportSummary);
            postResultFail(OUTPUT_TEST_FAIL_STATE_MISMATCH,
                           EOL_ERR_OUTPUT_STATE_MISMATCH,
                           expectedSummary,
                           actualSummary);
            ctx.subState = OUTPUT_SUB_DONE;
        }
        break;
    }
}

bool OutputTest_HasProgressEvent(void)
{
    return ctx.hasProgress;
}

output_test_progress_t OutputTest_GetProgressEvent(void)
{
    ctx.hasProgress = false;
    return ctx.progress;
}

bool OutputTest_HasResult(void)
{
    return ctx.hasResult;
}

output_test_result_t OutputTest_GetResult(void)
{
    ctx.hasResult = false;
    return ctx.result;
}
