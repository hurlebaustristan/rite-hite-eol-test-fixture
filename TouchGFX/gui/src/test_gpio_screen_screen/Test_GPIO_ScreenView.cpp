#include <gui/test_gpio_screen_screen/Test_GPIO_ScreenView.hpp>

Test_GPIO_ScreenView::Test_GPIO_ScreenView()
{

}

void Test_GPIO_ScreenView::setupScreen()
{
    Test_GPIO_ScreenViewBase::setupScreen();
}

void Test_GPIO_ScreenView::tearDownScreen()
{
    Test_GPIO_ScreenViewBase::tearDownScreen();
}

void Test_GPIO_ScreenView::updateDigitalInputColors(uint8_t state)
{
    test_GPIO_Container1.updateDigitalInputColors(state);
}
