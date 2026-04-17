/**
  ******************************************************************************
  * @file    comm_test.c
 * @brief   Non-blocking EOL comm/reset test engine (COMM + MCU Reset + Fault).
  *
  *          Designed to be called from Model::tick() (~60 Hz).
  *          Uses UART4 to communicate with the ESP32-S3 and PB11 to pulse the
  *          MCU reset button output.
  *
  *          The state machine uses HAL tick counts so it never blocks the UI.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "comm_test.h"
#include "eol_report.h"
#include "gpio_config.h"
#include "uart_config.h"
#include "stm32u5xx_hal.h"
#include <string.h>

/* ======================================================================== */
/*  Internal State                                                          */
/* ======================================================================== */

typedef enum
{
    COMM_SUB_WAIT_BEFORE_PING,
    COMM_SUB_SEND_PING,
    COMM_SUB_WAIT_PONG,
    COMM_SUB_EVALUATE,
} comm_sub_state_t;

typedef enum
{
    MCU_RESET_SUB_ASSERT_PULSE,
    MCU_RESET_SUB_HOLD_PULSE,
    MCU_RESET_SUB_RELEASE_PULSE,
    MCU_RESET_SUB_WAIT_RECOVERY,
    MCU_RESET_SUB_SEND_PING,
    MCU_RESET_SUB_WAIT_PONG,
    MCU_RESET_SUB_EVALUATE,
    MCU_RESET_SUB_WAIT_RETRY_DELAY,
} mcu_reset_sub_state_t;

typedef enum
{
    FAULT_CHECK_SUB_ASSERT_PULSE,
    FAULT_CHECK_SUB_HOLD_PULSE,
    FAULT_CHECK_SUB_RELEASE_PULSE,
    FAULT_CHECK_SUB_WAIT_RECOVERY,
    FAULT_CHECK_SUB_SEND_READY,
    FAULT_CHECK_SUB_WAIT_OK,
    FAULT_CHECK_SUB_EVALUATE,
    FAULT_CHECK_SUB_WAIT_RETRY_DELAY,
} fault_check_sub_state_t;

typedef struct
{
    eol_stage_t          stage;
    comm_sub_state_t     commSub;
    mcu_reset_sub_state_t mcuResetSub;
    fault_check_sub_state_t faultCheckSub;
    uint8_t              attempt;        /* 1-based attempt counter          */
    uint32_t             timestamp;      /* HAL_GetTick() snapshot           */

    uint8_t              rxBuf[EOL_MSG_PONG_LEN + 1];
    uint8_t              rxLen;
    volatile bool        rxDone;
    volatile bool        uartError;
    HAL_StatusTypeDef    txStatus;
    uint32_t             uartErrorCode;

    eol_test_result_t    result;
    bool                 hasResult;

    eol_progress_event_t progressEvent;
    bool                 hasProgressEvent;
} comm_test_ctx_t;

static comm_test_ctx_t ctx;

/* ======================================================================== */
/*  Helpers                                                                 */
/* ======================================================================== */

static void postResult(eol_stage_t stage, eol_error_code_t err)
{
    ctx.result.stage = stage;
    ctx.result.errorCode = err;
    ctx.hasResult = true;
}

static void postProgressEvent(eol_progress_event_t evt)
{
    ctx.progressEvent = evt;
    ctx.hasProgressEvent = true;
}

static void finishWithError(eol_stage_t stage, eol_error_code_t err)
{
    postResult(stage, err);
    ctx.stage = EOL_STAGE_DONE;
}

static void recordStagePass(eol_stage_t stage)
{
    switch (stage)
    {
    case EOL_STAGE_COMM:
        (void)EOL_Report_AddRow(EOL_REPORT_ROW_PASS, "Communication", "PONG response", "PONG");
        break;

    case EOL_STAGE_MCU_RESET:
        (void)EOL_Report_AddRow(EOL_REPORT_ROW_PASS, "MCU Reset", "PONG response", "PONG");
        break;

    case EOL_STAGE_FAULT_CHECK:
        (void)EOL_Report_AddRow(EOL_REPORT_ROW_PASS, "Fault Check", "OK response", "OK");
        break;

    default:
        break;
    }
}

