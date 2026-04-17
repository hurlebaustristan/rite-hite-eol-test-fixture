#include <gui/test_complete_screen_screen/Test_Complete_ScreenView.hpp>
#include <gui/test_complete_screen_screen/Test_Complete_ScreenPresenter.hpp>

#ifndef SIMULATOR
extern "C" {
#include "auto_test.h"
}
#endif

Test_Complete_ScreenPresenter::Test_Complete_ScreenPresenter(Test_Complete_ScreenView& v)
    : view(v)
{

}

void Test_Complete_ScreenPresenter::activate()
{
#ifndef SIMULATOR
    AutoTest_ClearAutoMode();
#endif
}

void Test_Complete_ScreenPresenter::deactivate()
{

}
