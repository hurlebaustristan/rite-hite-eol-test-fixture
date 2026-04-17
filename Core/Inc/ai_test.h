/**
  ******************************************************************************
  * @file    ai_test.h
  * @brief   Non-blocking Analog Inputs 1..4 test engine for Inputs_Screen.
  ******************************************************************************
  */

#ifndef AI_TEST_H
#define AI_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "eol_test_protocol.h"

typedef enum
{
    AI_TEST_LEVEL_LOW = 0,
    AI_TEST_LEVEL_MID,
    AI_TEST_LEVEL_HIGH
} ai_test_level_t;

typedef enum
{
    AI_TEST_KIND_CURRENT = 0, /* AI1/AI2 */
    AI_TEST_KIND_VOLTAGE      /* AI3/AI4 */
} ai_test_channel_kind_t;

typedef enum
{
    AI_TEST_FAIL_NONE = 0,
    AI_TEST_FAIL_NO_RESPONSE,
    AI_TEST_FAIL_INVALID_REPLY,
    AI_TEST_FAIL_OUT_OF_RANGE
} ai_test_failure_kind_t;

typedef struct
{
    uint8_t channelPassed; /* 1..4 */
} ai_test_progress_t;

typedef struct
{
    bool                    passed;
    uint8_t                 failedChannel; /* 1..4 if failed */
    ai_test_level_t         failedLevel;
    ai_test_channel_kind_t  kind;
    ai_test_failure_kind_t  failureKind;
    float                   measuredAdcVolts;
    float                   measuredEngineering;
    float                   expectedEngineering;
    float                   toleranceEngineering;
    eol_error_code_t        errorCode;
} ai_test_result_t;

void AITest_Start(void);
void AITest_Tick(void);

bool AITest_HasProgressEvent(void);
ai_test_progress_t AITest_GetProgressEvent(void);

bool AITest_HasResult(void);
ai_test_result_t AITest_GetResult(void);

#ifdef __cplusplus
}
#endif

#endif /* AI_TEST_H */
