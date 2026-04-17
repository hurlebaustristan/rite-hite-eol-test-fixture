#include <gui/inputs_screen_screen/Inputs_ScreenView.hpp>

Inputs_ScreenView::Inputs_ScreenView()
{

}

void Inputs_ScreenView::setupScreen()
{
    Inputs_ScreenViewBase::setupScreen();
    inputs_Screen1.prepareForActivation();
}

void Inputs_ScreenView::tearDownScreen()
{
    Inputs_ScreenViewBase::tearDownScreen();
}

void Inputs_ScreenView::onDigitalInputPass(uint8_t channel)
{
    inputs_Screen1.onDigitalInputPass(channel);
}

void Inputs_ScreenView::onDigitalInputFail(uint8_t channel, uint8_t failKind)
{
    inputs_Screen1.onDigitalInputFail(channel, failKind);
}

void Inputs_ScreenView::onDigitalInputsComplete()
{
    inputs_Screen1.onDigitalInputsComplete();
}

void Inputs_ScreenView::onAnalogInputPass(uint8_t channel)
{
    inputs_Screen1.onAnalogInputPass(channel);
}

void Inputs_ScreenView::onAnalogInputFail(uint8_t channel, uint8_t level, uint8_t failKind,
                                          float actual, float expected, float tol, uint8_t kind)
{
    inputs_Screen1.onAnalogInputFail(channel, level, failKind, actual, expected, tol, kind);
}

void Inputs_ScreenView::onAnalogInputsComplete()
{
    inputs_Screen1.onAnalogInputsComplete();
}
