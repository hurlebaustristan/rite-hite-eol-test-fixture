/**
  ******************************************************************************
  * @file    output_test.h
  * @brief   Non-blocking Outputs test engine for Outputs_Screen.
  *******************************************************************************
  */

#ifndef OUTPUT_TEST_H
#define OUTPUT_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "eol_test_protocol.h"

typedef enum
{
    OUTPUT_TEST_ITEM_DO1 = 0,
    OUTPUT_TEST_ITEM_DO2,
    OUTPUT_TEST_ITEM_K1_NO,
    OUTPUT_TEST_ITEM_K1_NC,
    OUTPUT_TEST_ITEM_K2_NO,
    OUTPUT_TEST_ITEM_K2_NC
} output_test_item_t;

typedef enum
{
    OUTPUT_TEST_PHASE_NONE = 0,
    OUTPUT_TEST_PHASE_K1_ON,
    OUTPUT_TEST_PHASE_K1_OFF,
    OUTPUT_TEST_PHASE_K2_ON,
    OUTPUT_TEST_PHASE_K2_OFF
} output_test_phase_t;

typedef enum
{
    OUTPUT_TEST_FAIL_NONE = 0,
    OUTPUT_TEST_FAIL_NO_RESPONSE,
    OUTPUT_TEST_FAIL_INVALID_REPLY,
    OUTPUT_TEST_FAIL_STATE_MISMATCH
} output_test_failure_kind_t;

typedef struct
{
    output_test_item_t itemPassed;
} output_test_progress_t;

typedef struct
{
    bool                       passed;
    output_test_phase_t        failedPhase;
    output_test_failure_kind_t failureKind;
    eol_error_code_t           errorCode;
    char                       rawReply[16];
    char                       expectedState[32];
    char                       actualState[32];
} output_test_result_t;

void OutputTest_Start(void);
void OutputTest_Tick(void);

bool OutputTest_HasProgressEvent(void);
output_test_progress_t OutputTest_GetProgressEvent(void);

bool OutputTest_HasResult(void);
output_test_result_t OutputTest_GetResult(void);

#ifdef __cplusplus
}
#endif

#endif /* OUTPUT_TEST_H */
