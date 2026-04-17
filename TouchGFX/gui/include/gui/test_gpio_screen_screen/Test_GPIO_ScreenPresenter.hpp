#ifndef TEST_GPIO_SCREENPRESENTER_HPP
#define TEST_GPIO_SCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class Test_GPIO_ScreenView;

class Test_GPIO_ScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    Test_GPIO_ScreenPresenter(Test_GPIO_ScreenView& v);

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

    /**
      * @brief  Receives packed digital-input state from Model::tick().
      *         Forwards to the View so the container can update text colours.
      */
    virtual void notifyDigitalInputState(uint8_t state);

    virtual ~Test_GPIO_ScreenPresenter() {}

private:
    Test_GPIO_ScreenPresenter();

    Test_GPIO_ScreenView& view;
};

#endif // TEST_GPIO_SCREENPRESENTER_HPP
