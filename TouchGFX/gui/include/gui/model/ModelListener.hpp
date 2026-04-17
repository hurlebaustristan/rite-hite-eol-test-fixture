#ifndef MODELLISTENER_HPP
#define MODELLISTENER_HPP

#include <gui/model/Model.hpp>
#include <cstdint>

class ModelListener
{
public:
    ModelListener() : model(0) {}
    
    virtual ~ModelListener() {}

    void bind(Model* m)
    {
        model = m;
    }

    /**
      * @brief  Called every tick with the packed digital-input state.
      *         Bit 0 = K1NO, 1 = K1NC, 2 = K2NO, 3 = K2NC, 4 = DO1, 5 = DO2.
      *         Override in the active Presenter to forward to the View.
      */
    virtual void notifyDigitalInputState(uint8_t /*state*/) {}

    /**
      * @brief  Called when a comm-test stage completes (pass or fail).
      * @param  stage      Which stage just finished.
      * @param  errorCode  EOL_ERR_NONE on success, else the specific fault.
      */
    virtual void notifyCommTestResult(uint8_t stage, uint16_t errorCode) {}

    /**
      * @brief  Called for non-terminal comm-test progress updates.
      * @param  stage          Current stage emitting the update.
      * @param  progressEvent  Stage-specific event identifier.
      */
    virtual void notifyCommTestProgress(uint8_t stage, uint8_t progressEvent) {}

    /**
      * @brief  Called when one Digital Input (1..12) fully passes (HIGH then LOW).
      */
    virtual void notifyDigitalInputTestPass(uint8_t /*channel*/) {}

    /**
      * @brief  Called when a Digital Input test fails.
      * @param  channel   1..12
      * @param  failKind  di_test_failure_kind_t value (cast to uint8_t)
      */
    virtual void notifyDigitalInputTestFail(uint8_t /*channel*/, uint8_t /*failKind*/) {}

    /**
      * @brief  Called when all 12 digital inputs pass.
      */
    virtual void notifyDigitalInputsComplete() {}

    /**
      * @brief  Called when one analog input channel (1..4) passes all 3 levels.
      */
    virtual void notifyAnalogInputTestPass(uint8_t /*channel*/) {}

    /**
      * @brief  Called when an analog input test point fails.
      * @param  channel    1..4
      * @param  level      ai_test_level_t cast to uint8_t
      * @param  failKind   ai_test_failure_kind_t cast to uint8_t
      * @param  actual     Measured engineering value (mA or V), if applicable
      * @param  expected   Expected engineering value (mA or V)
      * @param  tol        Tolerance in engineering units
      * @param  kind       ai_test_channel_kind_t cast to uint8_t
      */
    virtual void notifyAnalogInputTestFail(uint8_t /*channel*/, uint8_t /*level*/,
                                           uint8_t /*failKind*/, float /*actual*/,
                                           float /*expected*/, float /*tol*/,
                                           uint8_t /*kind*/) {}

    /**
      * @brief  Called when all 4 analog inputs pass.
      */
    virtual void notifyAnalogInputsComplete() {}

    /**
      * @brief  Called when one Outputs-screen item passes.
      * @param  item  output_test_item_t cast to uint8_t
      */
    virtual void notifyOutputTestPass(uint8_t /*item*/) {}

    /**
      * @brief  Called when the Outputs-screen automated test fails.
      * @param  phase          output_test_phase_t cast to uint8_t
      * @param  failKind       output_test_failure_kind_t cast to uint8_t
      * @param  expectedState  Logical expected contact summary for mismatch failures
      * @param  actualState    Logical measured contact summary for mismatch failures
      */
    virtual void notifyOutputTestFail(uint8_t /*phase*/, uint8_t /*failKind*/,
                                      const char* /*expectedState*/,
                                      const char* /*actualState*/) {}

    /**
      * @brief  Called when the Outputs-screen automated test fully passes.
      */
    virtual void notifyOutputsComplete() {}

    /**
      * @brief  Called when a Buttons/LEDs-screen item is active and ready for
      *         press detection. The fixture may pulse the actuator, and the
      *         operator can still press/release manually if needed.
      * @param  item  button_led_test_item_t cast to uint8_t
      */
    virtual void notifyButtonLedTestReady(uint8_t /*item*/) {}

    /**
      * @brief  Called when one Buttons/LEDs-screen item passes.
      * @param  item  button_led_test_item_t cast to uint8_t
      */
    virtual void notifyButtonLedTestPass(uint8_t /*item*/) {}

    /**
      * @brief  Called when the button sequence is complete and the operator
      *         GOOD/BAD buttons should be enabled.
      */
    virtual void notifyButtonLedVisualReady() {}

    /**
      * @brief  Called when the Buttons/LEDs screen stage fails.
      * @param  step           button_led_test_step_t cast to uint8_t
      * @param  failKind       button_led_test_failure_kind_t cast to uint8_t
      * @param  expectedToken  Expected acknowledgement or operator action
      * @param  actualText     Actual fault text
      */
    virtual void notifyButtonLedTestFail(uint8_t /*step*/, uint8_t /*failKind*/,
                                         const char* /*expectedToken*/,
                                         const char* /*actualText*/) {}

    /**
      * @brief  Called when the Buttons/LEDs screen stage fully passes.
      */
    virtual void notifyButtonLedTestComplete() {}

protected:
    Model* model;
};

#endif // MODELLISTENER_HPP
