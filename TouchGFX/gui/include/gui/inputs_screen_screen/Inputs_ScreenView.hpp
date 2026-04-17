#ifndef INPUTS_SCREENVIEW_HPP
#define INPUTS_SCREENVIEW_HPP

#include <gui_generated/inputs_screen_screen/Inputs_ScreenViewBase.hpp>
#include <gui/inputs_screen_screen/Inputs_ScreenPresenter.hpp>

class Inputs_ScreenView : public Inputs_ScreenViewBase
{
public:
    Inputs_ScreenView();
    virtual ~Inputs_ScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    void onDigitalInputPass(uint8_t channel);
    void onDigitalInputFail(uint8_t channel, uint8_t failKind);
    void onDigitalInputsComplete();
    void onAnalogInputPass(uint8_t channel);
    void onAnalogInputFail(uint8_t channel, uint8_t level, uint8_t failKind,
                           float actual, float expected, float tol, uint8_t kind);
    void onAnalogInputsComplete();
protected:
};

#endif // INPUTS_SCREENVIEW_HPP
