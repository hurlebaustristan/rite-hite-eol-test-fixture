#ifndef BUTTON_LED_TEST_H
#define BUTTON_LED_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "eol_test_protocol.h"

typedef enum
{
    BUTTON_LED_TEST_ITEM_USR0 = 0,
    BUTTON_LED_TEST_ITEM_PNL0,
    BUTTON_LED_TEST_ITEM_PNL1,
    BUTTON_LED_TEST_ITEM_PNL2,
    BUTTON_LED_TEST_ITEM_PNL3,
    BUTTON_LED_TEST_ITEM_VISUAL_READY
} button_led_test_item_t;

typedef enum
{
    BUTTON_LED_TEST_STEP_NONE = 0,
    BUTTON_LED_TEST_STEP_BUTTONS_INIT,
    BUTTON_LED_TEST_STEP_LED_BEGIN,
    BUTTON_LED_TEST_STEP_LED_ROTATE,
    BUTTON_LED_TEST_STEP_USR0_PRESS,
    BUTTON_LED_TEST_STEP_USR0_RELEASE,
    BUTTON_LED_TEST_STEP_PNL0_PRESS,
    BUTTON_LED_TEST_STEP_PNL0_RELEASE,
    BUTTON_LED_TEST_STEP_PNL1_PRESS,
    BUTTON_LED_TEST_STEP_PNL1_RELEASE,
    BUTTON_LED_TEST_STEP_PNL2_PRESS,
    BUTTON_LED_TEST_STEP_PNL2_RELEASE,
    BUTTON_LED_TEST_STEP_PNL3_PRESS,
    BUTTON_LED_TEST_STEP_PNL3_RELEASE,
    BUTTON_LED_TEST_STEP_LED_STOP,
    BUTTON_LED_TEST_STEP_VISUAL_CHECK
} button_led_test_step_t;

typedef enum
{
    BUTTON_LED_PROGRESS_READY = 0,
    BUTTON_LED_PROGRESS_PASS,
    BUTTON_LED_PROGRESS_VISUAL_READY
} button_led_test_progress_kind_t;

typedef enum
{
    BUTTON_LED_TEST_FAIL_NONE = 0,
    BUTTON_LED_TEST_FAIL_NO_RESPONSE,
    BUTTON_LED_TEST_FAIL_INVALID_REPLY,
    BUTTON_LED_TEST_FAIL_OPERATOR_REJECTED,
    BUTTON_LED_TEST_FAIL_TIMEOUT,
    BUTTON_LED_TEST_FAIL_STATE_MISMATCH
} button_led_test_failure_kind_t;

typedef struct
{
    button_led_test_item_t          item;
    button_led_test_progress_kind_t kind;
} button_led_test_progress_t;

typedef struct
{
    bool                           passed;
    button_led_test_step_t         failedStep;
    button_led_test_failure_kind_t failureKind;
    eol_error_code_t               errorCode;
    char                           rawReply[24];
    char                           expectedToken[24];
    char                           actualText[32];
} button_led_test_result_t;

void ButtonLedTest_Start(void);
void ButtonLedTest_Tick(void);
void ButtonLedTest_SubmitVisualResult(bool good);
bool ButtonLedTest_IsWaitingForVisualDecision(void);

bool ButtonLedTest_HasProgressEvent(void);
button_led_test_progress_t ButtonLedTest_GetProgressEvent(void);

bool ButtonLedTest_HasResult(void);
button_led_test_result_t ButtonLedTest_GetResult(void);

#ifdef __cplusplus
}
#endif

#endif /* BUTTON_LED_TEST_H */
