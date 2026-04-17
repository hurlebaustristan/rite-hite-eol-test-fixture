#include <gui/containers/Inputs_Screen.hpp>
#include <gui/common/FrontendApplication.hpp>
#include <touchgfx/Color.hpp>
#include <cstdio>
#include <cstring>

#ifndef SIMULATOR
extern "C" {
#include "eol_test_protocol.h"
#include "di_test.h"
#include "ai_test.h"
#include "eol_format.h"
}
#endif

static const touchgfx::colortype COLOR_GREEN = touchgfx::Color::getColorFromRGB(0, 200, 0);
static const touchgfx::colortype COLOR_RED   = touchgfx::Color::getColorFromRGB(220, 0, 0);
static const touchgfx::colortype COLOR_BLACK = touchgfx::Color::getColorFromRGB(0, 0, 0);

static uint8_t progressForDigitalPassedCount(uint8_t passedCount)
{
    if (passedCount >= 12u)
    {
        return 75u;
    }

    /* Round((n * 75) / 12) using integer math. */
    return (uint8_t)(((uint16_t)passedCount * 75u + 6u) / 12u);
}

static uint8_t progressForAnalogPassedCount(uint8_t passedCount)
{
    if (passedCount >= 4u)
    {
        return 100u;
    }

    /* 75 + round((n * 25) / 4) */
    return (uint8_t)(75u + (((uint16_t)passedCount * 25u + 2u) / 4u));
}

#ifndef SIMULATOR
static const char* levelNameShort(uint8_t level)
{
    switch ((ai_test_level_t)level)
    {
    case AI_TEST_LEVEL_LOW:  return "Low";
    case AI_TEST_LEVEL_MID:  return "Mid";
    case AI_TEST_LEVEL_HIGH: return "High";
    default:                 return "Level";
    }
}
#endif

Inputs_Screen::Inputs_Screen()
    : digitalPassedMask(0u),
      digitalPassedCount(0u),
      analogPassedMask(0u),
      analogPassedCount(0u),
      digitalInputsComplete(false),
      analogInputsComplete(false)
{
}

void Inputs_Screen::initialize()
{
    Inputs_ScreenBase::initialize();
}

void Inputs_Screen::prepareForActivation()
{
    digitalPassedMask = 0u;
    digitalPassedCount = 0u;
    analogPassedMask = 0u;
    analogPassedCount = 0u;
    digitalInputsComplete = false;
    analogInputsComplete = false;

    Digital_Inputs_Header.setColor(COLOR_BLACK);
    Digital_Inputs_Header.invalidate();
    Analog_Inputs_Header.setColor(COLOR_BLACK);
    Analog_Inputs_Header.invalidate();

    Digital_Input_1.setColor(COLOR_BLACK);  Digital_Input_1.invalidate();
    Digital_Input_2.setColor(COLOR_BLACK);  Digital_Input_2.invalidate();
    Digital_Input_3.setColor(COLOR_BLACK);  Digital_Input_3.invalidate();
    Digital_Input_4.setColor(COLOR_BLACK);  Digital_Input_4.invalidate();
    Digital_Input_5.setColor(COLOR_BLACK);  Digital_Input_5.invalidate();
    Digital_Input_6.setColor(COLOR_BLACK);  Digital_Input_6.invalidate();
    Digital_Input_7.setColor(COLOR_BLACK);  Digital_Input_7.invalidate();
    Digital_Input_8.setColor(COLOR_BLACK);  Digital_Input_8.invalidate();
    Digital_Input_9.setColor(COLOR_BLACK);  Digital_Input_9.invalidate();
    Digital_Input_10.setColor(COLOR_BLACK); Digital_Input_10.invalidate();
    Digital_Input_11.setColor(COLOR_BLACK); Digital_Input_11.invalidate();
    Digital_Input_12.setColor(COLOR_BLACK); Digital_Input_12.invalidate();

    Analog_Input_1.setColor(COLOR_BLACK); Analog_Input_1.invalidate();
    Analog_Input_2.setColor(COLOR_BLACK); Analog_Input_2.invalidate();
    Analog_Input_3.setColor(COLOR_BLACK); Analog_Input_3.invalidate();
    Analog_Input_4.setColor(COLOR_BLACK); Analog_Input_4.invalidate();

    boxprogressbar_Inputs.setValue(0);
    boxprogressbar_Inputs.invalidate();
}

