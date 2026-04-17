#include "button_led_test.h"
#include "eol_report.h"
#include "gpio_config.h"
#include "uart_config.h"
#include "stm32u5xx_hal.h"

#include <stdio.h>
#include <string.h>

#define BUTTON_LED_INIT_TIMEOUT_MS           3000u
#define BUTTON_LED_BUTTON_POLL_TIMEOUT_MS     100u
#define BUTTON_LED_PRESS_ACTION_TIMEOUT_MS   1500u
#define BUTTON_LED_RELEASE_ACTION_TIMEOUT_MS 2000u
#define BUTTON_LED_TX_TIMEOUT_MS              100u
#define BUTTON_LED_BUTTON_POLL_PERIOD_MS      100u
#define BUTTON_LED_ASSIST_PRE_PRESS_DELAY_MS 100u
#define BUTTON_LED_ASSIST_PRESS_SETTLE_MS      50u
#define BUTTON_LED_ASSIST_RELEASE_SETTLE_MS   750u
#define BUTTON_LED_INIT_MAX_ATTEMPTS            2u
#define BUTTON_LED_BUTTON_MAX_ATTEMPTS          1u
#define BUTTON_LED_RX_BUF_LEN                  24u

typedef enum
{
    BUTTON_LED_SUB_IDLE = 0,
    BUTTON_LED_SUB_SEND_COMMAND,
    BUTTON_LED_SUB_WAIT_REPLY,
    BUTTON_LED_SUB_WAIT_BUTTON_RETRY,
    BUTTON_LED_SUB_DRIVE_BUTTON_HIGH,
    BUTTON_LED_SUB_WAIT_PRESS_SETTLE,
    BUTTON_LED_SUB_HOLD_BUTTON_HIGH,
    BUTTON_LED_SUB_DRIVE_BUTTON_LOW,
    BUTTON_LED_SUB_WAIT_RELEASE_SETTLE,
    BUTTON_LED_SUB_WAIT_VISUAL_DECISION,
    BUTTON_LED_SUB_DONE
} button_led_sub_state_t;

typedef enum
{
    BUTTON_LED_REPLY_PENDING = 0,
    BUTTON_LED_REPLY_VALID,
    BUTTON_LED_REPLY_NEGATIVE,
    BUTTON_LED_REPLY_INVALID
} button_led_reply_parse_status_t;

typedef struct
{
    button_led_test_item_t item;
    button_led_test_step_t pressStep;
    button_led_test_step_t releaseStep;
    bool                   isControlButton;
    uint8_t                gpioIndex;
    const char*            pressToken;
    const char*            pressAck;
    const char*            pressNak;
    const char*            releaseToken;
    const char*            releaseAck;
    const char*            releaseNak;
} button_led_button_def_t;

typedef struct
{
    button_led_sub_state_t          subState;
    button_led_test_step_t          currentStep;
    button_led_test_step_t          pendingStep;
    uint8_t                         currentButtonIndex;
    uint8_t                         attempt;
    uint32_t                        timestamp;
    uint32_t                        buttonPressStartMs;
    uint32_t                        buttonDetectStartMs;
    uint32_t                        nextButtonPollAtMs;
    uint32_t                        replyTimeoutMs;
    uint8_t                         maxAttempts;
    bool                            txError;
    bool                            fixtureButtonActive;
    bool                            ledRotationActive;
    bool                            visualResultSubmitted;
    bool                            visualResultGood;
    bool                            visualReadyProgressPending;

    char                            commandToken[24];
    char                            expectedAck[24];
    char                            negativeAck[24];
    uint8_t                         rxLen;
    char                            rxBuf[BUTTON_LED_RX_BUF_LEN];
    button_led_reply_parse_status_t replyStatus;

    button_led_test_progress_t      progress;
    bool                            hasProgress;
    button_led_test_progress_t      pendingProgress;
    bool                            hasPendingProgress;

    button_led_test_result_t        result;
    bool                            hasResult;
} button_led_test_ctx_t;

