#ifndef TEST_GPIO_CONTAINER_HPP
#define TEST_GPIO_CONTAINER_HPP

#include <gui_generated/containers/Test_GPIO_ContainerBase.hpp>

class Test_GPIO_Container : public Test_GPIO_ContainerBase
{
public:
    Test_GPIO_Container();
    virtual ~Test_GPIO_Container() {}

    virtual void initialize();

    /* Slider callback */
    void sliderValueChangedHandler(const touchgfx::Slider& src, int value);

    /* Single handler for ALL toggle-button outputs */
    void toggleButtonHandler(const touchgfx::AbstractButton& src);

    /**
      * @brief  Update the 6 digital-input text colours.
      *         GREEN = HIGH (1),  RED = LOW (0).
      * @param  state  Packed bits: 0=K1NO, 1=K1NC, 2=K2NO, 3=K2NC, 4=DO1, 5=DO2
      */
    void updateDigitalInputColors(uint8_t state);

protected:
    touchgfx::Callback<Test_GPIO_Container, const touchgfx::Slider&, int> sliderCallback;
    touchgfx::Callback<Test_GPIO_Container, const touchgfx::AbstractButton&> toggleCallback;

private:
    uint8_t lastInputState;   /**< Cache to avoid redundant redraws */
};

#endif // TEST_GPIO_CONTAINER_HPP
