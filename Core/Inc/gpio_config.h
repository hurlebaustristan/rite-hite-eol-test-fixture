/**
  ******************************************************************************
  * @file    gpio_config.h
  * @brief   EOL Test Fixture GPIO pin definitions and init prototype.
  *
  *          This file centralises every user-defined GPIO so that pin
  *          assignments can be changed in ONE place.  Group edits by row
  *          to keep the mapping readable.
  *
  *          ALL toggle-button GPIOs are OUTPUTS.  The TouchGFX toggle
  *          drives the pin HIGH (on) or LOW (off).
  ******************************************************************************
  */

#ifndef GPIO_CONFIG_H
#define GPIO_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32u5xx_hal.h"

/* ======================================================================== */
/*  ROW 1 — Digital Outputs 1-5                                             */
/* ======================================================================== */
#define DI1_Pin             GPIO_PIN_5
#define DI1_GPIO_Port       GPIOF

#define DI2_Pin             GPIO_PIN_3
#define DI2_GPIO_Port       GPIOF

#define DI3_Pin             GPIO_PIN_2
#define DI3_GPIO_Port       GPIOD

#define DI4_Pin             GPIO_PIN_12
#define DI4_GPIO_Port       GPIOC

#define DI5_Pin             GPIO_PIN_11
#define DI5_GPIO_Port       GPIOC

/* ======================================================================== */
/*  ROW 2 — Digital Outputs 6-10                                            */
/* ======================================================================== */
#define DI6_Pin             GPIO_PIN_10
#define DI6_GPIO_Port       GPIOC

#define DI7_Pin             GPIO_PIN_9
#define DI7_GPIO_Port       GPIOC

#define DI8_Pin             GPIO_PIN_8
#define DI8_GPIO_Port       GPIOC

#define DI9_Pin             GPIO_PIN_3
#define DI9_GPIO_Port       GPIOA

#define DI10_Pin            GPIO_PIN_3
#define DI10_GPIO_Port      GPIOC

/* ======================================================================== */
/*  ROW 3 — Digital Outputs 11-12  &  Aux Panel Buttons 0-2                 */
/* ======================================================================== */
#define DI11_Pin            GPIO_PIN_1
#define DI11_GPIO_Port      GPIOC

#define DI12_Pin            GPIO_PIN_0
#define DI12_GPIO_Port      GPIOC

#define AUX0BTN_Pin         GPIO_PIN_0
#define AUX0BTN_GPIO_Port   GPIOE

#define AUX1BTN_Pin         GPIO_PIN_3
#define AUX1BTN_GPIO_Port   GPIOE

#define AUX2BTN_Pin         GPIO_PIN_8
#define AUX2BTN_GPIO_Port   GPIOF

/* ======================================================================== */
/*  ROW 4 — Aux Panel Button 3  &  Control Buttons                         */
/* ======================================================================== */
#define AUX3BTN_Pin         GPIO_PIN_2
#define AUX3BTN_GPIO_Port   GPIOF

#define USRBTN_Pin          GPIO_PIN_2
#define USRBTN_GPIO_Port    GPIOB

#define MCUBTN_Pin          GPIO_PIN_11
#define MCUBTN_GPIO_Port    GPIOB

#define FLTBTN_Pin          GPIO_PIN_8
#define FLTBTN_GPIO_Port    GPIOA

/* ======================================================================== */
/*  ROW 5 — ADC Multiplexer Address Select (outputs)                        */
/* ======================================================================== */
#define DACA0_Pin           GPIO_PIN_3
#define DACA0_GPIO_Port     GPIOB

#define DACA1_Pin           GPIO_PIN_6
#define DACA1_GPIO_Port     GPIOC

#define DACEN_Pin           GPIO_PIN_11
#define DACEN_GPIO_Port     GPIOD

/* ======================================================================== */
/*  DIGITAL INPUTS — Read-back from DUT (active-high, pull-down)            */
/* ======================================================================== */
#define K1NO_Pin            GPIO_PIN_13
#define K1NO_GPIO_Port      GPIOB

#define K1NC_Pin            GPIO_PIN_12
#define K1NC_GPIO_Port      GPIOD

#define K2NO_Pin            GPIO_PIN_4
#define K2NO_GPIO_Port      GPIOB

#define K2NC_Pin            GPIO_PIN_5
#define K2NC_GPIO_Port      GPIOB

#define DO1_Pin             GPIO_PIN_1
#define DO1_GPIO_Port       GPIOB

#define DO2_Pin             GPIO_PIN_2
#define DO2_GPIO_Port       GPIOC

