#include <gui/fault_screen_screen/Fault_ScreenView.hpp>

Fault_ScreenView::Fault_ScreenView()
{
}

void Fault_ScreenView::setupScreen()
{
    Fault_ScreenViewBase::setupScreen();
    displayFault();
}

void Fault_ScreenView::tearDownScreen()
{
    Fault_ScreenViewBase::tearDownScreen();
}

void Fault_ScreenView::displayFault()
{
    fault_Screen1.showFault();
}

