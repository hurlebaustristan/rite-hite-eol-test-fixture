#include <gui/containers/Outputs_Screen.hpp>
#include <gui/common/FrontendApplication.hpp>
#include <touchgfx/Color.hpp>
#include <cstdio>
#include <cstring>

#ifndef SIMULATOR
extern "C" {
#include "eol_test_protocol.h"
}
#endif

enum
{
    OUTPUT_ITEM_DO1 = 0,
    OUTPUT_ITEM_DO2,
    OUTPUT_ITEM_K1_NO,
    OUTPUT_ITEM_K1_NC,
    OUTPUT_ITEM_K2_NO,
    OUTPUT_ITEM_K2_NC
};

enum
{
    OUTPUT_PHASE_NONE = 0,
    OUTPUT_PHASE_K1_ON,
    OUTPUT_PHASE_K1_OFF,
    OUTPUT_PHASE_K2_ON,
    OUTPUT_PHASE_K2_OFF
};

enum
{
    OUTPUT_FAIL_NONE = 0,
    OUTPUT_FAIL_NO_RESPONSE,
    OUTPUT_FAIL_INVALID_REPLY,
    OUTPUT_FAIL_STATE_MISMATCH
};

static const touchgfx::colortype COLOR_GREEN = touchgfx::Color::getColorFromRGB(0, 200, 0);
static const touchgfx::colortype COLOR_RED   = touchgfx::Color::getColorFromRGB(220, 0, 0);
static const touchgfx::colortype COLOR_BLACK = touchgfx::Color::getColorFromRGB(0, 0, 0);

static uint8_t progressForPassedCount(uint8_t passedCount)
{
    if (passedCount >= 6u)
    {
        return 100u;
    }

    return (uint8_t)(((uint16_t)passedCount * 100u + 3u) / 6u);
}

static const char* phaseName(uint8_t phase)
{
    switch (phase)
    {
    case OUTPUT_PHASE_K1_ON:  return "Relay K1 ON";
    case OUTPUT_PHASE_K1_OFF: return "Relay K1 OFF";
    case OUTPUT_PHASE_K2_ON:  return "Relay K2 ON";
    case OUTPUT_PHASE_K2_OFF: return "Relay K2 OFF";
    default:                  return "Relay Output";
    }
}

static const char* ackTokenForPhase(uint8_t phase)
{
    switch (phase)
    {
    case OUTPUT_PHASE_K1_ON:  return "OK_K1_ON";
    case OUTPUT_PHASE_K1_OFF: return "OK_K1_OFF";
    case OUTPUT_PHASE_K2_ON:  return "OK_K2_ON";
    case OUTPUT_PHASE_K2_OFF: return "OK_K2_OFF";
    default:                  return "OK";
    }
}

Outputs_Screen::Outputs_Screen()
    : passedMask(0u),
      passedCount(0u)
{
}

void Outputs_Screen::initialize()
{
    Outputs_ScreenBase::initialize();
}

void Outputs_Screen::prepareForActivation()
{
    passedMask = 0u;
    passedCount = 0u;

    Digital_Outputs.setColor(COLOR_BLACK);
    Digital_Outputs.invalidate();
    Relay_Outputs.setColor(COLOR_BLACK);
    Relay_Outputs.invalidate();

    Digital_Output_1.setColor(COLOR_BLACK); Digital_Output_1.invalidate();
    Digital_Output_2.setColor(COLOR_BLACK); Digital_Output_2.invalidate();
    K1_NO.setColor(COLOR_BLACK);            K1_NO.invalidate();
    K1_NC.setColor(COLOR_BLACK);            K1_NC.invalidate();
    K2_NO.setColor(COLOR_BLACK);            K2_NO.invalidate();
    K2_NC.setColor(COLOR_BLACK);            K2_NC.invalidate();

    boxprogressbar_Outputs.setValue(0);
    boxprogressbar_Outputs.invalidate();
}

