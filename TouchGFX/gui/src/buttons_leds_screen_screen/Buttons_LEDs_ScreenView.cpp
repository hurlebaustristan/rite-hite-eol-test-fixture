#include <gui/buttons_leds_screen_screen/Buttons_LEDs_ScreenView.hpp>

Buttons_LEDs_ScreenView::Buttons_LEDs_ScreenView()
    : goodButtonPressedCallback(this, &Buttons_LEDs_ScreenView::handleGoodPressed),
      badButtonPressedCallback(this, &Buttons_LEDs_ScreenView::handleBadPressed)
{

}

void Buttons_LEDs_ScreenView::setupScreen()
{
    Buttons_LEDs_ScreenViewBase::setupScreen();
    buttons_LEDs_Screen1.prepareForActivation();
    buttons_LEDs_Screen1.setGoodButtonAction(goodButtonPressedCallback);
    buttons_LEDs_Screen1.setBadButtonAction(badButtonPressedCallback);
}

void Buttons_LEDs_ScreenView::tearDownScreen()
{
    Buttons_LEDs_ScreenViewBase::tearDownScreen();
}

void Buttons_LEDs_ScreenView::handleTickEvent()
{
    Buttons_LEDs_ScreenViewBase::handleTickEvent();
    buttons_LEDs_Screen1.tickHandler();
}

void Buttons_LEDs_ScreenView::onButtonLedItemReady(uint8_t item)
{
    buttons_LEDs_Screen1.onButtonItemReady(item);
}

void Buttons_LEDs_ScreenView::onButtonLedItemPass(uint8_t item)
{
    buttons_LEDs_Screen1.onButtonItemPass(item);
}

void Buttons_LEDs_ScreenView::onButtonLedVisualReady()
{
    buttons_LEDs_Screen1.onVisualReady();
}

void Buttons_LEDs_ScreenView::onButtonLedTestFail(uint8_t step, uint8_t failKind,
                                                  const char* expectedToken,
                                                  const char* actualText)
{
    buttons_LEDs_Screen1.onButtonLedTestFail(step, failKind, expectedToken, actualText);
}

void Buttons_LEDs_ScreenView::onButtonLedTestComplete()
{
    buttons_LEDs_Screen1.onButtonLedTestComplete();
}

void Buttons_LEDs_ScreenView::handleGoodPressed(const touchgfx::AbstractButton& src)
{
    (void)src;

    if (presenter)
    {
        presenter->submitVisualResult(true);
    }
}

void Buttons_LEDs_ScreenView::handleBadPressed(const touchgfx::AbstractButton& src)
{
    (void)src;

    if (presenter)
    {
        presenter->submitVisualResult(false);
    }
}
