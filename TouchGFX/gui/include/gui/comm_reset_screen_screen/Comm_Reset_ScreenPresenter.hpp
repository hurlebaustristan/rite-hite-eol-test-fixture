#ifndef COMM_RESET_SCREENPRESENTER_HPP
#define COMM_RESET_SCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class Comm_Reset_ScreenView;

class Comm_Reset_ScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    Comm_Reset_ScreenPresenter(Comm_Reset_ScreenView& v);

    virtual void activate();
    virtual void deactivate();

    virtual ~Comm_Reset_ScreenPresenter() {}

    /**
      * @brief  Override from ModelListener — forward comm-test result to the View.
      */
    virtual void notifyCommTestResult(uint8_t stage, uint16_t errorCode);

    /**
      * @brief  Forward non-terminal comm-test progress updates to the View.
      */
    virtual void notifyCommTestProgress(uint8_t stage, uint8_t progressEvent);

private:
    Comm_Reset_ScreenPresenter();
    Comm_Reset_ScreenView& view;
};

#endif // COMM_RESET_SCREENPRESENTER_HPP
