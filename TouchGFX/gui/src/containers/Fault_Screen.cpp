#include <gui/containers/Fault_Screen.hpp>
#include <gui/common/FrontendApplication.hpp>
#include <touchgfx/Unicode.hpp>
#include <cstdio>
#include <cstring>

Fault_Screen::Fault_Screen()
{
    memset(errorCodeBuf,  0, sizeof(errorCodeBuf));
    memset(testFailedBuf, 0, sizeof(testFailedBuf));
    memset(expectedBuf,   0, sizeof(expectedBuf));
    memset(actualBuf,     0, sizeof(actualBuf));
}

void Fault_Screen::initialize()
{
    Fault_ScreenBase::initialize();

    Expected_Value.setPosition(9, 341, 135, 80);
    Expected_Value.setWideTextAction(WIDE_TEXT_WORDWRAP_ELLIPSIS_AFTER_SPACE);

    Actual_Value.setPosition(172, 341, 135, 80);
    Actual_Value.setWideTextAction(WIDE_TEXT_WORDWRAP_ELLIPSIS_AFTER_SPACE);
}

/* --------------------------------------------------------------------- */
/*  showFault — read FaultInfo and set the wildcard buffers               */
/* --------------------------------------------------------------------- */
void Fault_Screen::showFault()
{
    const FaultInfo& fi = FrontendApplication::faultInfo;

    /* Error Code — e.g. "0x0101" */
    char tmp[12];
    snprintf(tmp, sizeof(tmp), "0x%04X", fi.errorCode);
    touchgfx::Unicode::strncpy(errorCodeBuf, tmp, BUF_SIZE);
    Error_Code.setWildcard(errorCodeBuf);
    Error_Code.resizeToCurrentText();
    Error_Code.invalidate();

    /* Test Failed — plain English name */
    touchgfx::Unicode::strncpy(testFailedBuf, fi.testName, BUF_SIZE);
    Test_Failed.setWildcard(testFailedBuf);
    Test_Failed.resizeToCurrentText();
    Test_Failed.invalidate();

    /* Expected */
    touchgfx::Unicode::strncpy(expectedBuf, fi.expected, BUF_SIZE);
    Expected_Value.setWildcard(expectedBuf);
    Expected_Value.invalidate();

    /* Actual */
    touchgfx::Unicode::strncpy(actualBuf, fi.actual, BUF_SIZE);
    Actual_Value.setWildcard(actualBuf);
    Actual_Value.invalidate();
}

