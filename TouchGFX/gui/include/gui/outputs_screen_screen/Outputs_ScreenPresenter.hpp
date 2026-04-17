#ifndef OUTPUTS_SCREENPRESENTER_HPP
#define OUTPUTS_SCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class Outputs_ScreenView;

class Outputs_ScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    Outputs_ScreenPresenter(Outputs_ScreenView& v);

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

    virtual void notifyOutputTestPass(uint8_t item);
    virtual void notifyOutputTestFail(uint8_t phase, uint8_t failKind,
                                      const char* expectedState,
                                      const char* actualState);
    virtual void notifyOutputsComplete();

    virtual ~Outputs_ScreenPresenter() {}

private:
    Outputs_ScreenPresenter();

    Outputs_ScreenView& view;
};

#endif // OUTPUTS_SCREENPRESENTER_HPP