static const button_led_button_def_t s_buttonDefs[] = {
    {
        BUTTON_LED_TEST_ITEM_USR0,
        BUTTON_LED_TEST_STEP_USR0_PRESS,
        BUTTON_LED_TEST_STEP_USR0_RELEASE,
        true,
        0u,
        "BTN_USR0_PRESS",
        "OK_BTN_USR0_PRESS",
        "BTN_USR0_PRESS_HIGH",
        "BTN_USR0_RELEASE",
        "OK_BTN_USR0_RELEASE",
        "BTN_USR0_RELEASE_LOW"
    },
    {
        BUTTON_LED_TEST_ITEM_PNL0,
        BUTTON_LED_TEST_STEP_PNL0_PRESS,
        BUTTON_LED_TEST_STEP_PNL0_RELEASE,
        false,
        0u,
        "BTN_PNL0_PRESS",
        "OK_BTN_PNL0_PRESS",
        "BTN_PNL0_PRESS_HIGH",
        "BTN_PNL0_RELEASE",
        "OK_BTN_PNL0_RELEASE",
        "BTN_PNL0_RELEASE_LOW"
    },
    {
        BUTTON_LED_TEST_ITEM_PNL1,
        BUTTON_LED_TEST_STEP_PNL1_PRESS,
        BUTTON_LED_TEST_STEP_PNL1_RELEASE,
        false,
        1u,
        "BTN_PNL1_PRESS",
        "OK_BTN_PNL1_PRESS",
        "BTN_PNL1_PRESS_HIGH",
        "BTN_PNL1_RELEASE",
        "OK_BTN_PNL1_RELEASE",
        "BTN_PNL1_RELEASE_LOW"
    },
    {
        BUTTON_LED_TEST_ITEM_PNL2,
        BUTTON_LED_TEST_STEP_PNL2_PRESS,
        BUTTON_LED_TEST_STEP_PNL2_RELEASE,
        false,
        2u,
        "BTN_PNL2_PRESS",
        "OK_BTN_PNL2_PRESS",
        "BTN_PNL2_PRESS_HIGH",
        "BTN_PNL2_RELEASE",
        "OK_BTN_PNL2_RELEASE",
        "BTN_PNL2_RELEASE_LOW"
    },
    {
        BUTTON_LED_TEST_ITEM_PNL3,
        BUTTON_LED_TEST_STEP_PNL3_PRESS,
        BUTTON_LED_TEST_STEP_PNL3_RELEASE,
        false,
        3u,
        "BTN_PNL3_PRESS",
        "OK_BTN_PNL3_PRESS",
        "BTN_PNL3_PRESS_HIGH",
        "BTN_PNL3_RELEASE",
        "OK_BTN_PNL3_RELEASE",
        "BTN_PNL3_RELEASE_LOW"
    }
};

static button_led_test_ctx_t ctx;

static bool isPressStep(button_led_test_step_t step);
static bool isReleaseStep(button_led_test_step_t step);
static bool isButtonStep(button_led_test_step_t step);
static uint32_t currentButtonActionTimeoutMs(void);
static void queueCommand(button_led_test_step_t step);
static void beginManualButtonPressWait(void);
static void driveCurrentFixtureButton(GPIO_PinState state);
static void releaseCurrentFixtureButton(void);
static void beginReleaseDetection(button_led_test_step_t releaseStep);
static void handleNegativeButtonReply(void);

static const char* reportStepName(button_led_test_step_t step)
{
    switch (step)
    {
    case BUTTON_LED_TEST_STEP_BUTTONS_INIT:  return "Buttons Init";
    case BUTTON_LED_TEST_STEP_LED_BEGIN:     return "LED Begin";
    case BUTTON_LED_TEST_STEP_LED_ROTATE:    return "LED Rotate";
    case BUTTON_LED_TEST_STEP_USR0_PRESS:
    case BUTTON_LED_TEST_STEP_USR0_RELEASE:  return "USR_0 Button";
    case BUTTON_LED_TEST_STEP_PNL0_PRESS:
    case BUTTON_LED_TEST_STEP_PNL0_RELEASE:  return "PNL_0 Button";
    case BUTTON_LED_TEST_STEP_PNL1_PRESS:
    case BUTTON_LED_TEST_STEP_PNL1_RELEASE:  return "PNL_1 Button";
    case BUTTON_LED_TEST_STEP_PNL2_PRESS:
    case BUTTON_LED_TEST_STEP_PNL2_RELEASE:  return "PNL_2 Button";
    case BUTTON_LED_TEST_STEP_PNL3_PRESS:
    case BUTTON_LED_TEST_STEP_PNL3_RELEASE:  return "PNL_3 Button";
    case BUTTON_LED_TEST_STEP_LED_STOP:      return "LED Stop";
    case BUTTON_LED_TEST_STEP_VISUAL_CHECK:  return "LED Visual Check";
    default:                                 return "Buttons / LEDs";
    }
}

static void recordLogicalButtonPass(button_led_test_item_t item)
{
    const char* testName = "Button";

    switch (item)
    {
    case BUTTON_LED_TEST_ITEM_USR0: testName = "USR_0 Button"; break;
    case BUTTON_LED_TEST_ITEM_PNL0: testName = "PNL_0 Button"; break;
    case BUTTON_LED_TEST_ITEM_PNL1: testName = "PNL_1 Button"; break;
    case BUTTON_LED_TEST_ITEM_PNL2: testName = "PNL_2 Button"; break;
    case BUTTON_LED_TEST_ITEM_PNL3: testName = "PNL_3 Button"; break;
    default:                        testName = "Button";       break;
    }

    (void)EOL_Report_AddRow(EOL_REPORT_ROW_PASS, testName, "Pressed and released", "Pressed and released");
}

static void recordVisualCheck(bool good)
{
    (void)EOL_Report_AddRow(good ? EOL_REPORT_ROW_PASS : EOL_REPORT_ROW_FAIL,
                            "LED Visual Check",
                            "Operator pressed GOOD",
                            good ? "Operator pressed GOOD" : "Operator pressed BAD");
}

