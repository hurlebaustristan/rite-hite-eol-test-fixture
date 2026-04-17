#include <gui/containers/Comm_Reset_Screen.hpp>
#include <gui/common/FrontendApplication.hpp>
#include <touchgfx/Color.hpp>
#include <cstring>

#ifndef SIMULATOR
extern "C" {
#include "eol_test_protocol.h"
#include "stm32u5xx_hal.h"
}
#endif

/* Colour constants */
static const touchgfx::colortype COLOR_GREEN = touchgfx::Color::getColorFromRGB(0, 200, 0);
static const touchgfx::colortype COLOR_RED   = touchgfx::Color::getColorFromRGB(255, 0, 0);
static const touchgfx::colortype COLOR_BLACK = touchgfx::Color::getColorFromRGB(0, 0, 0);
static const uint32_t SUCCESS_NEXT_SCREEN_DELAY_MS = 1000U;

Comm_Reset_Screen::Comm_Reset_Screen()
    : successTimestamp(0),
      successTransitionPending(false),
      faultTimestamp(0),
      faultPending(false),
      pendingErrorCode(0),
      pendingStage(0)
{
}

void Comm_Reset_Screen::initialize()
{
    Comm_Reset_ScreenBase::initialize();
}

void Comm_Reset_Screen::prepareForActivation()
{
    successTransitionPending = false;
    successTimestamp = 0;
    faultPending = false;
    faultTimestamp = 0;
    pendingErrorCode = 0;
    pendingStage = 0;

    Communication.setColor(COLOR_BLACK);
    Communication.invalidate();
    Connected.setColor(COLOR_BLACK);
    Connected.invalidate();
    MCU_Reset.setColor(COLOR_BLACK);
    MCU_Reset.invalidate();
    Connected_1.setColor(COLOR_BLACK);
    Connected_1.invalidate();
    Reconnected.setColor(COLOR_BLACK);
    Reconnected.invalidate();
    Fault_Check.setColor(COLOR_BLACK);
    Fault_Check.invalidate();
    FC_Button_Pressed.setColor(COLOR_BLACK);
    FC_Button_Pressed.invalidate();
    FC_Reconnected.setColor(COLOR_BLACK);
    FC_Reconnected.invalidate();

    boxprogressbar_Comm_Reset_Buttons.setValue(0);
    boxprogressbar_Comm_Reset_Buttons.invalidate();
}

