#include <gui/test_gpio_screen_screen/Test_GPIO_ScreenView.hpp>
#include <gui/test_gpio_screen_screen/Test_GPIO_ScreenPresenter.hpp>

Test_GPIO_ScreenPresenter::Test_GPIO_ScreenPresenter(Test_GPIO_ScreenView& v)
    : view(v)
{

}

void Test_GPIO_ScreenPresenter::activate()
{

}

void Test_GPIO_ScreenPresenter::deactivate()
{

}

void Test_GPIO_ScreenPresenter::notifyDigitalInputState(uint8_t state)
{
    view.updateDigitalInputColors(state);
}