static void recordStepFailureRow(button_led_test_failure_kind_t failKind)
{
    const char* expected = ctx.expectedAck;
    const char* actual = "No response";

    if (failKind == BUTTON_LED_TEST_FAIL_INVALID_REPLY)
    {
        actual = "Invalid reply";
    }
    else if (failKind == BUTTON_LED_TEST_FAIL_STATE_MISMATCH)
    {
        expected = ctx.result.expectedToken;
        actual = ctx.result.actualText;
    }
    else if (failKind == BUTTON_LED_TEST_FAIL_TIMEOUT)
    {
        expected = "Pressed and released";
        actual = isPressStep(ctx.currentStep) ? "Press timeout" : "Release timeout";
    }
    else if (failKind == BUTTON_LED_TEST_FAIL_OPERATOR_REJECTED)
    {
        expected = "Operator pressed GOOD";
        actual = "Operator pressed BAD";
    }
    else if (ctx.result.actualText[0] != '\0')
    {
        actual = ctx.result.actualText;
    }

    if (isPressStep(ctx.currentStep) || isReleaseStep(ctx.currentStep))
    {
        expected = "Pressed and released";
    }

    (void)EOL_Report_AddRow(EOL_REPORT_ROW_FAIL, reportStepName(ctx.currentStep), expected, actual);
    EOL_Report_FinalizeFail();
}

static void allFixtureButtonsLow(void)
{
    GPIO_Config_WriteCtrlBtn(0u, GPIO_PIN_RESET);
    GPIO_Config_WriteAuxBtn(0u, GPIO_PIN_RESET);
    GPIO_Config_WriteAuxBtn(1u, GPIO_PIN_RESET);
    GPIO_Config_WriteAuxBtn(2u, GPIO_PIN_RESET);
    GPIO_Config_WriteAuxBtn(3u, GPIO_PIN_RESET);
}

static void UART4_ClearRxStaleState(void)
{
    UART4_ClearRxBuffer();
    __HAL_UART_CLEAR_FLAG(&huart4, UART_CLEAR_PEF | UART_CLEAR_FEF |
                                   UART_CLEAR_NEF | UART_CLEAR_OREF);
    __HAL_UART_FLUSH_DRREGISTER(&huart4);
    huart4.ErrorCode = HAL_UART_ERROR_NONE;
}

static void bestEffortStopLedRotation(void)
{
    static const char stopToken[] = "LED_STOP_ALL_ON";

    if (!ctx.ledRotationActive)
    {
        return;
    }

    UART4_ClearRxStaleState();
    (void)UART4_Transmit((const uint8_t*)stopToken,
                         (uint16_t)strlen(stopToken),
                         BUTTON_LED_TX_TIMEOUT_MS);
    UART4_ClearRxBuffer();
    ctx.ledRotationActive = false;
}

static void cleanupFixtureState(void)
{
    allFixtureButtonsLow();
    ctx.fixtureButtonActive = false;
    bestEffortStopLedRotation();
}

static void resetReplyContext(void)
{
    ctx.rxLen = 0u;
    memset(ctx.rxBuf, 0, sizeof(ctx.rxBuf));
    ctx.txError = false;
    ctx.replyStatus = BUTTON_LED_REPLY_PENDING;
}

static void postProgress(button_led_test_item_t item, button_led_test_progress_kind_t kind)
{
    if (!ctx.hasProgress)
    {
        ctx.progress.item = item;
        ctx.progress.kind = kind;
        ctx.hasProgress = true;
        return;
    }

    /* Preserve event order when two progress events occur in one tick. */
    ctx.pendingProgress.item = item;
    ctx.pendingProgress.kind = kind;
    ctx.hasPendingProgress = true;
}

static void postReadyProgress(button_led_test_item_t item)
{
    postProgress(item, BUTTON_LED_PROGRESS_READY);
}

static void postPassProgress(button_led_test_item_t item)
{
    postProgress(item, BUTTON_LED_PROGRESS_PASS);
}

static void postVisualReadyProgress(void)
{
    postProgress(BUTTON_LED_TEST_ITEM_VISUAL_READY, BUTTON_LED_PROGRESS_VISUAL_READY);
}

static void postPassResult(void)
{
    memset(&ctx.result, 0, sizeof(ctx.result));
    ctx.result.passed = true;
    ctx.result.errorCode = EOL_ERR_NONE;
    ctx.result.failureKind = BUTTON_LED_TEST_FAIL_NONE;
    EOL_Report_FinalizePass();
    cleanupFixtureState();
    ctx.hasResult = true;
}