void Inputs_Screen::onDigitalInputPass(uint8_t channel)
{
    touchgfx::TextArea* label = 0;
    uint16_t bitMask;

    switch (channel)
    {
    case 1:  label = &Digital_Input_1; break;
    case 2:  label = &Digital_Input_2; break;
    case 3:  label = &Digital_Input_3; break;
    case 4:  label = &Digital_Input_4; break;
    case 5:  label = &Digital_Input_5; break;
    case 6:  label = &Digital_Input_6; break;
    case 7:  label = &Digital_Input_7; break;
    case 8:  label = &Digital_Input_8; break;
    case 9:  label = &Digital_Input_9; break;
    case 10: label = &Digital_Input_10; break;
    case 11: label = &Digital_Input_11; break;
    case 12: label = &Digital_Input_12; break;
    default: label = 0; break;
    }

    if (label == 0)
    {
        return;
    }

    label->setColor(COLOR_GREEN);
    label->invalidate();

    bitMask = (uint16_t)(1u << (channel - 1u));
    if ((digitalPassedMask & bitMask) == 0u)
    {
        digitalPassedMask |= bitMask;
        if (digitalPassedCount < 12u)
        {
            digitalPassedCount++;
        }
    }

    boxprogressbar_Inputs.setValue(progressForDigitalPassedCount(digitalPassedCount));
    boxprogressbar_Inputs.invalidate();
}

void Inputs_Screen::onDigitalInputFail(uint8_t channel, uint8_t failKind)
{
#ifndef SIMULATOR
    FaultInfo& fi = FrontendApplication::faultInfo;
    uint16_t errorCode = (uint16_t)EOL_ERR_DI_INVALID_REPLY;

    memset(&fi, 0, sizeof(fi));

    if (failKind == (uint8_t)DI_TEST_FAIL_NO_RESPONSE)
    {
        errorCode = (uint16_t)EOL_ERR_DI_NO_RESPONSE;
        strncpy(fi.actual, "No response", sizeof(fi.actual) - 1u);
    }
    else if (failKind == (uint8_t)DI_TEST_FAIL_FAIL_TOKEN)
    {
        errorCode = (uint16_t)EOL_ERR_DI_FAIL_TOKEN;
        strncpy(fi.actual, "Invalid transition", sizeof(fi.actual) - 1u);
    }
    else
    {
        errorCode = (uint16_t)EOL_ERR_DI_INVALID_REPLY;
        strncpy(fi.actual, "Invalid reply", sizeof(fi.actual) - 1u);
    }

    fi.errorCode = errorCode;
    (void)snprintf(fi.testName, sizeof(fi.testName), "Digital Input %u", (unsigned)channel);
    strncpy(fi.expected, "High then Low", sizeof(fi.expected) - 1u);

    application().gotoFault_ScreenScreenNoTransition();
#else
    (void)channel;
    (void)failKind;
#endif
}

void Inputs_Screen::onDigitalInputsComplete()
{
    digitalPassedMask = 0x0FFFu;
    digitalPassedCount = 12u;
    digitalInputsComplete = true;
    Digital_Inputs_Header.setColor(COLOR_GREEN);
    Digital_Inputs_Header.invalidate();
    boxprogressbar_Inputs.setValue(75);
    boxprogressbar_Inputs.invalidate();
}

