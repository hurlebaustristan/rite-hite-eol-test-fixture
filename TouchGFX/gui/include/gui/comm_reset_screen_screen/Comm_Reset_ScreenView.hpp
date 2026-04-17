#ifndef COMM_RESET_SCREENVIEW_HPP
#define COMM_RESET_SCREENVIEW_HPP

#include <gui_generated/comm_reset_screen_screen/Comm_Reset_ScreenViewBase.hpp>
#include <gui/comm_reset_screen_screen/Comm_Reset_ScreenPresenter.hpp>

class Comm_Reset_ScreenView : public Comm_Reset_ScreenViewBase
{
public:
    Comm_Reset_ScreenView();
    virtual ~Comm_Reset_ScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    /**
      * @brief  Called each frame by the framework.
      *         Forwards to the container's tick handler.
      */
    virtual void handleTickEvent();

    /**
      * @brief  Called by the Presenter when the Model reports a comm-test result.
      */
    void onCommTestResult(uint8_t stage, uint16_t errorCode);

    /**
      * @brief  Called by the Presenter when the Model reports stage progress.
      */
    void onCommTestProgress(uint8_t stage, uint8_t progressEvent);

protected:
};

#endif // COMM_RESET_SCREENVIEW_HPP
