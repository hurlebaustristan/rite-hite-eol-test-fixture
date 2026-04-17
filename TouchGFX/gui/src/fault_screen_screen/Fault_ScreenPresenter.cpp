#include <gui/fault_screen_screen/Fault_ScreenView.hpp>
#include <gui/fault_screen_screen/Fault_ScreenPresenter.hpp>

#ifndef SIMULATOR
extern "C" {
#include "auto_test.h"
}
#endif

Fault_ScreenPresenter::Fault_ScreenPresenter(Fault_ScreenView& v)
    : view(v)
{
}

void Fault_ScreenPresenter::activate()
{
#ifndef SIMULATOR
    AutoTest_ClearAutoMode();
#endif
    view.displayFault();
}

void Fault_ScreenPresenter::deactivate()
{
}
