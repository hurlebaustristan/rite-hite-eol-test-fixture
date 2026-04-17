#ifndef START_SCREENVIEW_HPP
#define START_SCREENVIEW_HPP

#include <gui_generated/start_screen_screen/Start_ScreenViewBase.hpp>
#include <gui/start_screen_screen/Start_ScreenPresenter.hpp>

class Start_ScreenView : public Start_ScreenViewBase
{
public:
    Start_ScreenView();
    virtual ~Start_ScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
};

#endif // START_SCREENVIEW_HPP
