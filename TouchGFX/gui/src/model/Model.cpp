#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>

#ifndef SIMULATOR
extern "C" {
#include "gpio_config.h"
#include "auto_test.h"
#include "comm_test.h"
#include "di_test.h"
#include "ai_test.h"
#include "output_test.h"
#include "button_led_test.h"
#include "export_uart.h"
#include "stm32u5xx_hal.h"
}

extern "C" void triggerAutoTestScreenTransition(void);
#endif

static const uint32_t BUTTON_LED_SCREEN_ENTRY_DELAY_MS = 500u;

Model::Model() : modelListener(0),
                 commTestRunning(false),
                 diTestRunning(false),
                 aiTestRunning(false),
                 outputTestRunning(false),
                 buttonLedTestRunning(false),
                 buttonLedTestPendingStart(false),
                 buttonLedTestStartTimestamp(0u)
{

}

void Model::tick()
{
#ifndef SIMULATOR
    ExportUart_Tick();

    if (AutoTest_ConsumeStartRequest())
    {
        triggerAutoTestScreenTransition();
    }

    /* Read all 6 digital inputs and notify the active presenter */
    uint8_t inputState = GPIO_Config_ReadAllInputs();
    if (modelListener)
    {
        modelListener->notifyDigitalInputState(inputState);
    }

    /* Drive the communication-test state machine when active */
    if (commTestRunning)
    {
        CommTest_Tick();

        if (CommTest_HasProgressEvent())
        {
            eol_progress_event_t evt = CommTest_GetProgressEvent();
            if (modelListener)
            {
                modelListener->notifyCommTestProgress(
                    (uint8_t)CommTest_CurrentStage(),
                    (uint8_t)evt);
            }
        }

        if (CommTest_HasResult())
        {
            eol_test_result_t res = CommTest_GetResult();
            if (modelListener)
            {
                modelListener->notifyCommTestResult(
                    (uint8_t)res.stage,
                    (uint16_t)res.errorCode);
            }

            /* Stop ticking the engine once it's done */
            if (CommTest_CurrentStage() == EOL_STAGE_DONE)
            {
                commTestRunning = false;
            }
        }
    }
#endif

#ifndef SIMULATOR
    if (diTestRunning)
    {
        DITest_Tick();

        if (DITest_HasProgressEvent())
        {
            di_test_progress_t prog = DITest_GetProgressEvent();
            if (modelListener)
            {
                modelListener->notifyDigitalInputTestPass(prog.channelPassed);
            }
        }

        if (DITest_HasResult())
        {
            di_test_result_t res = DITest_GetResult();

            if (modelListener)
            {
                if (res.passed)
                {
                    modelListener->notifyDigitalInputsComplete();
                }
                else
                {
                    modelListener->notifyDigitalInputTestFail(
                        res.failedChannel,
                        (uint8_t)res.failureKind);
                }
            }

            diTestRunning = false;
        }
    }
#endif

#ifndef SIMULATOR
    if (aiTestRunning)
    {
        AITest_Tick();

        if (AITest_HasProgressEvent())
        {
            ai_test_progress_t prog = AITest_GetProgressEvent();
            if (modelListener)
            {
                modelListener->notifyAnalogInputTestPass(prog.channelPassed);
            }
        }

        if (AITest_HasResult())
        {
            ai_test_result_t res = AITest_GetResult();

            if (modelListener)
            {
                if (res.passed)
                {
                    modelListener->notifyAnalogInputsComplete();
                }
                else
                {
                    modelListener->notifyAnalogInputTestFail(
                        res.failedChannel,
                        (uint8_t)res.failedLevel,
                        (uint8_t)res.failureKind,
                        res.measuredEngineering,
                        res.expectedEngineering,
                        res.toleranceEngineering,
                        (uint8_t)res.kind);
                }
            }

            aiTestRunning = false;
        }
    }
#endif

#ifndef SIMULATOR
    if (outputTestRunning)
    {
        OutputTest_Tick();

        if (OutputTest_HasProgressEvent())
        {
            output_test_progress_t prog = OutputTest_GetProgressEvent();
            if (AutoTest_IsAutoMode() &&
                (prog.itemPassed == OUTPUT_TEST_ITEM_K1_NO ||
                 prog.itemPassed == OUTPUT_TEST_ITEM_K1_NC ||
                 prog.itemPassed == OUTPUT_TEST_ITEM_K2_NO ||
                 prog.itemPassed == OUTPUT_TEST_ITEM_K2_NC))
            {
                AutoTest_SetPhase(AUTO_TEST_PHASE_RELAY_OUTPUTS);
            }
            if (modelListener)
            {
                modelListener->notifyOutputTestPass((uint8_t)prog.itemPassed);
            }
        }

        if (OutputTest_HasResult())
        {
            output_test_result_t res = OutputTest_GetResult();

            if (modelListener)
            {
                if (res.passed)
                {
                    modelListener->notifyOutputsComplete();
                }
                else
                {
                    modelListener->notifyOutputTestFail(
                        (uint8_t)res.failedPhase,
                        (uint8_t)res.failureKind,
                        res.expectedState,
                        res.actualState);
                }
            }

            outputTestRunning = false;
        }
    }
#endif

#ifndef SIMULATOR
    if (buttonLedTestPendingStart && !buttonLedTestRunning)
    {
        if ((HAL_GetTick() - buttonLedTestStartTimestamp) >= BUTTON_LED_SCREEN_ENTRY_DELAY_MS)
        {
            ButtonLedTest_Start();
            if (AutoTest_IsAutoMode())
            {
                AutoTest_SetPhase(AUTO_TEST_PHASE_BUTTONS);
            }
            buttonLedTestPendingStart = false;
            buttonLedTestRunning = true;
        }
    }
#endif

#ifndef SIMULATOR
    if (buttonLedTestRunning)
    {
        ButtonLedTest_Tick();

        if (ButtonLedTest_HasProgressEvent())
        {
            button_led_test_progress_t prog = ButtonLedTest_GetProgressEvent();
            if (modelListener)
            {
                if (prog.kind == BUTTON_LED_PROGRESS_VISUAL_READY)
                {
                    if (AutoTest_IsAutoMode())
                    {
                        AutoTest_SetPhase(AUTO_TEST_PHASE_LEDS);
                    }
                    modelListener->notifyButtonLedVisualReady();
                }
                else if (prog.kind == BUTTON_LED_PROGRESS_READY)
                {
                    modelListener->notifyButtonLedTestReady((uint8_t)prog.item);
                }
                else
                {
                    modelListener->notifyButtonLedTestPass((uint8_t)prog.item);
                }
            }
        }

        if (ButtonLedTest_HasResult())
        {
            button_led_test_result_t res = ButtonLedTest_GetResult();

            if (modelListener)
            {
                if (res.passed)
                {
                    modelListener->notifyButtonLedTestComplete();
                }
                else
                {
                    modelListener->notifyButtonLedTestFail(
                        (uint8_t)res.failedStep,
                        (uint8_t)res.failureKind,
                        res.expectedToken,
                        res.actualText);
                }
            }

            buttonLedTestRunning = false;
        }
    }
#endif
}