static void postOperatorRejectedResult(void)
{
    memset(&ctx.result, 0, sizeof(ctx.result));
    ctx.result.passed = false;
    ctx.result.failedStep = BUTTON_LED_TEST_STEP_VISUAL_CHECK;
    ctx.result.failureKind = BUTTON_LED_TEST_FAIL_OPERATOR_REJECTED;
    ctx.result.errorCode = EOL_ERR_BUTTON_LED_OPERATOR_REJECTED;
    strncpy(ctx.result.expectedToken, "Operator pressed GOOD",
            sizeof(ctx.result.expectedToken) - 1u);
    strncpy(ctx.result.actualText, "Operator pressed BAD",
            sizeof(ctx.result.actualText) - 1u);
    recordVisualCheck(false);
    EOL_Report_FinalizeFail();
    cleanupFixtureState();
    ctx.hasResult = true;
}

static void postTimeoutResult(const char* expectedText, const char* actualText)
{
    memset(&ctx.result, 0, sizeof(ctx.result));
    ctx.result.passed = false;
    ctx.result.failedStep = ctx.currentStep;
    ctx.result.failureKind = BUTTON_LED_TEST_FAIL_TIMEOUT;
    ctx.result.errorCode = EOL_ERR_BUTTON_LED_TIMEOUT;
    strncpy(ctx.result.rawReply, ctx.rxBuf, sizeof(ctx.result.rawReply) - 1u);
    ctx.result.rawReply[sizeof(ctx.result.rawReply) - 1u] = '\0';
    strncpy(ctx.result.expectedToken, expectedText, sizeof(ctx.result.expectedToken) - 1u);
    ctx.result.expectedToken[sizeof(ctx.result.expectedToken) - 1u] = '\0';
    strncpy(ctx.result.actualText, actualText, sizeof(ctx.result.actualText) - 1u);
    ctx.result.actualText[sizeof(ctx.result.actualText) - 1u] = '\0';
    recordStepFailureRow(BUTTON_LED_TEST_FAIL_TIMEOUT);
    cleanupFixtureState();
    ctx.hasResult = true;
}

static void postMismatchResult(const char* expectedText, const char* actualText)
{
    memset(&ctx.result, 0, sizeof(ctx.result));
    ctx.result.passed = false;
    ctx.result.failedStep = ctx.currentStep;
    ctx.result.failureKind = BUTTON_LED_TEST_FAIL_STATE_MISMATCH;
    ctx.result.errorCode = EOL_ERR_BUTTON_LED_STATE_MISMATCH;
    strncpy(ctx.result.rawReply, ctx.rxBuf, sizeof(ctx.result.rawReply) - 1u);
    ctx.result.rawReply[sizeof(ctx.result.rawReply) - 1u] = '\0';
    strncpy(ctx.result.expectedToken, expectedText, sizeof(ctx.result.expectedToken) - 1u);
    ctx.result.expectedToken[sizeof(ctx.result.expectedToken) - 1u] = '\0';
    strncpy(ctx.result.actualText, actualText, sizeof(ctx.result.actualText) - 1u);
    ctx.result.actualText[sizeof(ctx.result.actualText) - 1u] = '\0';
    recordStepFailureRow(BUTTON_LED_TEST_FAIL_STATE_MISMATCH);
    cleanupFixtureState();
    ctx.hasResult = true;
}

static void postFailureResult(button_led_test_failure_kind_t failKind,
                              eol_error_code_t errorCode,
                              const char* actualText)
{
    memset(&ctx.result, 0, sizeof(ctx.result));
    ctx.result.passed = false;
    ctx.result.failedStep = ctx.currentStep;
    ctx.result.failureKind = failKind;
    ctx.result.errorCode = errorCode;
    strncpy(ctx.result.rawReply, ctx.rxBuf, sizeof(ctx.result.rawReply) - 1u);
    ctx.result.rawReply[sizeof(ctx.result.rawReply) - 1u] = '\0';
    strncpy(ctx.result.expectedToken, ctx.expectedAck, sizeof(ctx.result.expectedToken) - 1u);
    ctx.result.expectedToken[sizeof(ctx.result.expectedToken) - 1u] = '\0';
    if (actualText != 0)
    {
        strncpy(ctx.result.actualText, actualText, sizeof(ctx.result.actualText) - 1u);
        ctx.result.actualText[sizeof(ctx.result.actualText) - 1u] = '\0';
    }
    recordStepFailureRow(failKind);
    cleanupFixtureState();
    ctx.hasResult = true;
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

    while (ctx.rxLen < (BUTTON_LED_RX_BUF_LEN - 1u))
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
            ctx.replyStatus = BUTTON_LED_REPLY_INVALID;
            break;
        }

        ctx.rxBuf[ctx.rxLen++] = (char)byteIn;
        ctx.rxBuf[ctx.rxLen] = '\0';
    }
}

