#ifndef AUTO_TEST_H
#define AUTO_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef enum
{
    AUTO_TEST_PHASE_NONE = 0,
    AUTO_TEST_PHASE_COMMS,
    AUTO_TEST_PHASE_DIGITAL_INPUTS,
    AUTO_TEST_PHASE_ANALOG_INPUTS,
    AUTO_TEST_PHASE_DIGITAL_OUTPUTS,
    AUTO_TEST_PHASE_RELAY_OUTPUTS,
    AUTO_TEST_PHASE_BUTTONS,
    AUTO_TEST_PHASE_LEDS
} auto_test_phase_t;

void AutoTest_RequestStart(void);
bool AutoTest_ConsumeStartRequest(void);
bool AutoTest_IsAutoMode(void);
void AutoTest_SetPhase(auto_test_phase_t phase);
auto_test_phase_t AutoTest_GetPhase(void);
const char* AutoTest_GetPhaseToken(void);
void AutoTest_ClearAutoMode(void);

#ifdef __cplusplus
}
#endif

#endif /* AUTO_TEST_H */