void Model::startCommTest()
{
#ifndef SIMULATOR
    if (AutoTest_IsAutoMode())
    {
        AutoTest_SetPhase(AUTO_TEST_PHASE_COMMS);
    }
    CommTest_Start();
    commTestRunning = true;
#endif
}

void Model::startDigitalInputsTest()
{
#ifndef SIMULATOR
    if (AutoTest_IsAutoMode())
    {
        AutoTest_SetPhase(AUTO_TEST_PHASE_DIGITAL_INPUTS);
    }
    DITest_Start();
    diTestRunning = true;
    aiTestRunning = false;
#endif
}

void Model::startAnalogInputsTest()
{
#ifndef SIMULATOR
    if (AutoTest_IsAutoMode())
    {
        AutoTest_SetPhase(AUTO_TEST_PHASE_ANALOG_INPUTS);
    }
    AITest_Start();
    aiTestRunning = true;
#endif
}

void Model::startOutputsTest()
{
#ifndef SIMULATOR
    if (AutoTest_IsAutoMode())
    {
        AutoTest_SetPhase(AUTO_TEST_PHASE_DIGITAL_OUTPUTS);
    }
    OutputTest_Start();
    outputTestRunning = true;
#endif
}

void Model::startButtonLedTest()
{
#ifndef SIMULATOR
    if (AutoTest_IsAutoMode())
    {
        AutoTest_SetPhase(AUTO_TEST_PHASE_BUTTONS);
    }
    buttonLedTestRunning = false;
    buttonLedTestPendingStart = true;
    buttonLedTestStartTimestamp = HAL_GetTick();
#endif
}

void Model::submitButtonLedVisualResult(bool good)
{
#ifndef SIMULATOR
    ButtonLedTest_SubmitVisualResult(good);
#else
    (void)good;
#endif
}
