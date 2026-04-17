#include <gui/comm_reset_screen_screen/Comm_Reset_ScreenView.hpp>

Comm_Reset_ScreenView::Comm_Reset_ScreenView()
{
}

void Comm_Reset_ScreenView::setupScreen()
{
    Comm_Reset_ScreenViewBase::setupScreen();
    comm_Reset_Screen1.prepareForActivation();
}

void Comm_Reset_ScreenView::tearDownScreen()
{
    Comm_Reset_ScreenViewBase::tearDownScreen();
}

void Comm_Reset_ScreenView::handleTickEvent()
{
    /* Preserve base tick processing, then run container timers/transitions. */
    Comm_Reset_ScreenViewBase::handleTickEvent();

    comm_Reset_Screen1.tickHandler();
}

void Comm_Reset_ScreenView::onCommTestResult(uint8_t stage, uint16_t errorCode)
{
    comm_Reset_Screen1.onCommTestResult(stage, errorCode);
}

void Comm_Reset_ScreenView::onCommTestProgress(uint8_t stage, uint8_t progressEvent)
{
    comm_Reset_Screen1.onCommTestProgress(stage, progressEvent);
}
