#ifndef BUTTONS_LEDS_SCREEN_HPP
#define BUTTONS_LEDS_SCREEN_HPP

#include <gui_generated/containers/Buttons_LEDs_ScreenBase.hpp>
#include <cstdint>
#include <touchgfx/Callback.hpp>
#include <touchgfx/containers/progress_indicators/AbstractProgressIndicator.hpp>
#include <touchgfx/widgets/AbstractButton.hpp>

class Buttons_LEDs_Screen : public Buttons_LEDs_ScreenBase
{
public:
    Buttons_LEDs_Screen();
    virtual ~Buttons_LEDs_Screen() {}

    virtual void initialize();
    void prepareForActivation();
    void tickHandler();

    void setGoodButtonAction(touchgfx::GenericCallback<const touchgfx::AbstractButton&>& callback);
    void setBadButtonAction(touchgfx::GenericCallback<const touchgfx::AbstractButton&>& callback);

    void onButtonItemReady(uint8_t item);
    void onButtonItemPass(uint8_t item);
    void onVisualReady();
    void onButtonLedTestFail(uint8_t step, uint8_t failKind,
                             const char* expectedToken, const char* actualText);
    void onButtonLedTestComplete();
protected:
    void setOperatorButtonsEnabled(bool enabled);
    void handleProgressNoOp(const touchgfx::AbstractProgressIndicator& src);

    uint8_t passedMask;
    uint8_t passedCount;
    bool visualReady;
    bool operatorDecisionLocked;
    bool successTransitionPending;
    uint32_t successTimestamp;
    touchgfx::Callback<Buttons_LEDs_Screen, const touchgfx::AbstractProgressIndicator&> progressNoOpCallback;
};

#endif // BUTTONS_LEDS_SCREEN_HPP
