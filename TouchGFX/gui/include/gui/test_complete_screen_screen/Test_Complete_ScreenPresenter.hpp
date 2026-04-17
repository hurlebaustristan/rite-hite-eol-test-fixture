#ifndef TEST_COMPLETE_SCREENPRESENTER_HPP
#define TEST_COMPLETE_SCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class Test_Complete_ScreenView;

class Test_Complete_ScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    Test_Complete_ScreenPresenter(Test_Complete_ScreenView& v);

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

    virtual ~Test_Complete_ScreenPresenter() {}

private:
    Test_Complete_ScreenPresenter();

    Test_Complete_ScreenView& view;
};

#endif // TEST_COMPLETE_SCREENPRESENTER_HPP
