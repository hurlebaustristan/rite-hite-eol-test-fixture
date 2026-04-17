#include <gui/containers/Buttons_LEDs_Screen.hpp>
#include <gui/common/FrontendApplication.hpp>
#include <touchgfx/Color.hpp>
#include <cstring>

extern "C" {
#include "button_led_test.h"
#include "eol_test_protocol.h"
}

#ifndef SIMULATOR
extern "C" {
#include "stm32u5xx_hal.h"
}
#endif

static const touchgfx::colortype COLOR_GREEN = touchgfx::Color::getColorFromRGB(0, 200, 0);
static const touchgfx::colortype COLOR_RED   = touchgfx::Color::getColorFromRGB(220, 0, 0);
static const touchgfx::colortype COLOR_YELLOW = touchgfx::Color::getColorFromRGB(255, 215, 0);
static const touchgfx::colortype COLOR_BLACK = touchgfx::Color::getColorFromRGB(0, 0, 0);
static const uint32_t BUTTON_LED_SUCCESS_DELAY_MS = 1000u;

static uint8_t progressForPassedCount(uint8_t passedCount)
{
    if (passedCount >= 6u)
    {
        return 100u;
    }

    return (uint8_t)(((uint16_t)passedCount * 100u) / 6u);
}

static const char* stepName(uint8_t step)
{
    switch (step)
    {
    case BUTTON_LED_TEST_STEP_BUTTONS_INIT:  return "Buttons Init";
    case BUTTON_LED_TEST_STEP_LED_BEGIN:     return "LED Begin";
    case BUTTON_LED_TEST_STEP_LED_ROTATE:    return "LED Rotate";
    case BUTTON_LED_TEST_STEP_USR0_PRESS:    return "USR_0 Press";
    case BUTTON_LED_TEST_STEP_USR0_RELEASE:  return "USR_0 Release";
    case BUTTON_LED_TEST_STEP_PNL0_PRESS:    return "PNL_0 Press";
    case BUTTON_LED_TEST_STEP_PNL0_RELEASE:  return "PNL_0 Release";
    case BUTTON_LED_TEST_STEP_PNL1_PRESS:    return "PNL_1 Press";
    case BUTTON_LED_TEST_STEP_PNL1_RELEASE:  return "PNL_1 Release";
    case BUTTON_LED_TEST_STEP_PNL2_PRESS:    return "PNL_2 Press";
    case BUTTON_LED_TEST_STEP_PNL2_RELEASE:  return "PNL_2 Release";
    case BUTTON_LED_TEST_STEP_PNL3_PRESS:    return "PNL_3 Press";
    case BUTTON_LED_TEST_STEP_PNL3_RELEASE:  return "PNL_3 Release";
    case BUTTON_LED_TEST_STEP_LED_STOP:      return "LED Stop";
    case BUTTON_LED_TEST_STEP_VISUAL_CHECK:  return "LED Visual Check";
    default:                                 return "Buttons / LEDs";
    }
}

Buttons_LEDs_Screen::Buttons_LEDs_Screen()
    : passedMask(0u),
      passedCount(0u),
      visualReady(false),
      operatorDecisionLocked(true),
      successTransitionPending(false),
      successTimestamp(0u),
      progressNoOpCallback(this, &Buttons_LEDs_Screen::handleProgressNoOp)
{
}

void Buttons_LEDs_Screen::initialize()
{
    Buttons_LEDs_ScreenBase::initialize();
    boxprogressbar_Buttons_LEDs.setValueUpdatedAction(progressNoOpCallback);
}

void Buttons_LEDs_Screen::prepareForActivation()
{
    passedMask = 0u;
    passedCount = 0u;
    visualReady = false;
    operatorDecisionLocked = true;
    successTransitionPending = false;
    successTimestamp = 0u;

    Buttons_Header.setColor(COLOR_BLACK);
    Buttons_Header.invalidate();
    USR_0_Button.setColor(COLOR_BLACK); USR_0_Button.invalidate();
    PNL_0_Button.setColor(COLOR_BLACK); PNL_0_Button.invalidate();
    PNL_1_Button.setColor(COLOR_BLACK); PNL_1_Button.invalidate();
    PNL_2_Button.setColor(COLOR_BLACK); PNL_2_Button.invalidate();
    PNL_3_Button.setColor(COLOR_BLACK); PNL_3_Button.invalidate();

    boxprogressbar_Buttons_LEDs.setValue(0);
    boxprogressbar_Buttons_LEDs.invalidate();

    animatedImage1.stopAnimation();
    animatedImage1.invalidate();

    setOperatorButtonsEnabled(false);
}