void Inputs_Screen::onAnalogInputPass(uint8_t channel)
{
    touchgfx::TextArea* label = 0;
    const uint8_t bitMask = (uint8_t)(1u << (channel - 1u));

    switch (channel)
    {
    case 1:  label = &Analog_Input_1; break;
    case 2:  label = &Analog_Input_2; break;
    case 3:  label = &Analog_Input_3; break;
    case 4:  label = &Analog_Input_4; break;
    default: label = 0; break;
    }

    if (label == 0)
    {
        return;
    }

    label->setColor(COLOR_GREEN);
    label->invalidate();

    if ((analogPassedMask & bitMask) == 0u)
    {
        analogPassedMask |= bitMask;
        if (analogPassedCount < 4u)
        {
            analogPassedCount++;
        }
    }

    boxprogressbar_Inputs.setValue(progressForAnalogPassedCount(analogPassedCount));
    boxprogressbar_Inputs.invalidate();
}

void Inputs_Screen::onAnalogInputFail(uint8_t channel, uint8_t level, uint8_t failKind,
                                      float actual, float expected, float tol, uint8_t kind)
{
#ifndef SIMULATOR
    FaultInfo& fi = FrontendApplication::faultInfo;
    uint16_t errorCode = (uint16_t)EOL_ERR_AI_INVALID_REPLY;
    touchgfx::TextArea* label = 0;

    switch (channel)
    {
    case 1:  label = &Analog_Input_1; break;
    case 2:  label = &Analog_Input_2; break;
    case 3:  label = &Analog_Input_3; break;
    case 4:  label = &Analog_Input_4; break;
    default: label = 0; break;
    }

    if (label)
    {
        label->setColor(COLOR_RED);
        label->invalidate();
    }

    memset(&fi, 0, sizeof(fi));

    if (failKind == (uint8_t)AI_TEST_FAIL_NO_RESPONSE)
    {
        errorCode = (uint16_t)EOL_ERR_AI_NO_RESPONSE;
        strncpy(fi.expected, "Sample reply", sizeof(fi.expected) - 1u);
        strncpy(fi.actual, "No response", sizeof(fi.actual) - 1u);
    }
    else if (failKind == (uint8_t)AI_TEST_FAIL_INVALID_REPLY)
    {
        errorCode = (uint16_t)EOL_ERR_AI_INVALID_REPLY;
        (void)snprintf(fi.expected, sizeof(fi.expected), "OK_AI%u_%s:<V>",
                       (unsigned)channel, levelNameShort(level));
        strncpy(fi.actual, "Invalid reply", sizeof(fi.actual) - 1u);
    }
    else
    {
        const bool isCurrent = (kind == (uint8_t)AI_TEST_KIND_CURRENT);
        errorCode = (uint16_t)EOL_ERR_AI_OUT_OF_RANGE;
        (void)EOL_FormatAnalogExpected(isCurrent, expected, tol, fi.expected, sizeof(fi.expected));
        (void)EOL_FormatAnalogActual(isCurrent, actual, fi.actual, sizeof(fi.actual));
    }

    fi.errorCode = errorCode;
    (void)snprintf(fi.testName, sizeof(fi.testName), "Analog Input %u - %s",
                   (unsigned)channel, levelNameShort(level));

    application().gotoFault_ScreenScreenNoTransition();
#else
    (void)channel;
    (void)level;
    (void)failKind;
    (void)actual;
    (void)expected;
    (void)tol;
    (void)kind;
#endif
}

void Inputs_Screen::onAnalogInputsComplete()
{
    analogPassedMask = 0x0Fu;
    analogPassedCount = 4u;
    analogInputsComplete = true;
    Analog_Inputs_Header.setColor(COLOR_GREEN);
    Analog_Inputs_Header.invalidate();
    boxprogressbar_Inputs.setValue(100);
    boxprogressbar_Inputs.invalidate();

    if (digitalInputsComplete && analogInputsComplete)
    {
        application().gotoOutputs_ScreenScreenNoTransition();
    }
}
