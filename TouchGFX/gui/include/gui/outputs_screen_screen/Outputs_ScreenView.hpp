#ifndef OUTPUTS_SCREENVIEW_HPP
#define OUTPUTS_SCREENVIEW_HPP

#include <gui_generated/outputs_screen_screen/Outputs_ScreenViewBase.hpp>
#include <gui/outputs_screen_screen/Outputs_ScreenPresenter.hpp>

class Outputs_ScreenView : public Outputs_ScreenViewBase
{
public:
    Outputs_ScreenView();
    virtual ~Outputs_ScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    void onOutputItemPass(uint8_t item);
    void onOutputTestFail(uint8_t phase, uint8_t failKind,
                          const char* expectedState, const char* actualState);
    void onOutputsComplete();
protected:
};

#endif // OUTPUTS_SCREENVIEW_HPP
