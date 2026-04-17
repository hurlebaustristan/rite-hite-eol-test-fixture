#include <gui/buttons_leds_screen_screen/Buttons_LEDs_ScreenView.hpp>
#include <gui/buttons_leds_screen_screen/Buttons_LEDs_ScreenPresenter.hpp>

Buttons_LEDs_ScreenPresenter::Buttons_LEDs_ScreenPresenter(Buttons_LEDs_ScreenView& v)
    : view(v)
{

}

void Buttons_LEDs_ScreenPresenter::activate()
{
    if (model)
    {
        model->startButtonLedTest();
    }
}

void Buttons_LEDs_ScreenPresenter::deactivate()
{

}

void Buttons_LEDs_ScreenPresenter::submitVisualResult(bool good)
{
    if (model)
    {
        model->submitButtonLedVisualResult(good);
    }
}

void Buttons_LEDs_ScreenPresenter::notifyButtonLedTestReady(uint8_t item)
{
    view.onButtonLedItemReady(item);
}

void Buttons_LEDs_ScreenPresenter::notifyButtonLedTestPass(uint8_t item)
{
    view.onButtonLedItemPass(item);
}

void Buttons_LEDs_ScreenPresenter::notifyButtonLedVisualReady()
{
    view.onButtonLedVisualReady();
}

void Buttons_LEDs_ScreenPresenter::notifyButtonLedTestFail(uint8_t step, uint8_t failKind,
                                                           const char* expectedToken,
                                                           const char* actualText)
{
    view.onButtonLedTestFail(step, failKind, expectedToken, actualText);
}

void Buttons_LEDs_ScreenPresenter::notifyButtonLedTestComplete()
{
    view.onButtonLedTestComplete();
}
