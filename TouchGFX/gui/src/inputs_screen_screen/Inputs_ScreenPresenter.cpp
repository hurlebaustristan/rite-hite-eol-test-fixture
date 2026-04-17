#include <gui/inputs_screen_screen/Inputs_ScreenView.hpp>
#include <gui/inputs_screen_screen/Inputs_ScreenPresenter.hpp>

Inputs_ScreenPresenter::Inputs_ScreenPresenter(Inputs_ScreenView& v)
    : view(v)
{

}

void Inputs_ScreenPresenter::activate()
{
    model->startDigitalInputsTest();
}

void Inputs_ScreenPresenter::deactivate()
{

}

void Inputs_ScreenPresenter::notifyDigitalInputTestPass(uint8_t channel)
{
    view.onDigitalInputPass(channel);
}

void Inputs_ScreenPresenter::notifyDigitalInputTestFail(uint8_t channel, uint8_t failKind)
{
    view.onDigitalInputFail(channel, failKind);
}

void Inputs_ScreenPresenter::notifyDigitalInputsComplete()
{
    view.onDigitalInputsComplete();
    if (model)
    {
        model->startAnalogInputsTest();
    }
}

void Inputs_ScreenPresenter::notifyAnalogInputTestPass(uint8_t channel)
{
    view.onAnalogInputPass(channel);
}

void Inputs_ScreenPresenter::notifyAnalogInputTestFail(uint8_t channel, uint8_t level,
                                                       uint8_t failKind, float actual,
                                                       float expected, float tol, uint8_t kind)
{
    view.onAnalogInputFail(channel, level, failKind, actual, expected, tol, kind);
}

void Inputs_ScreenPresenter::notifyAnalogInputsComplete()
{
    view.onAnalogInputsComplete();
}
