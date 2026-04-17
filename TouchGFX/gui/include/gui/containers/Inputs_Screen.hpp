#ifndef INPUTS_SCREEN_HPP
#define INPUTS_SCREEN_HPP

#include <gui_generated/containers/Inputs_ScreenBase.hpp>

class Inputs_Screen : public Inputs_ScreenBase
{
public:
    Inputs_Screen();
    virtual ~Inputs_Screen() {}

    virtual void initialize();

    void prepareForActivation();
    void onDigitalInputPass(uint8_t channel);
    void onDigitalInputFail(uint8_t channel, uint8_t failKind);
    void onDigitalInputsComplete();
    void onAnalogInputPass(uint8_t channel);
    void onAnalogInputFail(uint8_t channel, uint8_t level, uint8_t failKind,
                           float actual, float expected, float tol, uint8_t kind);
    void onAnalogInputsComplete();

protected:
    uint16_t digitalPassedMask;
    uint8_t digitalPassedCount;
    uint8_t analogPassedMask;
    uint8_t analogPassedCount;
    bool digitalInputsComplete;
    bool analogInputsComplete;
};

#endif // INPUTS_SCREEN_HPP