void Outputs_Screen::onOutputItemPass(uint8_t item)
{
    touchgfx::TextArea* label = 0;
    const uint8_t bitMask = (uint8_t)(1u << item);

    switch (item)
    {
    case OUTPUT_ITEM_DO1:   label = &Digital_Output_1; break;
    case OUTPUT_ITEM_DO2:   label = &Digital_Output_2; break;
    case OUTPUT_ITEM_K1_NO: label = &K1_NO;            break;
    case OUTPUT_ITEM_K1_NC: label = &K1_NC;            break;
    case OUTPUT_ITEM_K2_NO: label = &K2_NO;            break;
    case OUTPUT_ITEM_K2_NC: label = &K2_NC;            break;
    default:                label = 0;                 break;
    }

    if (label == 0)
    {
        return;
    }

    label->setColor(COLOR_GREEN);
    label->invalidate();

    if ((passedMask & bitMask) == 0u)
    {
        passedMask |= bitMask;
        if (passedCount < 6u)
        {
            passedCount++;
        }
    }

    if ((passedMask & 0x03u) == 0x03u)
    {
        Digital_Outputs.setColor(COLOR_GREEN);
        Digital_Outputs.invalidate();
    }

    if ((passedMask & 0x3Cu) == 0x3Cu)
    {
        Relay_Outputs.setColor(COLOR_GREEN);
        Relay_Outputs.invalidate();
    }

    boxprogressbar_Outputs.setValue(progressForPassedCount(passedCount));
    boxprogressbar_Outputs.invalidate();
}

void Outputs_Screen::onOutputTestFail(uint8_t phase, uint8_t failKind,
                                      const char* expectedState, const char* actualState)
{
#ifndef SIMULATOR
    FaultInfo& fi = FrontendApplication::faultInfo;
    touchgfx::TextArea* label = 0;

    switch (phase)
    {
    case OUTPUT_PHASE_K1_ON:  label = &K1_NO; break;
    case OUTPUT_PHASE_K1_OFF: label = &K1_NC; break;
    case OUTPUT_PHASE_K2_ON:  label = &K2_NO; break;
    case OUTPUT_PHASE_K2_OFF: label = &K2_NC; break;
    default:                  label = 0;      break;
    }

    if (label != 0)
    {
        label->setColor(COLOR_RED);
        label->invalidate();
    }

    memset(&fi, 0, sizeof(fi));
    strncpy(fi.testName, phaseName(phase), sizeof(fi.testName) - 1u);
    fi.testName[sizeof(fi.testName) - 1u] = '\0';

    if (failKind == (uint8_t)OUTPUT_FAIL_NO_RESPONSE)
    {
        fi.errorCode = (uint16_t)EOL_ERR_OUTPUT_NO_RESPONSE;
        strncpy(fi.expected, ackTokenForPhase(phase), sizeof(fi.expected) - 1u);
        strncpy(fi.actual, "No response", sizeof(fi.actual) - 1u);
    }
    else if (failKind == (uint8_t)OUTPUT_FAIL_INVALID_REPLY)
    {
        fi.errorCode = (uint16_t)EOL_ERR_OUTPUT_INVALID_REPLY;
        strncpy(fi.expected, ackTokenForPhase(phase), sizeof(fi.expected) - 1u);
        strncpy(fi.actual, "Invalid reply", sizeof(fi.actual) - 1u);
    }
    else
    {
        fi.errorCode = (uint16_t)EOL_ERR_OUTPUT_STATE_MISMATCH;
        strncpy(fi.expected, expectedState != 0 ? expectedState : "Expected state",
                sizeof(fi.expected) - 1u);
        strncpy(fi.actual, actualState != 0 ? actualState : "Measured state",
                sizeof(fi.actual) - 1u);
    }

    fi.expected[sizeof(fi.expected) - 1u] = '\0';
    fi.actual[sizeof(fi.actual) - 1u] = '\0';

    application().gotoFault_ScreenScreenNoTransition();
#else
    (void)phase;
    (void)failKind;
    (void)expectedState;
    (void)actualState;
#endif
}

void Outputs_Screen::onOutputsComplete()
{
    passedMask = 0x3Fu;
    passedCount = 6u;

    Digital_Output_1.setColor(COLOR_GREEN); Digital_Output_1.invalidate();
    Digital_Output_2.setColor(COLOR_GREEN); Digital_Output_2.invalidate();
    K1_NO.setColor(COLOR_GREEN);            K1_NO.invalidate();
    K1_NC.setColor(COLOR_GREEN);            K1_NC.invalidate();
    K2_NO.setColor(COLOR_GREEN);            K2_NO.invalidate();
    K2_NC.setColor(COLOR_GREEN);            K2_NC.invalidate();

    Digital_Outputs.setColor(COLOR_GREEN);
    Digital_Outputs.invalidate();
    Relay_Outputs.setColor(COLOR_GREEN);
    Relay_Outputs.invalidate();

    boxprogressbar_Outputs.setValue(100);
    boxprogressbar_Outputs.invalidate();

    application().gotoButtons_LEDs_ScreenScreenNoTransition();
}