static void recordStageFail(eol_stage_t stage, eol_error_code_t err)
{
    const char* testName = "Unknown";
    const char* expected = "N/A";
    const char* actual = "N/A";

    switch (stage)
    {
    case EOL_STAGE_COMM:
        testName = "Communication";
        expected = "PONG response";
        actual = (err == EOL_ERR_COMM_PING_BAD_REPLY) ? "Invalid reply" : "No response";
        break;

    case EOL_STAGE_MCU_RESET:
        testName = "MCU Reset";
        if (err == EOL_ERR_MCU_RESET_PULSE_VERIFY_FAIL)
        {
            expected = "PB11 pulse + PONG after reset";
            actual = "Pulse verify failed";
        }
        else
        {
            expected = "PONG response";
            actual = (err == EOL_ERR_MCU_RESET_PING_BAD_REPLY) ? "Invalid reply" : "No response";
        }
        break;

    case EOL_STAGE_FAULT_CHECK:
        testName = "Fault Check";
        if (err == EOL_ERR_FAULT_CHECK_PULSE_VERIFY_FAIL)
        {
            expected = "PA8 pulse + OK response";
            actual = "Pulse verify failed";
        }
        else
        {
            expected = "OK response";
            actual = (err == EOL_ERR_FAULT_CHECK_BAD_REPLY) ? "Invalid reply" : "No response";
        }
        break;

    default:
        break;
    }

    (void)EOL_Report_AddRow(EOL_REPORT_ROW_FAIL, testName, expected, actual);
    EOL_Report_FinalizeFail();
}

static void recordAndFinishWithError(eol_stage_t stage, eol_error_code_t err)
{
    recordStageFail(stage, err);
    finishWithError(stage, err);
}

static void resetHandshakeAttemptContext(void)
{
    memset(ctx.rxBuf, 0, sizeof(ctx.rxBuf));
    ctx.rxLen = 0;
    ctx.rxDone = false;
    ctx.uartError = false;
    ctx.txStatus = HAL_OK;
    ctx.uartErrorCode = HAL_UART_ERROR_NONE;
}

static void UART4_ClearRxStaleState(void)
{
    UART4_ClearRxBuffer();
    __HAL_UART_CLEAR_FLAG(&huart4, UART_CLEAR_PEF | UART_CLEAR_FEF |
                                   UART_CLEAR_NEF | UART_CLEAR_OREF);
    __HAL_UART_FLUSH_DRREGISTER(&huart4);
    huart4.ErrorCode = HAL_UART_ERROR_NONE;
}

static void UART4_PollRxIntoContext(uint8_t expectedLen)
{
    uint8_t byteIn = 0;
    const uint8_t targetLen = (expectedLen > EOL_MSG_PONG_LEN) ? EOL_MSG_PONG_LEN : expectedLen;

    while (ctx.rxLen < targetLen)
    {
        if (!UART4_ReadByte(&byteIn))
        {
            break;
        }
        ctx.rxBuf[ctx.rxLen++] = byteIn;
    }

    if (ctx.rxLen <= EOL_MSG_PONG_LEN)
    {
        ctx.rxBuf[ctx.rxLen] = 0;
    }

    if (ctx.rxLen >= targetLen)
    {
        ctx.rxDone = true;
    }
}

static bool sendRequestAndStartWait(const uint8_t *txData, uint16_t txLen)
{
    resetHandshakeAttemptContext();
    UART4_ClearRxStaleState();

    ctx.txStatus = UART4_Transmit(txData, txLen, 100);
    if (ctx.txStatus != HAL_OK)
    {
        ctx.uartError = true;
        ctx.uartErrorCode = huart4.ErrorCode;
        return false;
    }

    ctx.timestamp = HAL_GetTick();
    return true;
}

static bool attemptReceivedExact(const char *expected, uint8_t expectedLen)
{
    return (ctx.rxLen >= expectedLen) &&
           (memcmp(ctx.rxBuf, expected, expectedLen) == 0);
}

static bool verifyMcuResetPin(GPIO_PinState expected)
{
    return HAL_GPIO_ReadPin(MCUBTN_GPIO_Port, MCUBTN_Pin) == expected;
}