static button_led_reply_parse_status_t classifyReply(void)
{
    if (ctx.rxLen == 0u)
    {
        return BUTTON_LED_REPLY_PENDING;
    }

    if (strcmp(ctx.rxBuf, ctx.expectedAck) == 0)
    {
        return BUTTON_LED_REPLY_VALID;
    }

    if ((ctx.negativeAck[0] != '\0') && (strcmp(ctx.rxBuf, ctx.negativeAck) == 0))
    {
        return BUTTON_LED_REPLY_NEGATIVE;
    }

    if (isPrefixOfExpected(ctx.rxBuf, ctx.rxLen, ctx.expectedAck) ||
        ((ctx.negativeAck[0] != '\0') && isPrefixOfExpected(ctx.rxBuf, ctx.rxLen, ctx.negativeAck)))
    {
        return BUTTON_LED_REPLY_PENDING;
    }

    return BUTTON_LED_REPLY_INVALID;
}

static const button_led_button_def_t* currentButtonDef(void)
{
    if (ctx.currentButtonIndex >= (sizeof(s_buttonDefs) / sizeof(s_buttonDefs[0])))
    {
        return 0;
    }

    return &s_buttonDefs[ctx.currentButtonIndex];
}

static bool isPressStep(button_led_test_step_t step)
{
    return (step == BUTTON_LED_TEST_STEP_USR0_PRESS) ||
           (step == BUTTON_LED_TEST_STEP_PNL0_PRESS) ||
           (step == BUTTON_LED_TEST_STEP_PNL1_PRESS) ||
           (step == BUTTON_LED_TEST_STEP_PNL2_PRESS) ||
           (step == BUTTON_LED_TEST_STEP_PNL3_PRESS);
}

static bool isReleaseStep(button_led_test_step_t step)
{
    return (step == BUTTON_LED_TEST_STEP_USR0_RELEASE) ||
           (step == BUTTON_LED_TEST_STEP_PNL0_RELEASE) ||
           (step == BUTTON_LED_TEST_STEP_PNL1_RELEASE) ||
           (step == BUTTON_LED_TEST_STEP_PNL2_RELEASE) ||
           (step == BUTTON_LED_TEST_STEP_PNL3_RELEASE);
}

static bool isButtonStep(button_led_test_step_t step)
{
    return isPressStep(step) || isReleaseStep(step);
}

static uint32_t currentButtonActionTimeoutMs(void)
{
    if (isReleaseStep(ctx.currentStep))
    {
        return BUTTON_LED_RELEASE_ACTION_TIMEOUT_MS;
    }

    return BUTTON_LED_PRESS_ACTION_TIMEOUT_MS;
}

static void beginButtonDetection(button_led_test_step_t step)
{
    ctx.buttonDetectStartMs = HAL_GetTick();
    ctx.nextButtonPollAtMs = ctx.buttonDetectStartMs;
    queueCommand(step);
}

static void driveCurrentFixtureButton(GPIO_PinState state)
{
    const button_led_button_def_t* def = currentButtonDef();

    if (def == 0)
    {
        return;
    }

    if (def->isControlButton)
    {
        GPIO_Config_WriteCtrlBtn(def->gpioIndex, state);
    }
    else
    {
        GPIO_Config_WriteAuxBtn(def->gpioIndex, state);
    }
}

static void releaseCurrentFixtureButton(void)
{
    if (!ctx.fixtureButtonActive)
    {
        return;
    }

    driveCurrentFixtureButton(GPIO_PIN_RESET);
    ctx.fixtureButtonActive = false;
}

static void beginReleaseDetection(button_led_test_step_t releaseStep)
{
    releaseCurrentFixtureButton();
    ctx.buttonPressStartMs = HAL_GetTick();
    ctx.pendingStep = releaseStep;
    ctx.timestamp = HAL_GetTick();
    ctx.subState = BUTTON_LED_SUB_WAIT_RELEASE_SETTLE;
}

static void handleNegativeButtonReply(void)
{
    if (isPressStep(ctx.currentStep))
    {
        postMismatchResult("ESP32 reads LOW", "ESP32 saw HIGH");
    }
    else if (isReleaseStep(ctx.currentStep))
    {
        postMismatchResult("ESP32 reads HIGH", "ESP32 saw LOW");
    }
    else
    {
        postFailureResult(BUTTON_LED_TEST_FAIL_INVALID_REPLY,
                          EOL_ERR_BUTTON_LED_INVALID_REPLY,
                          "Unexpected negative ack");
    }

    ctx.subState = BUTTON_LED_SUB_DONE;
}

static void beginManualButtonPressWait(void)
{
    const button_led_button_def_t* def = currentButtonDef();

    if (def == 0)
    {
        postFailureResult(BUTTON_LED_TEST_FAIL_INVALID_REPLY,
                          EOL_ERR_BUTTON_LED_INVALID_REPLY,
                          "Invalid button map");
        ctx.subState = BUTTON_LED_SUB_DONE;
        return;
    }

    postReadyProgress(def->item);
    allFixtureButtonsLow();
    ctx.pendingStep = def->pressStep;
    ctx.timestamp = HAL_GetTick();
    ctx.subState = BUTTON_LED_SUB_DRIVE_BUTTON_HIGH;
}

