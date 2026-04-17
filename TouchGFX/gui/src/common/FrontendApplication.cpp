#include <gui/common/FrontendApplication.hpp>
#include <gui/common/FrontendHeap.hpp>
#include <gui/fault_screen_screen/Fault_ScreenView.hpp>
#include <gui/fault_screen_screen/Fault_ScreenPresenter.hpp>
#include <touchgfx/transitions/NoTransition.hpp>
#include <cstring>

#ifndef SIMULATOR
extern "C" void triggerAutoTestScreenTransition(void)
{
    static_cast<FrontendApplication*>(
        touchgfx::Application::getInstance()
    )->gotoComm_Reset_ScreenScreenNoTransition();
}
#endif

/* Static fault payload — zero-initialised */
FaultInfo FrontendApplication::faultInfo = {};

FrontendApplication::FrontendApplication(Model& m, FrontendHeap& heap)
    : FrontendApplicationBase(m, heap),
      faultTransitionCallback(this, &FrontendApplication::gotoFault_ScreenScreenNoTransitionImpl)
{
}

/* --------------------------------------------------------------------- */
/*  gotoFault_ScreenScreenNoTransition — hand-written (not in Base)       */
/* --------------------------------------------------------------------- */
void FrontendApplication::gotoFault_ScreenScreenNoTransition()
{
    pendingScreenTransitionCallback = &faultTransitionCallback;
}

void FrontendApplication::gotoFault_ScreenScreenNoTransitionImpl()
{
    touchgfx::makeTransition<Fault_ScreenView, Fault_ScreenPresenter,
                             touchgfx::NoTransition, Model>(
        &currentScreen, &currentPresenter, frontendHeap,
        &currentTransition, &model);
}
