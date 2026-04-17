#include <gui/outputs_screen_screen/Outputs_ScreenView.hpp>
#include <gui/outputs_screen_screen/Outputs_ScreenPresenter.hpp>

Outputs_ScreenPresenter::Outputs_ScreenPresenter(Outputs_ScreenView& v)
    : view(v)
{

}

void Outputs_ScreenPresenter::activate()
{
    if (model)
    {
        model->startOutputsTest();
    }
}

void Outputs_ScreenPresenter::deactivate()
{

}

void Outputs_ScreenPresenter::notifyOutputTestPass(uint8_t item)
{
    view.onOutputItemPass(item);
}

void Outputs_ScreenPresenter::notifyOutputTestFail(uint8_t phase, uint8_t failKind,
                                                   const char* expectedState,
                                                   const char* actualState)
{
    view.onOutputTestFail(phase, failKind, expectedState, actualState);
}

void Outputs_ScreenPresenter::notifyOutputsComplete()
{
    view.onOutputsComplete();
}