void Buttons_LEDs_Screen::tickHandler()
{
#ifndef SIMULATOR
    if (successTransitionPending &&
        ((HAL_GetTick() - successTimestamp) >= BUTTON_LED_SUCCESS_DELAY_MS))
    {
        successTransitionPending = false;
        application().gotoTest_Complete_ScreenScreenNoTransition();
    }
#endif
}

void Buttons_LEDs_Screen::setGoodButtonAction(touchgfx::GenericCallback<const touchgfx::AbstractButton&>& callback)
{
    button_LEDs_GOOD.setAction(callback);
}

void Buttons_LEDs_Screen::setBadButtonAction(touchgfx::GenericCallback<const touchgfx::AbstractButton&>& callback)
{
    button_LEDs_BAD.setAction(callback);
}

void Buttons_LEDs_Screen::setOperatorButtonsEnabled(bool enabled)
{
    const uint8_t alpha = enabled ? 255u : 96u;

    button_LEDs_GOOD.setTouchable(enabled);
    button_LEDs_GOOD.setAlpha(alpha);
    button_LEDs_GOOD.invalidate();

    button_LEDs_BAD.setTouchable(enabled);
    button_LEDs_BAD.setAlpha(alpha);
    button_LEDs_BAD.invalidate();
}

void Buttons_LEDs_Screen::onButtonItemReady(uint8_t item)
{
    touchgfx::TextArea* label = 0;

    switch (item)
    {
    case BUTTON_LED_TEST_ITEM_USR0: label = &USR_0_Button; break;
    case BUTTON_LED_TEST_ITEM_PNL0: label = &PNL_0_Button; break;
    case BUTTON_LED_TEST_ITEM_PNL1: label = &PNL_1_Button; break;
    case BUTTON_LED_TEST_ITEM_PNL2: label = &PNL_2_Button; break;
    case BUTTON_LED_TEST_ITEM_PNL3: label = &PNL_3_Button; break;
    default:                       label = 0;            break;
    }

    if (label == 0)
    {
        return;
    }

    label->setColor(COLOR_YELLOW);
    label->invalidate();
}

void Buttons_LEDs_Screen::onButtonItemPass(uint8_t item)
{
    touchgfx::TextArea* label = 0;
    const uint8_t bitMask = (uint8_t)(1u << item);

    switch (item)
    {
    case BUTTON_LED_TEST_ITEM_USR0: label = &USR_0_Button; break;
    case BUTTON_LED_TEST_ITEM_PNL0: label = &PNL_0_Button; break;
    case BUTTON_LED_TEST_ITEM_PNL1: label = &PNL_1_Button; break;
    case BUTTON_LED_TEST_ITEM_PNL2: label = &PNL_2_Button; break;
    case BUTTON_LED_TEST_ITEM_PNL3: label = &PNL_3_Button; break;
    default:                       label = 0;            break;
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
        if (passedCount < 5u)
        {
            passedCount++;
        }
    }

    if (passedMask == 0x1Fu)
    {
        Buttons_Header.setColor(COLOR_GREEN);
        Buttons_Header.invalidate();
    }

    boxprogressbar_Buttons_LEDs.setValue(progressForPassedCount(passedCount));
    boxprogressbar_Buttons_LEDs.invalidate();
}

void Buttons_LEDs_Screen::onVisualReady()
{
    visualReady = true;
    operatorDecisionLocked = false;
    animatedImage1.startAnimation(false, true, true);
    animatedImage1.invalidate();
    setOperatorButtonsEnabled(true);
}

