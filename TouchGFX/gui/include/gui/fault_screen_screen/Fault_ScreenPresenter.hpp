#ifndef FAULT_SCREENPRESENTER_HPP
#define FAULT_SCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class Fault_ScreenView;

class Fault_ScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    Fault_ScreenPresenter(Fault_ScreenView& v);

    virtual void activate();
    virtual void deactivate();

    virtual ~Fault_ScreenPresenter() {}

private:
    Fault_ScreenPresenter();
    Fault_ScreenView& view;
};

#endif // FAULT_SCREENPRESENTER_HPP