/* ======================================================================== */
/*  SLIDER — DAC Analog Output                                              */
/* ======================================================================== */
#define DAC_OUT_Pin         GPIO_PIN_4
#define DAC_OUT_GPIO_Port   GPIOA

/* ======================================================================== */
/*  Total pin count helpers (useful for iteration)                          */
/* ======================================================================== */
#define DIGITAL_OUTPUT_COUNT    12
#define AUX_BUTTON_COUNT         4
#define CONTROL_BUTTON_COUNT     3   /* USR, MCU-RST, FAULT */
#define DIGITAL_INPUT_COUNT      6   /* K1NO, K1NC, K2NO, K2NC, DO1, DO2 */

/* ======================================================================== */
/*  DAC Configuration                                                       */
/* ======================================================================== */

/** Slider range: 0 – 330  (maps to 0.00 V – 3.30 V) */
#define DAC_SLIDER_MAX      330

/** 12-bit DAC full-scale count */
#define DAC_12BIT_MAX       4095

/* ======================================================================== */
/*  Public API                                                              */
/* ======================================================================== */

/**
  * @brief  Initialise all EOL Test Fixture GPIOs as push-pull outputs (LOW).
  *         Call AFTER MX_GPIO_Init() so the port clocks are already running.
  */
void GPIO_Config_Init(void);

/**
  * @brief  Initialise DAC1 Channel 1 (PA4) using direct register access.
  */
void DAC_Config_Init(void);

/**
  * @brief  Set the DAC output from a 0-330 slider value.
  *         0 → 0.00 V,  330 → 3.30 V
  * @param  sliderValue  0 – 330
  */
void DAC_SetFromSlider(uint16_t sliderValue);

/**
  * @brief  Write a digital output (DI1-DI12).
  * @param  index  1-based (1..12)
  * @param  state  GPIO_PIN_SET (HIGH) or GPIO_PIN_RESET (LOW)
  */
void GPIO_Config_WriteDI(uint8_t index, GPIO_PinState state);

/**
  * @brief  Write an aux-panel button output (AUX0-AUX3).
  * @param  index  0-based (0..3)
  * @param  state  GPIO_PIN_SET or GPIO_PIN_RESET
  */
void GPIO_Config_WriteAuxBtn(uint8_t index, GPIO_PinState state);

/**
  * @brief  Write a control button output.
  * @param  index  0 = USR (PB2), 1 = MCU-RST (PB11), 2 = FAULT (PA8)
  * @param  state  GPIO_PIN_SET or GPIO_PIN_RESET
  */
void GPIO_Config_WriteCtrlBtn(uint8_t index, GPIO_PinState state);

/**
  * @brief  Write the DACA0 (PB3) mux-select output.
  * @param  state  GPIO_PIN_SET or GPIO_PIN_RESET
  */
void GPIO_Config_WriteDacA0(GPIO_PinState state);

/**
  * @brief  Write the DACA1 (PC6) mux-select output.
  * @param  state  GPIO_PIN_SET or GPIO_PIN_RESET
  */
void GPIO_Config_WriteDacA1(GPIO_PinState state);

/**
  * @brief  Write the DAC-enable output (PD11).
  * @param  state  GPIO_PIN_SET or GPIO_PIN_RESET
  */
void GPIO_Config_WriteDacEn(GPIO_PinState state);

/**
  * @brief  Set the ADG704 mux address (A1:A0).
  * @param  channel  0-3
  */
void GPIO_Config_SetMuxChannel(uint8_t channel);

/**
  * @brief  Initialise the 6 digital-input GPIOs as inputs with pull-down.
  *         Call AFTER MX_GPIO_Init() so the port clocks are already running.
  */
void GPIO_Config_InputInit(void);

/**
  * @brief  Read a single digital input.
  * @param  index  0 = K1NO, 1 = K1NC, 2 = K2NO, 3 = K2NC, 4 = DO1, 5 = DO2
  * @retval GPIO_PIN_SET (HIGH) or GPIO_PIN_RESET (LOW)
  */
GPIO_PinState GPIO_Config_ReadInput(uint8_t index);

/**
  * @brief  Read all 6 digital inputs and pack into a single byte.
  *         Bit 0 = K1NO, bit 1 = K1NC, bit 2 = K2NO,
  *         bit 3 = K2NC, bit 4 = DO1, bit 5 = DO2.
  *         1 = HIGH, 0 = LOW.
  * @retval Packed 6-bit state.
  */
uint8_t GPIO_Config_ReadAllInputs(void);

#ifdef __cplusplus
}
#endif

#endif /* GPIO_CONFIG_H */
