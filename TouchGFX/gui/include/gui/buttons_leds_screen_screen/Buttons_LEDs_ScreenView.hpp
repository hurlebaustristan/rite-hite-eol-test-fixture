#ifndef BUTTONS_LEDS_SCREENVIEW_HPP
#define BUTTONS_LEDS_SCREENVIEW_HPP

#include <cstdint>
#include <gui_generated/buttons_leds_screen_screen/Buttons_LEDs_ScreenViewBase.hpp>
#include <gui/buttons_leds_screen_screen/Buttons_LEDs_ScreenPresenter.hpp>
#include <touchgfx/Callback.hpp>
#include <touchgfx/widgets/AbstractButton.hpp>

class Buttons_LEDs_ScreenView : public Buttons_LEDs_ScreenViewBase
{
public:
    Buttons_LEDs_ScreenView();
    virtual ~Buttons_LEDs_ScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();

    void onButtonLedItemReady(uint8_t item);
    void onButtonLedItemPass(uint8_t item);
    void onButtonLedVisualReady();
    void onButtonLedTestFail(uint8_t step, uint8_t failKind,
                             const char* expectedToken, const char* actualText);
    void onButtonLedTestComplete();

    void handleGoodPressed(const touchgfx::AbstractButton& src);
    void handleBadPressed(const touchgfx::AbstractButton& src);
protected:
    touchgfx::Callback<Buttons_LEDs_ScreenView, const touchgfx::AbstractButton&> goodButtonPressedCallback;
    touchgfx::Callback<Buttons_LEDs_ScreenView, const touchgfx::AbstractButton&> badButtonPressedCallback;
};

#endif // BUTTONS_LEDS_SCREENVIEW_HPP
