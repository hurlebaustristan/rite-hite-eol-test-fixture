#include "auto_test.h"

static volatile bool startRequested;
static volatile bool autoMode;
static volatile auto_test_phase_t currentPhase;

static const char* phaseToken(auto_test_phase_t phase)
{
    switch (phase)
    {
    case AUTO_TEST_PHASE_COMMS:            return "COMMS";
    case AUTO_TEST_PHASE_DIGITAL_INPUTS:   return "DIGITAL_INPUTS";
    case AUTO_TEST_PHASE_ANALOG_INPUTS:    return "ANALOG_INPUTS";
    case AUTO_TEST_PHASE_DIGITAL_OUTPUTS:  return "DIGITAL_OUTPUTS";
    case AUTO_TEST_PHASE_RELAY_OUTPUTS:    return "RELAY_OUTPUTS";
    case AUTO_TEST_PHASE_BUTTONS:          return "BUTTONS";
    case AUTO_TEST_PHASE_LEDS:             return "LEDS";
    default:                               return "";
    }
}

void AutoTest_RequestStart(void)
{
    startRequested = true;
    currentPhase = AUTO_TEST_PHASE_NONE;
}

bool AutoTest_ConsumeStartRequest(void)
{
    if (startRequested)
    {
        startRequested = false;
        autoMode = true;
        currentPhase = AUTO_TEST_PHASE_COMMS;
        return true;
    }
    return false;
}

bool AutoTest_IsAutoMode(void)
{
    return autoMode;
}

void AutoTest_SetPhase(auto_test_phase_t phase)
{
    currentPhase = phase;
}

auto_test_phase_t AutoTest_GetPhase(void)
{
    return currentPhase;
}

const char* AutoTest_GetPhaseToken(void)
{
    return phaseToken((auto_test_phase_t)currentPhase);
}

void AutoTest_ClearAutoMode(void)
{
    autoMode = false;
    currentPhase = AUTO_TEST_PHASE_NONE;
}
