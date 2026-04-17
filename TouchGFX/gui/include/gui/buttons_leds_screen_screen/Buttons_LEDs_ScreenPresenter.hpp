#ifndef BUTTONS_LEDS_SCREENPRESENTER_HPP
#define BUTTONS_LEDS_SCREENPRESENTER_HPP

#include <cstdint>
#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class Buttons_LEDs_ScreenView;

class Buttons_LEDs_ScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    Buttons_LEDs_ScreenPresenter(Buttons_LEDs_ScreenView& v);

    /**
     * The activate function is called automatically when this screen is "switched in"
     * (ie. made active). Initialization logic can be placed here.
     */
    virtual void activate();

    /**
     * The deactivate function is called automatically when this screen is "switched out"
     * (ie. made inactive). Teardown functionality can be placed here.
     */
    virtual void deactivate();

    virtual ~Buttons_LEDs_ScreenPresenter() {}

    void submitVisualResult(bool good);
    virtual void notifyButtonLedTestReady(uint8_t item);
    virtual void notifyButtonLedTestPass(uint8_t item);
    virtual void notifyButtonLedVisualReady();
    virtual void notifyButtonLedTestFail(uint8_t step, uint8_t failKind,
                                         const char* expectedToken,
                                         const char* actualText);
    virtual void notifyButtonLedTestComplete();

private:
    Buttons_LEDs_ScreenPresenter();

    Buttons_LEDs_ScreenView& view;
};

#endif // BUTTONS_LEDS_SCREENPRESENTER_HPP
