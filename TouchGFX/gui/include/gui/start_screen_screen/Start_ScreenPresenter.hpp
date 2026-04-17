#ifndef START_SCREENPRESENTER_HPP
#define START_SCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class Start_ScreenView;

class Start_ScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    Start_ScreenPresenter(Start_ScreenView& v);

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

    virtual ~Start_ScreenPresenter() {}

private:
    Start_ScreenPresenter();

    Start_ScreenView& view;
};

#endif // START_SCREENPRESENTER_HPP
