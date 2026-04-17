#ifndef INPUTS_SCREENPRESENTER_HPP
#define INPUTS_SCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class Inputs_ScreenView;

class Inputs_ScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    Inputs_ScreenPresenter(Inputs_ScreenView& v);

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

    virtual void notifyDigitalInputTestPass(uint8_t channel);
    virtual void notifyDigitalInputTestFail(uint8_t channel, uint8_t failKind);
    virtual void notifyDigitalInputsComplete();
    virtual void notifyAnalogInputTestPass(uint8_t channel);
    virtual void notifyAnalogInputTestFail(uint8_t channel, uint8_t level,
                                           uint8_t failKind, float actual,
                                           float expected, float tol, uint8_t kind);
    virtual void notifyAnalogInputsComplete();

    virtual ~Inputs_ScreenPresenter() {}

private:
    Inputs_ScreenPresenter();

    Inputs_ScreenView& view;
};

#endif // INPUTS_SCREENPRESENTER_HPP
