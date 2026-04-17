/**
  ******************************************************************************
  * @file    di_test.h
  * @brief   Non-blocking Digital Inputs 1..12 test engine for Inputs_Screen.
  *
  *          Sequence per channel:
  *            1) Send "DIxTEST"
  *            2) Drive DIx HIGH for 1 s
  *            3) Drive DIx LOW  for 1 s
  *            4) Wait for "DIxOK" / "DIxFAIL"
  *
  *          Designed to run from Model::tick() without blocking the UI.
  ******************************************************************************
  */

#ifndef DI_TEST_H
#define DI_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "eol_test_protocol.h"

typedef enum
{
    DI_TEST_FAIL_NONE = 0,
    DI_TEST_FAIL_NO_RESPONSE,
    DI_TEST_FAIL_FAIL_TOKEN,
    DI_TEST_FAIL_INVALID_REPLY
} di_test_failure_kind_t;

typedef struct
{
    uint8_t channelPassed; /* 1..12 */
} di_test_progress_t;

typedef struct
{
    bool                  passed;
    uint8_t               failedChannel; /* 1..12 when passed=false */
    di_test_failure_kind_t failureKind;
    eol_error_code_t      errorCode;
    char                  rawReply[16];
} di_test_result_t;

void DITest_Start(void);
void DITest_Tick(void);

bool DITest_HasProgressEvent(void);
di_test_progress_t DITest_GetProgressEvent(void);

bool DITest_HasResult(void);
di_test_result_t DITest_GetResult(void);

#ifdef __cplusplus
}
#endif

#endif /* DI_TEST_H */
