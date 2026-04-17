#ifndef FAULT_SCREEN_HPP
#define FAULT_SCREEN_HPP

#include <gui_generated/containers/Fault_ScreenBase.hpp>
#include <touchgfx/Unicode.hpp>

class Fault_Screen : public Fault_ScreenBase
{
public:
    Fault_Screen();
    virtual ~Fault_Screen() {}

    virtual void initialize();

    /**
      * @brief  Populate all four wildcard text areas from the shared FaultInfo.
      */
    void showFault();

protected:
    /* Wildcard buffers — TextAreaWithOneWildcard needs a Unicode::UnicodeChar* */
    static const uint16_t BUF_SIZE = 40;
    touchgfx::Unicode::UnicodeChar errorCodeBuf[BUF_SIZE];
    touchgfx::Unicode::UnicodeChar testFailedBuf[BUF_SIZE];
    touchgfx::Unicode::UnicodeChar expectedBuf[BUF_SIZE];
    touchgfx::Unicode::UnicodeChar actualBuf[BUF_SIZE];
};

#endif // FAULT_SCREEN_HPP
