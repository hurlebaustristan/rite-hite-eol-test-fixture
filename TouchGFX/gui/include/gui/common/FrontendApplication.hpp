#ifndef FRONTENDAPPLICATION_HPP
#define FRONTENDAPPLICATION_HPP

#include <gui_generated/common/FrontendApplicationBase.hpp>
#include <cstdint>

class FrontendHeap;

using namespace touchgfx;

/* ======================================================================== */
/*  Shared fault data — written before gotoFault_Screen, read on arrival    */
/* ======================================================================== */
struct FaultInfo
{
    uint16_t errorCode;         /* e.g. 0x0101                              */
    char     testName[32];      /* e.g. "Communication"                     */
    char     expected[32];      /* e.g. "PONG response"                     */
    char     actual[32];        /* e.g. "No response"                       */
};

class FrontendApplication : public FrontendApplicationBase
{
public:
    FrontendApplication(Model& m, FrontendHeap& heap);
    virtual ~FrontendApplication() { }

    virtual void handleTickEvent()
    {
        model.tick();
        FrontendApplicationBase::handleTickEvent();
    }

    /* ---- Navigate to Fault_Screen (not in generated base) ---- */
    void gotoFault_ScreenScreenNoTransition();

    /* ---- Shared fault payload ---- */
    static FaultInfo faultInfo;

private:
    void gotoFault_ScreenScreenNoTransitionImpl();
    touchgfx::Callback<FrontendApplication> faultTransitionCallback;
};

#endif // FRONTENDAPPLICATION_HPP
