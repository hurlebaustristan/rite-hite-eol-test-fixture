#ifndef START_SCREEN_CONTAINER_HPP
#define START_SCREEN_CONTAINER_HPP

#include <gui_generated/containers/Start_Screen_ContainerBase.hpp>

class Start_Screen_Container : public Start_Screen_ContainerBase
{
public:
    Start_Screen_Container();
    virtual ~Start_Screen_Container() {}

    virtual void initialize();
protected:
};

#endif // START_SCREEN_CONTAINER_HPP
