/**
  ******************************************************************************
  * @file    comm_test.h
  * @brief   Communication test engine for the Comm_Reset_Screen.
  *
  *          Runs a non-blocking state machine driven from Model::tick().
  *          Each tick advances the current test stage and reports results
  *          back to the UI through a polling interface.
  *
  *          Stage 1 — COMM:  Send PING, expect PONG (up to 3 retries).
  *
  *          Future stages (MCU_Reset, Fault_Check) are stubbed and will
  *          be implemented when the corresponding screens are wired up.
  ******************************************************************************
  */

#ifndef COMM_TEST_H
#define COMM_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "eol_test_protocol.h"
#include <stdbool.h>

/* ======================================================================== */
/*  Public API                                                              */
/* ======================================================================== */

/**
  * @brief  Start (or restart) the communication-test sequence.
  *         Resets internal state and begins at EOL_STAGE_COMM.
  */
void CommTest_Start(void);

/**
  * @brief  Call once per TouchGFX tick (~16 ms at 60 fps).
  *         Advances the internal state machine one step.
  *
  *         After each call, use CommTest_HasResult() / CommTest_GetResult()
  *         to check whether a stage just completed or failed.
  */
void CommTest_Tick(void);

/**
  * @brief  Returns true when CommTest_Tick() has produced a new result
  *         that has not yet been consumed by CommTest_GetResult().
  */
bool CommTest_HasResult(void);

/**
  * @brief  Consume and return the latest result.
  *         Clears the "has result" flag.
  */
eol_test_result_t CommTest_GetResult(void);

/**
  * @brief  Returns true when a non-terminal progress event is pending.
  */
bool CommTest_HasProgressEvent(void);

/**
  * @brief  Consume and return the latest progress event.
  *         Clears the "has progress event" flag.
  */
eol_progress_event_t CommTest_GetProgressEvent(void);

/**
  * @brief  Return the stage the engine is currently executing.
  */
eol_stage_t CommTest_CurrentStage(void);

#ifdef __cplusplus
}
#endif

#endif /* COMM_TEST_H */
