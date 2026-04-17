#include <gui/outputs_screen_screen/Outputs_ScreenView.hpp>

Outputs_ScreenView::Outputs_ScreenView()
{

}

void Outputs_ScreenView::setupScreen()
{
    Outputs_ScreenViewBase::setupScreen();
    outputs_Screen1.prepareForActivation();
}

void Outputs_ScreenView::tearDownScreen()
{
    Outputs_ScreenViewBase::tearDownScreen();
}

void Outputs_ScreenView::onOutputItemPass(uint8_t item)
{
    outputs_Screen1.onOutputItemPass(item);
}

void Outputs_ScreenView::onOutputTestFail(uint8_t phase, uint8_t failKind,
                                          const char* expectedState, const char* actualState)
{
    outputs_Screen1.onOutputTestFail(phase, failKind, expectedState, actualState);
}

void Outputs_ScreenView::onOutputsComplete()
{
    outputs_Screen1.onOutputsComplete();
}
