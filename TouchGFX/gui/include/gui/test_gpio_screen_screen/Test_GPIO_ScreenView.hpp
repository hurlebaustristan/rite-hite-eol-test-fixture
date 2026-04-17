#ifndef TEST_GPIO_SCREENVIEW_HPP
#define TEST_GPIO_SCREENVIEW_HPP

#include <gui_generated/test_gpio_screen_screen/Test_GPIO_ScreenViewBase.hpp>
#include <gui/test_gpio_screen_screen/Test_GPIO_ScreenPresenter.hpp>

class Test_GPIO_ScreenView : public Test_GPIO_ScreenViewBase
{
public:
    Test_GPIO_ScreenView();
    virtual ~Test_GPIO_ScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    /**
      * @brief  Called by the Presenter every tick with packed input state.
      *         Forwards to the Test_GPIO_Container to update text colours.
      */
    void updateDigitalInputColors(uint8_t state);

protected:
};

#endif // TEST_GPIO_SCREENVIEW_HPP
