#ifndef OUTPUTS_SCREEN_HPP
#define OUTPUTS_SCREEN_HPP

#include <gui_generated/containers/Outputs_ScreenBase.hpp>

class Outputs_Screen : public Outputs_ScreenBase
{
public:
    Outputs_Screen();
    virtual ~Outputs_Screen() {}

    virtual void initialize();
    void prepareForActivation();
    void onOutputItemPass(uint8_t item);
    void onOutputTestFail(uint8_t phase, uint8_t failKind,
                          const char* expectedState, const char* actualState);
    void onOutputsComplete();
protected:
    uint8_t passedMask;
    uint8_t passedCount;
};

#endif // OUTPUTS_SCREEN_HPP