/* --------------------------------------------------------------------- */
/*  onCommTestResult — update text colours and trigger fault if needed    */
/* --------------------------------------------------------------------- */
void Comm_Reset_Screen::onCommTestResult(uint8_t stage, uint16_t errorCode)
{
#ifndef SIMULATOR
    if (stage == (uint8_t)EOL_STAGE_COMM)
    {
        if (errorCode == (uint16_t)EOL_ERR_NONE)
        {
            /* SUCCESS — turn "Communication" and "Connected" green */
            faultPending = false;
            pendingErrorCode = 0;
            pendingStage = 0;
            Communication.setColor(COLOR_GREEN);
            Communication.invalidate();
            Connected.setColor(COLOR_GREEN);
            Connected.invalidate();

            /* Advance the progress bar (33 % for this stage) */
            boxprogressbar_Comm_Reset_Buttons.setValue(33);
            boxprogressbar_Comm_Reset_Buttons.invalidate();
        }
        else
        {
            /* FAILURE — turn text red, start 3 s timer */
            Communication.setColor(COLOR_RED);
            Communication.invalidate();
            Connected.setColor(COLOR_RED);
            Connected.invalidate();

            faultPending    = true;
            pendingErrorCode = errorCode;
            pendingStage     = stage;
            faultTimestamp   = HAL_GetTick();
        }
    }
    else if (stage == (uint8_t)EOL_STAGE_MCU_RESET)
    {
        if (errorCode == (uint16_t)EOL_ERR_NONE)
        {
            faultPending = false;
            pendingErrorCode = 0;
            pendingStage = 0;

            MCU_Reset.setColor(COLOR_GREEN);
            MCU_Reset.invalidate();
            Connected_1.setColor(COLOR_GREEN);
            Connected_1.invalidate();
            Reconnected.setColor(COLOR_GREEN);
            Reconnected.invalidate();

            boxprogressbar_Comm_Reset_Buttons.setValue(66);
            boxprogressbar_Comm_Reset_Buttons.invalidate();
        }
        else
        {
            MCU_Reset.setColor(COLOR_RED);
            MCU_Reset.invalidate();

            if (errorCode == (uint16_t)EOL_ERR_MCU_RESET_PULSE_VERIFY_FAIL)
            {
                Connected_1.setColor(COLOR_RED);
                Connected_1.invalidate();
            }
            else
            {
                Reconnected.setColor(COLOR_RED);
                Reconnected.invalidate();
            }

            faultPending = true;
            pendingErrorCode = errorCode;
            pendingStage = stage;
            faultTimestamp = HAL_GetTick();
        }
    }
    else if (stage == (uint8_t)EOL_STAGE_FAULT_CHECK)
    {
        if (errorCode == (uint16_t)EOL_ERR_NONE)
        {
            faultPending = false;
            pendingErrorCode = 0;
            pendingStage = 0;
            successTransitionPending = true;
            successTimestamp = HAL_GetTick();

            Fault_Check.setColor(COLOR_GREEN);
            Fault_Check.invalidate();
            FC_Button_Pressed.setColor(COLOR_GREEN);
            FC_Button_Pressed.invalidate();
            FC_Reconnected.setColor(COLOR_GREEN);
            FC_Reconnected.invalidate();

            boxprogressbar_Comm_Reset_Buttons.setValue(100);
            boxprogressbar_Comm_Reset_Buttons.invalidate();
        }
        else
        {
            Fault_Check.setColor(COLOR_RED);
            Fault_Check.invalidate();

            if (errorCode == (uint16_t)EOL_ERR_FAULT_CHECK_PULSE_VERIFY_FAIL)
            {
                FC_Button_Pressed.setColor(COLOR_RED);
                FC_Button_Pressed.invalidate();
            }
            else
            {
                FC_Reconnected.setColor(COLOR_RED);
                FC_Reconnected.invalidate();
            }

            faultPending = true;
            pendingErrorCode = errorCode;
            pendingStage = stage;
            faultTimestamp = HAL_GetTick();
        }
    }
#else
    (void)stage;
    (void)errorCode;
#endif
}

void Comm_Reset_Screen::onCommTestProgress(uint8_t stage, uint8_t progressEvent)
{
#ifndef SIMULATOR
    if ((stage == (uint8_t)EOL_STAGE_MCU_RESET) &&
        (progressEvent == (uint8_t)EOL_PROGRESS_EVT_MCU_RESET_BUTTON_PULSE_DONE))
    {
        Connected_1.setColor(COLOR_GREEN);
        Connected_1.invalidate();
    }
    else if ((stage == (uint8_t)EOL_STAGE_FAULT_CHECK) &&
             (progressEvent == (uint8_t)EOL_PROGRESS_EVT_FAULT_CHECK_BUTTON_PULSE_DONE))
    {
        FC_Button_Pressed.setColor(COLOR_GREEN);
        FC_Button_Pressed.invalidate();
    }
#else
    (void)stage;
    (void)progressEvent;
#endif
}

