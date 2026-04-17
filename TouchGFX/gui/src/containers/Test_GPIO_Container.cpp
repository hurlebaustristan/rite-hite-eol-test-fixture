#include <gui/containers/Test_GPIO_Container.hpp>
#include <touchgfx/Color.hpp>

#ifndef SIMULATOR
extern "C" {
#include "gpio_config.h"
}
#endif

/* Colour constants for digital-input text */
static const touchgfx::colortype COLOR_HIGH = touchgfx::Color::getColorFromRGB(0, 255, 0);   /* Green */
static const touchgfx::colortype COLOR_LOW  = touchgfx::Color::getColorFromRGB(255, 0, 0);   /* Red   */

/* --------------------------------------------------------------------- */
/*  Constructor                                                          */
/* --------------------------------------------------------------------- */
Test_GPIO_Container::Test_GPIO_Container()
    : sliderCallback(this, &Test_GPIO_Container::sliderValueChangedHandler),
      toggleCallback(this, &Test_GPIO_Container::toggleButtonHandler),
      lastInputState(0xFF)   /* Force first update (impossible real value) */
{
}

/* --------------------------------------------------------------------- */
/*  initialize — register callbacks on every toggle button and slider    */
/* --------------------------------------------------------------------- */
void Test_GPIO_Container::initialize()
{
    Test_GPIO_ContainerBase::initialize();

    /* Slider → DAC output */
    DAC_PA4.setNewValueCallback(sliderCallback);

    /* Row 1: DI1-DI5 */
    DI1_PF5.setAction(toggleCallback);
    DI2_PF3.setAction(toggleCallback);
    DI3_PD2.setAction(toggleCallback);
    DI4_PC12.setAction(toggleCallback);
    DI5_PC11.setAction(toggleCallback);

    /* Row 2: DI6-DI10 */
    DI6_PC10.setAction(toggleCallback);
    DI7_PC9.setAction(toggleCallback);
    DI8_PC8.setAction(toggleCallback);
    DI9_PA3.setAction(toggleCallback);
    DI10_PC3.setAction(toggleCallback);

    /* Row 3: DI11-DI12, AUX0-AUX2 */
    DI11_PC1.setAction(toggleCallback);
    DI12_PC0.setAction(toggleCallback);
    AUX0BTN_PE0.setAction(toggleCallback);
    AUX1BTN_PE3.setAction(toggleCallback);
    AUX2BTN_PF8.setAction(toggleCallback);

    /* Row 4: AUX3, USR, MCU, FLT */
    AUX3BTN_PF2.setAction(toggleCallback);
    USRBTN_PB2.setAction(toggleCallback);
    MCUBTN_PB11.setAction(toggleCallback);
    FLTBTN_PA8.setAction(toggleCallback);

    /* Row 5: DACA0, DACEN, DACA1 */
    DACA0_PB3.setAction(toggleCallback);
    DACEN_PD11.setAction(toggleCallback);
    DACA1_PC6.setAction(toggleCallback);
}

