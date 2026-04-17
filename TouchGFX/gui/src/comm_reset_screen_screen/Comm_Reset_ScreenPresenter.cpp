#include <gui/comm_reset_screen_screen/Comm_Reset_ScreenView.hpp>
#include <gui/comm_reset_screen_screen/Comm_Reset_ScreenPresenter.hpp>

Comm_Reset_ScreenPresenter::Comm_Reset_ScreenPresenter(Comm_Reset_ScreenView& v)
    : view(v)
{
}

void Comm_Reset_ScreenPresenter::activate()
{
    /* Start the PING/PONG comm test as soon as this screen becomes active */
    model->startCommTest();
}

void Comm_Reset_ScreenPresenter::deactivate()
{
}

void Comm_Reset_ScreenPresenter::notifyCommTestResult(uint8_t stage, uint16_t errorCode)
{
    view.onCommTestResult(stage, errorCode);
}

void Comm_Reset_ScreenPresenter::notifyCommTestProgress(uint8_t stage, uint8_t progressEvent)
{
    view.onCommTestProgress(stage, progressEvent);
}
