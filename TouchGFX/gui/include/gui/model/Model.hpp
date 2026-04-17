#ifndef MODEL_HPP
#define MODEL_HPP

#include <cstdint>

class ModelListener;

class Model
{
public:
    Model();

    void bind(ModelListener* listener)
    {
        modelListener = listener;
    }

    void tick();

    /**
      * @brief  Begin the communication test sequence (PING/PONG + future stages).
      *         Called by the Comm_Reset_ScreenPresenter when the screen activates.
      */
    void startCommTest();

    /**
      * @brief  Begin the Digital Inputs 1..12 automated test on Inputs_Screen.
      */
    void startDigitalInputsTest();

    /**
      * @brief  Begin the Analog Inputs 1..4 automated test on Inputs_Screen.
      */
    void startAnalogInputsTest();

    /**
      * @brief  Begin the Outputs automated test on Outputs_Screen.
      */
    void startOutputsTest();

    /**
      * @brief  Begin the Buttons / LEDs automated test on Buttons_LEDs_Screen.
      */
    void startButtonLedTest();

    /**
      * @brief  Submit the operator LED visual decision after all button presses pass.
      */
    void submitButtonLedVisualResult(bool good);

protected:
    ModelListener* modelListener;
    bool commTestRunning;
    bool diTestRunning;
    bool aiTestRunning;
    bool outputTestRunning;
    bool buttonLedTestRunning;
    bool buttonLedTestPendingStart;
    uint32_t buttonLedTestStartTimestamp;
};

#endif // MODEL_HPP