static void handleButtonDetectionTimeout(void)
{
    if (isPressStep(ctx.currentStep))
    {
        postTimeoutResult("Automatic press detect within 1.5s", "Press timeout");
    }
    else if (isReleaseStep(ctx.currentStep))
    {
        postTimeoutResult("Automatic release detect within 2.0s", "Release timeout");
    }
    else
    {
        postFailureResult(BUTTON_LED_TEST_FAIL_NO_RESPONSE,
                          EOL_ERR_BUTTON_LED_NO_RESPONSE,
                          "No response");
    }

    ctx.subState = BUTTON_LED_SUB_DONE;
}
static void queueCommand(button_led_test_step_t step)
{
    const button_led_button_def_t* def = currentButtonDef();

    ctx.currentStep = step;
    memset(ctx.commandToken, 0, sizeof(ctx.commandToken));
    memset(ctx.expectedAck, 0, sizeof(ctx.expectedAck));
    memset(ctx.negativeAck, 0, sizeof(ctx.negativeAck));

    switch (step)
    {
    case BUTTON_LED_TEST_STEP_BUTTONS_INIT:
        strncpy(ctx.commandToken, "BTN_BEGIN", sizeof(ctx.commandToken) - 1u);
        strncpy(ctx.expectedAck, "OK_BTN_BEGIN", sizeof(ctx.expectedAck) - 1u);
        ctx.replyTimeoutMs = BUTTON_LED_INIT_TIMEOUT_MS;
        ctx.maxAttempts = BUTTON_LED_INIT_MAX_ATTEMPTS;
        break;

    case BUTTON_LED_TEST_STEP_LED_BEGIN:
        strncpy(ctx.commandToken, "LED_BEGIN", sizeof(ctx.commandToken) - 1u);
        strncpy(ctx.expectedAck, "OK_LED_BEGIN", sizeof(ctx.expectedAck) - 1u);
        ctx.replyTimeoutMs = BUTTON_LED_INIT_TIMEOUT_MS;
        ctx.maxAttempts = BUTTON_LED_INIT_MAX_ATTEMPTS;
        break;

    case BUTTON_LED_TEST_STEP_LED_ROTATE:
        strncpy(ctx.commandToken, "LED_ROTATE_250", sizeof(ctx.commandToken) - 1u);
        strncpy(ctx.expectedAck, "OK_LED_ROTATE_250", sizeof(ctx.expectedAck) - 1u);
        ctx.replyTimeoutMs = BUTTON_LED_INIT_TIMEOUT_MS;
        ctx.maxAttempts = BUTTON_LED_INIT_MAX_ATTEMPTS;
        break;

    case BUTTON_LED_TEST_STEP_LED_STOP:
        strncpy(ctx.commandToken, "LED_STOP_ALL_ON", sizeof(ctx.commandToken) - 1u);
        strncpy(ctx.expectedAck, "OK_LED_STOP_ALL_ON", sizeof(ctx.expectedAck) - 1u);
        ctx.replyTimeoutMs = BUTTON_LED_INIT_TIMEOUT_MS;
        ctx.maxAttempts = BUTTON_LED_INIT_MAX_ATTEMPTS;
        break;

    case BUTTON_LED_TEST_STEP_USR0_PRESS:
    case BUTTON_LED_TEST_STEP_PNL0_PRESS:
    case BUTTON_LED_TEST_STEP_PNL1_PRESS:
    case BUTTON_LED_TEST_STEP_PNL2_PRESS:
    case BUTTON_LED_TEST_STEP_PNL3_PRESS:
        if (def != 0)
        {
            strncpy(ctx.commandToken, def->pressToken, sizeof(ctx.commandToken) - 1u);
            strncpy(ctx.expectedAck, def->pressAck, sizeof(ctx.expectedAck) - 1u);
            strncpy(ctx.negativeAck, def->pressNak, sizeof(ctx.negativeAck) - 1u);
        }
        ctx.replyTimeoutMs = BUTTON_LED_BUTTON_POLL_TIMEOUT_MS;
        ctx.maxAttempts = BUTTON_LED_BUTTON_MAX_ATTEMPTS;
        break;

    case BUTTON_LED_TEST_STEP_USR0_RELEASE:
    case BUTTON_LED_TEST_STEP_PNL0_RELEASE:
    case BUTTON_LED_TEST_STEP_PNL1_RELEASE:
    case BUTTON_LED_TEST_STEP_PNL2_RELEASE:
    case BUTTON_LED_TEST_STEP_PNL3_RELEASE:
        if (def != 0)
        {
            strncpy(ctx.commandToken, def->releaseToken, sizeof(ctx.commandToken) - 1u);
            strncpy(ctx.expectedAck, def->releaseAck, sizeof(ctx.expectedAck) - 1u);
            strncpy(ctx.negativeAck, def->releaseNak, sizeof(ctx.negativeAck) - 1u);
        }
        ctx.replyTimeoutMs = BUTTON_LED_BUTTON_POLL_TIMEOUT_MS;
        ctx.maxAttempts = BUTTON_LED_BUTTON_MAX_ATTEMPTS;
        break;

    default:
        break;
    }

    ctx.attempt = 1u;
    ctx.subState = BUTTON_LED_SUB_SEND_COMMAND;
}

