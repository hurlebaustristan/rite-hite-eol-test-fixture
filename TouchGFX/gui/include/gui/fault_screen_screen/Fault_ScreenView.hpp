#ifndef FAULT_SCREENVIEW_HPP
#define FAULT_SCREENVIEW_HPP

#include <gui_generated/fault_screen_screen/Fault_ScreenViewBase.hpp>
#include <gui/fault_screen_screen/Fault_ScreenPresenter.hpp>

class Fault_ScreenView : public Fault_ScreenViewBase
{
public:
    Fault_ScreenView();
    virtual ~Fault_ScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    /**
      * @brief  Populate fault details in the container.
      */
    void displayFault();
protected:
};

#endif // FAULT_SCREENVIEW_HPP
