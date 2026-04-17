#ifndef TEST_COMPLETE_SCREENVIEW_HPP
#define TEST_COMPLETE_SCREENVIEW_HPP

#include <gui_generated/test_complete_screen_screen/Test_Complete_ScreenViewBase.hpp>
#include <gui/test_complete_screen_screen/Test_Complete_ScreenPresenter.hpp>

class Test_Complete_ScreenView : public Test_Complete_ScreenViewBase
{
public:
    Test_Complete_ScreenView();
    virtual ~Test_Complete_ScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
};

#endif // TEST_COMPLETE_SCREENVIEW_HPP