/* --------------------------------------------------------------------- */
/*  tickHandler — manage 3 s red-text delay then goto Fault_Screen       */
/* --------------------------------------------------------------------- */
void Comm_Reset_Screen::tickHandler()
{
#ifndef SIMULATOR
    if (successTransitionPending)
    {
        if ((HAL_GetTick() - successTimestamp) >= SUCCESS_NEXT_SCREEN_DELAY_MS)
        {
            successTransitionPending = false;
            application().gotoInputs_ScreenScreenNoTransition();
            return;
        }
    }

    if (faultPending)
    {
        if ((HAL_GetTick() - faultTimestamp) >= EOL_FAULT_DISPLAY_MS)
        {
            faultPending = false;

            /* Populate the shared FaultInfo before navigating */
            FaultInfo& fi = FrontendApplication::faultInfo;
            fi.errorCode = pendingErrorCode;

            if (pendingErrorCode == (uint16_t)EOL_ERR_COMM_PING_NO_RESPONSE)
            {
                strncpy(fi.testName, "Communication", sizeof(fi.testName));
                strncpy(fi.expected, "PONG response",  sizeof(fi.expected));
                strncpy(fi.actual,   "No response",    sizeof(fi.actual));
            }
            else if (pendingErrorCode == (uint16_t)EOL_ERR_COMM_PING_BAD_REPLY)
            {
                strncpy(fi.testName, "Communication",  sizeof(fi.testName));
                strncpy(fi.expected, "PONG response",  sizeof(fi.expected));
                strncpy(fi.actual,   "Invalid reply",  sizeof(fi.actual));
            }
            else if (pendingErrorCode == (uint16_t)EOL_ERR_MCU_RESET_NO_RECONNECT)
            {
                strncpy(fi.testName, "MCU Reset",               sizeof(fi.testName));
                strncpy(fi.expected, "PONG response",           sizeof(fi.expected));
                strncpy(fi.actual,   "No response",             sizeof(fi.actual));
            }
            else if (pendingErrorCode == (uint16_t)EOL_ERR_MCU_RESET_PING_BAD_REPLY)
            {
                strncpy(fi.testName, "MCU Reset",               sizeof(fi.testName));
                strncpy(fi.expected, "PONG response",           sizeof(fi.expected));
                strncpy(fi.actual,   "Invalid reply",           sizeof(fi.actual));
            }
            else if (pendingErrorCode == (uint16_t)EOL_ERR_MCU_RESET_PULSE_VERIFY_FAIL)
            {
                strncpy(fi.testName, "MCU Reset",                  sizeof(fi.testName));
                strncpy(fi.expected, "PB11 pulse + PONG after reset", sizeof(fi.expected));
                strncpy(fi.actual,   "Pulse verify failed",        sizeof(fi.actual));
            }
            else if (pendingErrorCode == (uint16_t)EOL_ERR_FAULT_CHECK_NO_RECONNECT)
            {
                strncpy(fi.testName, "Fault Check",    sizeof(fi.testName));
                strncpy(fi.expected, "OK response",    sizeof(fi.expected));
                strncpy(fi.actual,   "No response",    sizeof(fi.actual));
            }
            else if (pendingErrorCode == (uint16_t)EOL_ERR_FAULT_CHECK_BAD_REPLY)
            {
                strncpy(fi.testName, "Fault Check",    sizeof(fi.testName));
                strncpy(fi.expected, "OK response",    sizeof(fi.expected));
                strncpy(fi.actual,   "Invalid reply",  sizeof(fi.actual));
            }
            else if (pendingErrorCode == (uint16_t)EOL_ERR_FAULT_CHECK_PULSE_VERIFY_FAIL)
            {
                strncpy(fi.testName, "Fault Check",              sizeof(fi.testName));
                strncpy(fi.expected, "PA8 pulse + OK response",  sizeof(fi.expected));
                strncpy(fi.actual,   "Pulse verify failed",      sizeof(fi.actual));
            }
            else
            {
                strncpy(fi.testName, "Unknown",        sizeof(fi.testName));
                strncpy(fi.expected, "N/A",            sizeof(fi.expected));
                strncpy(fi.actual,   "N/A",            sizeof(fi.actual));
            }

            application().gotoFault_ScreenScreenNoTransition();
        }
    }
#endif
}