void Buttons_LEDs_Screen::onButtonLedTestFail(uint8_t step, uint8_t failKind,
                                              const char* expectedToken,
                                              const char* actualText)
{
#ifndef SIMULATOR
    FaultInfo& fi = FrontendApplication::faultInfo;
    touchgfx::TextArea* label = 0;

    switch (step)
    {
    case BUTTON_LED_TEST_STEP_USR0_PRESS:
    case BUTTON_LED_TEST_STEP_USR0_RELEASE:
        label = &USR_0_Button;
        break;
    case BUTTON_LED_TEST_STEP_PNL0_PRESS:
    case BUTTON_LED_TEST_STEP_PNL0_RELEASE:
        label = &PNL_0_Button;
        break;
    case BUTTON_LED_TEST_STEP_PNL1_PRESS:
    case BUTTON_LED_TEST_STEP_PNL1_RELEASE:
        label = &PNL_1_Button;
        break;
    case BUTTON_LED_TEST_STEP_PNL2_PRESS:
    case BUTTON_LED_TEST_STEP_PNL2_RELEASE:
        label = &PNL_2_Button;
        break;
    case BUTTON_LED_TEST_STEP_PNL3_PRESS:
    case BUTTON_LED_TEST_STEP_PNL3_RELEASE:
        label = &PNL_3_Button;
        break;
    default:
        label = 0;
        break;
    }

    operatorDecisionLocked = true;
    setOperatorButtonsEnabled(false);

    if (label != 0)
    {
        label->setColor(COLOR_RED);
        label->invalidate();
    }
    else
    {
        Buttons_Header.setColor(COLOR_RED);
        Buttons_Header.invalidate();
    }

    memset(&fi, 0, sizeof(fi));

    if (failKind == (uint8_t)BUTTON_LED_TEST_FAIL_OPERATOR_REJECTED)
    {
        fi.errorCode = (uint16_t)EOL_ERR_BUTTON_LED_OPERATOR_REJECTED;
    }
    else if (failKind == (uint8_t)BUTTON_LED_TEST_FAIL_TIMEOUT)
    {
        fi.errorCode = (uint16_t)EOL_ERR_BUTTON_LED_TIMEOUT;
    }
    else if (failKind == (uint8_t)BUTTON_LED_TEST_FAIL_STATE_MISMATCH)
    {
        fi.errorCode = (uint16_t)EOL_ERR_BUTTON_LED_STATE_MISMATCH;
    }
    else if (failKind == (uint8_t)BUTTON_LED_TEST_FAIL_INVALID_REPLY)
    {
        fi.errorCode = (uint16_t)EOL_ERR_BUTTON_LED_INVALID_REPLY;
    }
    else
    {
        fi.errorCode = (uint16_t)EOL_ERR_BUTTON_LED_NO_RESPONSE;
    }

    strncpy(fi.testName, stepName(step), sizeof(fi.testName) - 1u);
    fi.testName[sizeof(fi.testName) - 1u] = '\0';

    if (expectedToken != 0)
    {
        strncpy(fi.expected, expectedToken, sizeof(fi.expected) - 1u);
        fi.expected[sizeof(fi.expected) - 1u] = '\0';
    }

    if (actualText != 0)
    {
        strncpy(fi.actual, actualText, sizeof(fi.actual) - 1u);
        fi.actual[sizeof(fi.actual) - 1u] = '\0';
    }

    application().gotoFault_ScreenScreenNoTransition();
#else
    (void)step;
    (void)failKind;
    (void)expectedToken;
    (void)actualText;
#endif
}

void Buttons_LEDs_Screen::onButtonLedTestComplete()
{
#ifndef SIMULATOR
    visualReady = true;
    operatorDecisionLocked = true;
    setOperatorButtonsEnabled(false);

    passedMask = 0x1Fu;
    passedCount = 6u;

    Buttons_Header.setColor(COLOR_GREEN);
    Buttons_Header.invalidate();
    USR_0_Button.setColor(COLOR_GREEN); USR_0_Button.invalidate();
    PNL_0_Button.setColor(COLOR_GREEN); PNL_0_Button.invalidate();
    PNL_1_Button.setColor(COLOR_GREEN); PNL_1_Button.invalidate();
    PNL_2_Button.setColor(COLOR_GREEN); PNL_2_Button.invalidate();
    PNL_3_Button.setColor(COLOR_GREEN); PNL_3_Button.invalidate();

    boxprogressbar_Buttons_LEDs.setValue(100);
    boxprogressbar_Buttons_LEDs.invalidate();

    successTransitionPending = true;
    successTimestamp = HAL_GetTick();
#endif
}

void Buttons_LEDs_Screen::handleProgressNoOp(const touchgfx::AbstractProgressIndicator& src)
{
    (void)src;
}