static void retryOrFail(button_led_test_failure_kind_t failKind)
{
    if (ctx.attempt < ctx.maxAttempts)
    {
        ctx.attempt++;
        ctx.subState = BUTTON_LED_SUB_SEND_COMMAND;
        return;
    }

    if (failKind == BUTTON_LED_TEST_FAIL_INVALID_REPLY)
    {
        postFailureResult(failKind, EOL_ERR_BUTTON_LED_INVALID_REPLY, "Invalid reply");
    }
    else
    {
        postFailureResult(failKind, EOL_ERR_BUTTON_LED_NO_RESPONSE, "No response");
    }

    ctx.subState = BUTTON_LED_SUB_DONE;
}

static void handleCommandSuccess(void)
{
    const button_led_button_def_t* def = currentButtonDef();

    switch (ctx.currentStep)
    {
    case BUTTON_LED_TEST_STEP_BUTTONS_INIT:
        queueCommand(BUTTON_LED_TEST_STEP_LED_BEGIN);
        break;

    case BUTTON_LED_TEST_STEP_LED_BEGIN:
        queueCommand(BUTTON_LED_TEST_STEP_LED_ROTATE);
        break;

    case BUTTON_LED_TEST_STEP_LED_ROTATE:
        ctx.ledRotationActive = true;
        beginManualButtonPressWait();
        break;

    case BUTTON_LED_TEST_STEP_USR0_PRESS:
    case BUTTON_LED_TEST_STEP_PNL0_PRESS:
    case BUTTON_LED_TEST_STEP_PNL1_PRESS:
    case BUTTON_LED_TEST_STEP_PNL2_PRESS:
    case BUTTON_LED_TEST_STEP_PNL3_PRESS:
        if (def == 0)
        {
            postFailureResult(BUTTON_LED_TEST_FAIL_INVALID_REPLY,
                              EOL_ERR_BUTTON_LED_INVALID_REPLY,
                              "Invalid button map");
            ctx.subState = BUTTON_LED_SUB_DONE;
        }
        else
        {
            beginReleaseDetection(def->releaseStep);
        }
        break;

    case BUTTON_LED_TEST_STEP_USR0_RELEASE:
    case BUTTON_LED_TEST_STEP_PNL0_RELEASE:
    case BUTTON_LED_TEST_STEP_PNL1_RELEASE:
    case BUTTON_LED_TEST_STEP_PNL2_RELEASE:
    case BUTTON_LED_TEST_STEP_PNL3_RELEASE:
        if (def != 0)
        {
            recordLogicalButtonPass(def->item);
            postPassProgress(def->item);
        }
        ctx.currentButtonIndex++;
        if (ctx.currentButtonIndex < (sizeof(s_buttonDefs) / sizeof(s_buttonDefs[0])))
        {
            beginManualButtonPressWait();
        }
        else
        {
            ctx.visualReadyProgressPending = true;
            ctx.subState = BUTTON_LED_SUB_WAIT_VISUAL_DECISION;
        }
        break;

    case BUTTON_LED_TEST_STEP_LED_STOP:
        ctx.ledRotationActive = false;
        if (ctx.visualResultGood)
        {
            recordVisualCheck(true);
            postPassResult();
        }
        else
        {
            postOperatorRejectedResult();
        }
        ctx.subState = BUTTON_LED_SUB_DONE;
        break;

    default:
        break;
    }
}

void ButtonLedTest_Start(void)
{
    memset(&ctx, 0, sizeof(ctx));

    allFixtureButtonsLow();
    UART4_ClearRxStaleState();
    queueCommand(BUTTON_LED_TEST_STEP_BUTTONS_INIT);
}

