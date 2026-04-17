#ifndef COMM_RESET_SCREEN_HPP
#define COMM_RESET_SCREEN_HPP

#include <gui_generated/containers/Comm_Reset_ScreenBase.hpp>

class Comm_Reset_Screen : public Comm_Reset_ScreenBase
{
public:
    Comm_Reset_Screen();
    virtual ~Comm_Reset_Screen() {}

    virtual void initialize();

    /**
      * @brief  Reset transient UI/fault state each time the screen is shown.
      *         TouchGFX screen objects are reused, so constructor state alone
      *         is not sufficient after a failed run.
      */
    void prepareForActivation();

    /**
      * @brief  Called by the View when the comm-test result arrives.
      *         Turns the "Communication" / "Connected" text green on pass,
      *         or red on failure (then waits 3 s before navigating to Fault).
      *
      * @param  stage      EOL_STAGE_COMM, etc.
      * @param  errorCode  EOL_ERR_NONE on success.
      */
    void onCommTestResult(uint8_t stage, uint16_t errorCode);

    /**
      * @brief  Called by the View for non-terminal stage progress updates.
      *         Used for partial UI coloring during MCU Reset (button pulse done).
      */
    void onCommTestProgress(uint8_t stage, uint8_t progressEvent);

    /**
      * @brief  Called every tick by the View so the container can manage
      *         the 3-second red-text delay before moving to the Fault screen.
      */
    void tickHandler();

protected:
    uint32_t successTimestamp;   /* HAL_GetTick snapshot when tests complete */
    bool     successTransitionPending; /* wait 1 s before going to Inputs */
    uint32_t faultTimestamp;     /* HAL_GetTick snapshot when fault detected */
    bool     faultPending;       /* true while waiting 3 s with red text    */
    uint16_t pendingErrorCode;   /* saved so the Fault screen can show it   */
    uint8_t  pendingStage;       /* saved stage for the Fault screen        */
};

#endif // COMM_RESET_SCREEN_HPP
