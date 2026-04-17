#ifndef TEST_COMPLETE_SCREEN_HPP
#define TEST_COMPLETE_SCREEN_HPP

#include <gui_generated/containers/Test_Complete_ScreenBase.hpp>

class Test_Complete_Screen : public Test_Complete_ScreenBase
{
public:
    Test_Complete_Screen();
    virtual ~Test_Complete_Screen() {}

    virtual void initialize();
protected:
};

#endif // TEST_COMPLETE_SCREEN_HPP