void ButtonLedTest_Tick(void)
{
    if (ctx.visualReadyProgressPending && !ctx.hasProgress)
    {
        ctx.visualReadyProgressPending = false;
        postVisualReadyProgress();
    }

    switch (ctx.subState)
    {
    case BUTTON_LED_SUB_IDLE:
    case BUTTON_LED_SUB_DONE:
        break;

    case BUTTON_LED_SUB_SEND_COMMAND:
        UART4_ClearRxStaleState();
        resetReplyContext();

        if (UART4_Transmit((const uint8_t*)ctx.commandToken,
                           (uint16_t)strlen(ctx.commandToken),
                           BUTTON_LED_TX_TIMEOUT_MS) != HAL_OK)
        {
            ctx.txError = true;
            retryOrFail(BUTTON_LED_TEST_FAIL_NO_RESPONSE);
            break;
        }

        ctx.timestamp = HAL_GetTick();
        ctx.subState = BUTTON_LED_SUB_WAIT_REPLY;
        break;

    case BUTTON_LED_SUB_WAIT_REPLY:
        if (ctx.replyStatus != BUTTON_LED_REPLY_INVALID)
        {
            pollReplyBytes();
        }

        if (ctx.replyStatus != BUTTON_LED_REPLY_INVALID)
        {
            ctx.replyStatus = classifyReply();
        }

        if (ctx.replyStatus == BUTTON_LED_REPLY_VALID)
        {
            handleCommandSuccess();
        }
        else if (ctx.replyStatus == BUTTON_LED_REPLY_NEGATIVE)
        {
            handleNegativeButtonReply();
        }
        else if (ctx.replyStatus == BUTTON_LED_REPLY_INVALID)
        {
            retryOrFail(BUTTON_LED_TEST_FAIL_INVALID_REPLY);
        }
        else if (isButtonStep(ctx.currentStep) &&
                 ((HAL_GetTick() - ctx.buttonPressStartMs) >= currentButtonActionTimeoutMs()))
        {
            handleButtonDetectionTimeout();
        }
        else if ((HAL_GetTick() - ctx.timestamp) >= ctx.replyTimeoutMs)
        {
            if (isButtonStep(ctx.currentStep))
            {
                ctx.attempt = 1u;
                ctx.nextButtonPollAtMs = HAL_GetTick() + BUTTON_LED_BUTTON_POLL_PERIOD_MS;
                ctx.subState = BUTTON_LED_SUB_WAIT_BUTTON_RETRY;
            }
            else
            {
                retryOrFail(BUTTON_LED_TEST_FAIL_NO_RESPONSE);
            }
        }
        break;

    case BUTTON_LED_SUB_WAIT_BUTTON_RETRY:
        if ((HAL_GetTick() - ctx.buttonPressStartMs) >= currentButtonActionTimeoutMs())
        {
            handleButtonDetectionTimeout();
        }
        else if ((HAL_GetTick() - ctx.nextButtonPollAtMs) < 0x80000000u)
        {
            ctx.attempt = 1u;
            ctx.subState = BUTTON_LED_SUB_SEND_COMMAND;
        }
        break;

    case BUTTON_LED_SUB_DRIVE_BUTTON_HIGH:
        if ((HAL_GetTick() - ctx.timestamp) >= BUTTON_LED_ASSIST_PRE_PRESS_DELAY_MS)
        {
            allFixtureButtonsLow();
            driveCurrentFixtureButton(GPIO_PIN_SET);
            ctx.fixtureButtonActive = true;
            ctx.buttonPressStartMs = HAL_GetTick();
            ctx.subState = BUTTON_LED_SUB_WAIT_PRESS_SETTLE;
        }
        break;

    case BUTTON_LED_SUB_WAIT_PRESS_SETTLE:
        if ((HAL_GetTick() - ctx.buttonPressStartMs) >= BUTTON_LED_ASSIST_PRESS_SETTLE_MS)
        {
            beginButtonDetection(ctx.pendingStep);
        }
        break;

    case BUTTON_LED_SUB_HOLD_BUTTON_HIGH:
        break;

    case BUTTON_LED_SUB_DRIVE_BUTTON_LOW:
        break;

    case BUTTON_LED_SUB_WAIT_RELEASE_SETTLE:
        if ((HAL_GetTick() - ctx.timestamp) >= BUTTON_LED_ASSIST_RELEASE_SETTLE_MS)
        {
            beginButtonDetection(ctx.pendingStep);
        }
        break;

    case BUTTON_LED_SUB_WAIT_VISUAL_DECISION:
        if (ctx.visualResultSubmitted)
        {
            ctx.visualResultSubmitted = false;
            queueCommand(BUTTON_LED_TEST_STEP_LED_STOP);
        }
        break;

    default:
        break;
    }
}

void ButtonLedTest_SubmitVisualResult(bool good)
{
    if ((ctx.subState != BUTTON_LED_SUB_WAIT_VISUAL_DECISION) || ctx.hasResult)
    {
        return;
    }

    ctx.visualResultGood = good;
    ctx.visualResultSubmitted = true;
}

bool ButtonLedTest_IsWaitingForVisualDecision(void)
{
    return (ctx.subState == BUTTON_LED_SUB_WAIT_VISUAL_DECISION) && !ctx.hasResult;
}

bool ButtonLedTest_HasProgressEvent(void)
{
    return ctx.hasProgress;
}

button_led_test_progress_t ButtonLedTest_GetProgressEvent(void)
{
    button_led_test_progress_t event = ctx.progress;

    if (ctx.hasPendingProgress)
    {
        ctx.progress = ctx.pendingProgress;
        ctx.hasPendingProgress = false;
        ctx.hasProgress = true;
    }
    else
    {
        ctx.hasProgress = false;
    }

    return event;
}

bool ButtonLedTest_HasResult(void)
{
    return ctx.hasResult;
}

button_led_test_result_t ButtonLedTest_GetResult(void)
{
    ctx.hasResult = false;
    return ctx.result;
}