/* --------------------------------------------------------------------- */
/*  toggleButtonHandler — called after any ToggleButton is pressed.      */
/*  getState() == true  → toggled ON  → GPIO HIGH                       */
/*  getState() == false → toggled OFF → GPIO LOW                        */
/* --------------------------------------------------------------------- */
void Test_GPIO_Container::toggleButtonHandler(const touchgfx::AbstractButton& src)
{
#ifndef SIMULATOR
    /* Helper: get the new pin state from the toggle's current state.
       ToggleButton::getState() returns true when in the "on" bitmap. */
    auto pinState = [](const touchgfx::ToggleButton& btn) -> GPIO_PinState {
        return btn.getState() ? GPIO_PIN_SET : GPIO_PIN_RESET;
    };

    /* --- Row 1: DI1-DI5 --- */
    if (&src == &DI1_PF5)       { GPIO_Config_WriteDI(1,  pinState(DI1_PF5));  return; }
    if (&src == &DI2_PF3)       { GPIO_Config_WriteDI(2,  pinState(DI2_PF3));  return; }
    if (&src == &DI3_PD2)       { GPIO_Config_WriteDI(3,  pinState(DI3_PD2));  return; }
    if (&src == &DI4_PC12)      { GPIO_Config_WriteDI(4,  pinState(DI4_PC12)); return; }
    if (&src == &DI5_PC11)      { GPIO_Config_WriteDI(5,  pinState(DI5_PC11)); return; }

    /* --- Row 2: DI6-DI10 --- */
    if (&src == &DI6_PC10)      { GPIO_Config_WriteDI(6,  pinState(DI6_PC10)); return; }
    if (&src == &DI7_PC9)       { GPIO_Config_WriteDI(7,  pinState(DI7_PC9));  return; }
    if (&src == &DI8_PC8)       { GPIO_Config_WriteDI(8,  pinState(DI8_PC8));  return; }
    if (&src == &DI9_PA3)       { GPIO_Config_WriteDI(9,  pinState(DI9_PA3));  return; }
    if (&src == &DI10_PC3)      { GPIO_Config_WriteDI(10, pinState(DI10_PC3)); return; }

    /* --- Row 3: DI11-DI12, AUX0-AUX2 --- */
    if (&src == &DI11_PC1)      { GPIO_Config_WriteDI(11, pinState(DI11_PC1)); return; }
    if (&src == &DI12_PC0)      { GPIO_Config_WriteDI(12, pinState(DI12_PC0)); return; }
    if (&src == &AUX0BTN_PE0)   { GPIO_Config_WriteAuxBtn(0, pinState(AUX0BTN_PE0)); return; }
    if (&src == &AUX1BTN_PE3)   { GPIO_Config_WriteAuxBtn(1, pinState(AUX1BTN_PE3)); return; }
    if (&src == &AUX2BTN_PF8)   { GPIO_Config_WriteAuxBtn(2, pinState(AUX2BTN_PF8)); return; }

    /* --- Row 4: AUX3, USR, MCU, FLT --- */
    if (&src == &AUX3BTN_PF2)   { GPIO_Config_WriteAuxBtn(3, pinState(AUX3BTN_PF2)); return; }
    if (&src == &USRBTN_PB2)    { GPIO_Config_WriteCtrlBtn(0, pinState(USRBTN_PB2));  return; }
    if (&src == &MCUBTN_PB11)   { GPIO_Config_WriteCtrlBtn(1, pinState(MCUBTN_PB11)); return; }
    if (&src == &FLTBTN_PA8)    { GPIO_Config_WriteCtrlBtn(2, pinState(FLTBTN_PA8));  return; }

    /* --- Row 5: DACA0, DACEN, DACA1 --- */
    if (&src == &DACA0_PB3)     { GPIO_Config_WriteDacA0(pinState(DACA0_PB3)); return; }
    if (&src == &DACEN_PD11)    { GPIO_Config_WriteDacEn(pinState(DACEN_PD11)); return; }
    if (&src == &DACA1_PC6)     { GPIO_Config_WriteDacA1(pinState(DACA1_PC6)); return; }
#endif
}

/* --------------------------------------------------------------------- */
/*  Slider callback — drive DAC output on PA4 (0-330 → 0-3.3 V)         */
/* --------------------------------------------------------------------- */
void Test_GPIO_Container::sliderValueChangedHandler(const touchgfx::Slider& src, int value)
{
#ifndef SIMULATOR
    DAC_SetFromSlider((uint16_t)value);
#endif
}

/* --------------------------------------------------------------------- */
/*  updateDigitalInputColors — set text GREEN or RED per GPIO state       */
/* --------------------------------------------------------------------- */
void Test_GPIO_Container::updateDigitalInputColors(uint8_t state)
{
    /* Only redraw when something actually changed */
    if (state == lastInputState)
    {
        return;
    }
    lastInputState = state;

    /* Map each bit to its TextArea widget:
     *   bit 0 = K1NO,  bit 1 = K1NC,  bit 2 = K2NO,
     *   bit 3 = K2NC,  bit 4 = DO1,   bit 5 = DO2
     */
    touchgfx::TextArea* texts[] = {
        &K1NO_Read, &K1NC_Read, &K2NO_Read,
        &K2NC_Read, &DO1_Read,  &DO2_Read
    };

    for (int i = 0; i < 6; i++)
    {
        touchgfx::colortype color = (state & (1U << i)) ? COLOR_HIGH : COLOR_LOW;
        texts[i]->setColor(color);
        texts[i]->invalidate();
    }
}