static bool verifyFaultCheckPin(GPIO_PinState expected)
{
    return HAL_GPIO_ReadPin(FLTBTN_GPIO_Port, FLTBTN_Pin) == expected;
}

/* ======================================================================== */
/*  CommTest_Start                                                          */
/* ======================================================================== */
void CommTest_Start(void)
{
    memset(&ctx, 0, sizeof(ctx));
    EOL_Report_ResetForNewRun();

    ctx.stage = EOL_STAGE_COMM;
    ctx.commSub = COMM_SUB_WAIT_BEFORE_PING;
    ctx.mcuResetSub = MCU_RESET_SUB_ASSERT_PULSE;
    ctx.faultCheckSub = FAULT_CHECK_SUB_ASSERT_PULSE;
    ctx.attempt = 1;
    ctx.timestamp = HAL_GetTick();
}

/* ======================================================================== */
/*  CommTest_Tick                                                           */
/* ======================================================================== */
void CommTest_Tick(void)
{
    switch (ctx.stage)
    {
    case EOL_STAGE_COMM:
    {
        switch (ctx.commSub)
        {
        case COMM_SUB_WAIT_BEFORE_PING:
            if ((HAL_GetTick() - ctx.timestamp) >= EOL_PING_RETRY_DELAY_MS)
            {
                ctx.commSub = COMM_SUB_SEND_PING;
            }
            break;

        case COMM_SUB_SEND_PING:
            if (sendRequestAndStartWait((const uint8_t*)EOL_MSG_PING, EOL_MSG_PING_LEN))
            {
                ctx.commSub = COMM_SUB_WAIT_PONG;
            }
            else
            {
                ctx.commSub = COMM_SUB_EVALUATE;
            }
            break;

        case COMM_SUB_WAIT_PONG:
            UART4_PollRxIntoContext(EOL_MSG_PONG_LEN);

            if (ctx.rxDone || ctx.uartError ||
                ((HAL_GetTick() - ctx.timestamp) >= EOL_PING_TIMEOUT_MS))
            {
                ctx.commSub = COMM_SUB_EVALUATE;
            }
            break;

        case COMM_SUB_EVALUATE:
            if (attemptReceivedExact(EOL_MSG_PONG, EOL_MSG_PONG_LEN))
            {
                recordStagePass(EOL_STAGE_COMM);
                postResult(EOL_STAGE_COMM, EOL_ERR_NONE);

                /* Continue immediately into the MCU reset phase. */
                ctx.stage = EOL_STAGE_MCU_RESET;
                ctx.mcuResetSub = MCU_RESET_SUB_ASSERT_PULSE;
                ctx.attempt = 1;
                ctx.timestamp = HAL_GetTick();
            }
            else if (ctx.attempt >= EOL_PING_MAX_RETRIES)
            {
                recordAndFinishWithError(EOL_STAGE_COMM,
                                         (ctx.rxLen > 0U) ? EOL_ERR_COMM_PING_BAD_REPLY
                                                          : EOL_ERR_COMM_PING_NO_RESPONSE);
            }
            else
            {
                ctx.attempt++;
                ctx.timestamp = HAL_GetTick();
                ctx.commSub = COMM_SUB_WAIT_BEFORE_PING;
            }
            break;
        }
        break;
    }

    case EOL_STAGE_MCU_RESET:
    {
        switch (ctx.mcuResetSub)
        {
        case MCU_RESET_SUB_ASSERT_PULSE:
            GPIO_Config_WriteCtrlBtn(1, GPIO_PIN_SET); /* index 1 = MCU-RST PB11 */
            if (!verifyMcuResetPin(GPIO_PIN_SET))
            {
                recordAndFinishWithError(EOL_STAGE_MCU_RESET, EOL_ERR_MCU_RESET_PULSE_VERIFY_FAIL);
                break;
            }
            ctx.timestamp = HAL_GetTick();
            ctx.mcuResetSub = MCU_RESET_SUB_HOLD_PULSE;
            break;

        case MCU_RESET_SUB_HOLD_PULSE:
            if ((HAL_GetTick() - ctx.timestamp) >= EOL_MCU_RESET_PULSE_MS)
            {
                ctx.mcuResetSub = MCU_RESET_SUB_RELEASE_PULSE;
            }
            break;

        case MCU_RESET_SUB_RELEASE_PULSE:
            GPIO_Config_WriteCtrlBtn(1, GPIO_PIN_RESET);
            if (!verifyMcuResetPin(GPIO_PIN_RESET))
            {
                recordAndFinishWithError(EOL_STAGE_MCU_RESET, EOL_ERR_MCU_RESET_PULSE_VERIFY_FAIL);
                break;
            }

            postProgressEvent(EOL_PROGRESS_EVT_MCU_RESET_BUTTON_PULSE_DONE);
            ctx.attempt = 1;
            ctx.timestamp = HAL_GetTick();
            ctx.mcuResetSub = MCU_RESET_SUB_WAIT_RECOVERY;
            break;

        case MCU_RESET_SUB_WAIT_RECOVERY:
            if ((HAL_GetTick() - ctx.timestamp) >= EOL_MCU_RESET_RECOVERY_MS)
            {
                ctx.mcuResetSub = MCU_RESET_SUB_SEND_PING;
            }
            break;

        case MCU_RESET_SUB_WAIT_RETRY_DELAY:
            if ((HAL_GetTick() - ctx.timestamp) >= EOL_PING_RETRY_DELAY_MS)
            {
                ctx.mcuResetSub = MCU_RESET_SUB_SEND_PING;
            }
            break;

        case MCU_RESET_SUB_SEND_PING:
            if (sendRequestAndStartWait((const uint8_t*)EOL_MSG_PING, EOL_MSG_PING_LEN))
            {
                ctx.mcuResetSub = MCU_RESET_SUB_WAIT_PONG;
            }
            else
            {
                ctx.mcuResetSub = MCU_RESET_SUB_EVALUATE;
            }
            break;

        case MCU_RESET_SUB_WAIT_PONG:
            UART4_PollRxIntoContext(EOL_MSG_PONG_LEN);

            if (ctx.rxDone || ctx.uartError ||
                ((HAL_GetTick() - ctx.timestamp) >= EOL_PING_TIMEOUT_MS))
            {
                ctx.mcuResetSub = MCU_RESET_SUB_EVALUATE;
            }
            break;

        case MCU_RESET_SUB_EVALUATE:
            if (attemptReceivedExact(EOL_MSG_PONG, EOL_MSG_PONG_LEN))
            {
                recordStagePass(EOL_STAGE_MCU_RESET);
                postResult(EOL_STAGE_MCU_RESET, EOL_ERR_NONE);
                ctx.stage = EOL_STAGE_FAULT_CHECK;
                ctx.faultCheckSub = FAULT_CHECK_SUB_ASSERT_PULSE;
                ctx.attempt = 1;
                ctx.timestamp = HAL_GetTick();
            }
            else if (ctx.attempt >= EOL_PING_MAX_RETRIES)
            {
                recordAndFinishWithError(EOL_STAGE_MCU_RESET,
                                         (ctx.rxLen > 0U) ? EOL_ERR_MCU_RESET_PING_BAD_REPLY
                                                          : EOL_ERR_MCU_RESET_NO_RECONNECT);
            }
            else
            {
                ctx.attempt++;
                ctx.timestamp = HAL_GetTick();
                ctx.mcuResetSub = MCU_RESET_SUB_WAIT_RETRY_DELAY;
            }
            break;
        }
        break;
    }

    case EOL_STAGE_FAULT_CHECK:
    {
        switch (ctx.faultCheckSub)
        {
        case FAULT_CHECK_SUB_ASSERT_PULSE:
            GPIO_Config_WriteCtrlBtn(2, GPIO_PIN_SET); /* index 2 = FAULT / PA8 */
            if (!verifyFaultCheckPin(GPIO_PIN_SET))
            {
                recordAndFinishWithError(EOL_STAGE_FAULT_CHECK, EOL_ERR_FAULT_CHECK_PULSE_VERIFY_FAIL);
                break;
            }
            ctx.timestamp = HAL_GetTick();
            ctx.faultCheckSub = FAULT_CHECK_SUB_HOLD_PULSE;
            break;

        case FAULT_CHECK_SUB_HOLD_PULSE:
            if ((HAL_GetTick() - ctx.timestamp) >= EOL_FAULT_CHECK_PULSE_MS)
            {
                ctx.faultCheckSub = FAULT_CHECK_SUB_RELEASE_PULSE;
            }
            break;

        case FAULT_CHECK_SUB_RELEASE_PULSE:
            GPIO_Config_WriteCtrlBtn(2, GPIO_PIN_RESET);
            if (!verifyFaultCheckPin(GPIO_PIN_RESET))
            {
                recordAndFinishWithError(EOL_STAGE_FAULT_CHECK, EOL_ERR_FAULT_CHECK_PULSE_VERIFY_FAIL);
                break;
            }

            postProgressEvent(EOL_PROGRESS_EVT_FAULT_CHECK_BUTTON_PULSE_DONE);
            ctx.attempt = 1;
            ctx.timestamp = HAL_GetTick();
            ctx.faultCheckSub = FAULT_CHECK_SUB_WAIT_RECOVERY;
            break;

        case FAULT_CHECK_SUB_WAIT_RECOVERY:
            if ((HAL_GetTick() - ctx.timestamp) >= EOL_FAULT_CHECK_RECOVERY_MS)
            {
                ctx.faultCheckSub = FAULT_CHECK_SUB_SEND_READY;
            }
            break;

        case FAULT_CHECK_SUB_WAIT_RETRY_DELAY:
            if ((HAL_GetTick() - ctx.timestamp) >= EOL_PING_RETRY_DELAY_MS)
            {
                ctx.faultCheckSub = FAULT_CHECK_SUB_SEND_READY;
            }
            break;

        case FAULT_CHECK_SUB_SEND_READY:
            if (sendRequestAndStartWait((const uint8_t*)EOL_MSG_READY, EOL_MSG_READY_LEN))
            {
                ctx.faultCheckSub = FAULT_CHECK_SUB_WAIT_OK;
            }
            else
            {
                ctx.faultCheckSub = FAULT_CHECK_SUB_EVALUATE;
            }
            break;

        case FAULT_CHECK_SUB_WAIT_OK:
            UART4_PollRxIntoContext(EOL_MSG_OK_LEN);

            if (ctx.rxDone || ctx.uartError ||
                ((HAL_GetTick() - ctx.timestamp) >= EOL_PING_TIMEOUT_MS))
            {
                ctx.faultCheckSub = FAULT_CHECK_SUB_EVALUATE;
            }
            break;

        case FAULT_CHECK_SUB_EVALUATE:
            if (attemptReceivedExact(EOL_MSG_OK, EOL_MSG_OK_LEN))
            {
                recordStagePass(EOL_STAGE_FAULT_CHECK);
                postResult(EOL_STAGE_FAULT_CHECK, EOL_ERR_NONE);
                ctx.stage = EOL_STAGE_DONE;
            }
            else if (ctx.attempt >= EOL_PING_MAX_RETRIES)
            {
                recordAndFinishWithError(EOL_STAGE_FAULT_CHECK,
                                         (ctx.rxLen > 0U) ? EOL_ERR_FAULT_CHECK_BAD_REPLY
                                                          : EOL_ERR_FAULT_CHECK_NO_RECONNECT);
            }
            else
            {
                ctx.attempt++;
                ctx.timestamp = HAL_GetTick();
                ctx.faultCheckSub = FAULT_CHECK_SUB_WAIT_RETRY_DELAY;
            }
            break;
        }
        break;
    }

    case EOL_STAGE_IDLE:
    case EOL_STAGE_DONE:
    default:
        break;
    }
}

/* ======================================================================== */
/*  Result / Progress Mailboxes                                             */
/* ======================================================================== */
bool CommTest_HasResult(void)
{
    return ctx.hasResult;
}

eol_test_result_t CommTest_GetResult(void)
{
    ctx.hasResult = false;
    return ctx.result;
}

bool CommTest_HasProgressEvent(void)
{
    return ctx.hasProgressEvent;
}

eol_progress_event_t CommTest_GetProgressEvent(void)
{
    ctx.hasProgressEvent = false;
    return ctx.progressEvent;
}

eol_stage_t CommTest_CurrentStage(void)
{
    return ctx.stage;
}
