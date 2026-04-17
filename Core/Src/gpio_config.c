/**
  ******************************************************************************
  * @file    gpio_config.c
  * @brief   EOL Test Fixture GPIO initialisation and helper functions.
  *
  *          ALL toggle-button GPIOs are OUTPUTS (push-pull, start LOW).
  *          Pin assignments come from gpio_config.h — edit that file
  *          to remap pins.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "gpio_config.h"

/* ======================================================================== */
/*  Private: look-up tables for indexed access                              */
/* ======================================================================== */

/** Digital-output pin/port pairs (index 0 = DI1) */
static const uint16_t s_diPin[DIGITAL_OUTPUT_COUNT] = {
    DI1_Pin,  DI2_Pin,  DI3_Pin,  DI4_Pin,  DI5_Pin,  DI6_Pin,
    DI7_Pin,  DI8_Pin,  DI9_Pin,  DI10_Pin, DI11_Pin, DI12_Pin
};

static GPIO_TypeDef * const s_diPort[DIGITAL_OUTPUT_COUNT] = {
    DI1_GPIO_Port,  DI2_GPIO_Port,  DI3_GPIO_Port,
    DI4_GPIO_Port,  DI5_GPIO_Port,  DI6_GPIO_Port,
    DI7_GPIO_Port,  DI8_GPIO_Port,  DI9_GPIO_Port,
    DI10_GPIO_Port, DI11_GPIO_Port, DI12_GPIO_Port
};

/** Aux-button pin/port pairs (index 0 = AUX0) */
static const uint16_t s_auxPin[AUX_BUTTON_COUNT] = {
    AUX0BTN_Pin, AUX1BTN_Pin, AUX2BTN_Pin, AUX3BTN_Pin
};

static GPIO_TypeDef * const s_auxPort[AUX_BUTTON_COUNT] = {
    AUX0BTN_GPIO_Port, AUX1BTN_GPIO_Port,
    AUX2BTN_GPIO_Port, AUX3BTN_GPIO_Port
};

/** Control-button pin/port pairs (index 0 = USR) */
static const uint16_t s_ctrlPin[CONTROL_BUTTON_COUNT] = {
    USRBTN_Pin, MCUBTN_Pin, FLTBTN_Pin
};

static GPIO_TypeDef * const s_ctrlPort[CONTROL_BUTTON_COUNT] = {
    USRBTN_GPIO_Port, MCUBTN_GPIO_Port, FLTBTN_GPIO_Port
};

/** Digital-input pin/port pairs (index 0 = K1NO) */
static const uint16_t s_dinPin[DIGITAL_INPUT_COUNT] = {
    K1NO_Pin, K1NC_Pin, K2NO_Pin, K2NC_Pin, DO1_Pin, DO2_Pin
};

static GPIO_TypeDef * const s_dinPort[DIGITAL_INPUT_COUNT] = {
    K1NO_GPIO_Port, K1NC_GPIO_Port, K2NO_GPIO_Port,
    K2NC_GPIO_Port, DO1_GPIO_Port,  DO2_GPIO_Port
};

/* ======================================================================== */
/*  GPIO_Config_Init  — all pins as push-pull outputs, initial state LOW    */
/* ======================================================================== */
void GPIO_Config_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /*
     * NOTE: All port clocks (GPIOA-G) are already enabled by MX_GPIO_Init().
     */

    /* Common settings for all output pins */
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    /* -------------------------------------------------------------------- */
    /*  Digital Outputs  DI1 – DI12                                         */
    /* -------------------------------------------------------------------- */

    /* -- GPIOF outputs: DI1 (PF5), DI2 (PF3) -- */
    HAL_GPIO_WritePin(GPIOF, DI1_Pin | DI2_Pin, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = DI1_Pin | DI2_Pin;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    /* -- GPIOD output: DI3 (PD2) -- */
    HAL_GPIO_WritePin(DI3_GPIO_Port, DI3_Pin, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = DI3_Pin;
    HAL_GPIO_Init(DI3_GPIO_Port, &GPIO_InitStruct);

    /* -- GPIOC outputs: DI4 (PC12), DI5 (PC11), DI6 (PC10),
                          DI7 (PC9), DI8 (PC8), DI10 (PC3),
                          DI11 (PC1), DI12 (PC0) -- */
    HAL_GPIO_WritePin(GPIOC, DI4_Pin | DI5_Pin | DI6_Pin
                            | DI7_Pin | DI8_Pin | DI10_Pin
                            | DI11_Pin | DI12_Pin, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = DI4_Pin | DI5_Pin | DI6_Pin
                        | DI7_Pin | DI8_Pin | DI10_Pin
                        | DI11_Pin | DI12_Pin;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* -- GPIOA output: DI9 (PA3) -- */
    HAL_GPIO_WritePin(DI9_GPIO_Port, DI9_Pin, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = DI9_Pin;
    HAL_GPIO_Init(DI9_GPIO_Port, &GPIO_InitStruct);

    /* -------------------------------------------------------------------- */
    /*  Aux Panel Buttons  AUX0 – AUX3  (outputs)                          */
    /* -------------------------------------------------------------------- */

    /* -- GPIOE output: AUX0 (PE0) -- */
    HAL_GPIO_WritePin(AUX0BTN_GPIO_Port, AUX0BTN_Pin, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = AUX0BTN_Pin;
    HAL_GPIO_Init(AUX0BTN_GPIO_Port, &GPIO_InitStruct);

    /* -- GPIOE output: AUX1 (PE3) -- */
    HAL_GPIO_WritePin(AUX1BTN_GPIO_Port, AUX1BTN_Pin, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = AUX1BTN_Pin;
    HAL_GPIO_Init(AUX1BTN_GPIO_Port, &GPIO_InitStruct);

    /* -- GPIOF outputs: AUX2 (PF8), AUX3 (PF2) -- */
    HAL_GPIO_WritePin(GPIOF, AUX2BTN_Pin | AUX3BTN_Pin, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = AUX2BTN_Pin | AUX3BTN_Pin;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    /* -------------------------------------------------------------------- */
    /*  Control Buttons  USR, MCU-RST, FAULT  (outputs)                     */
    /* -------------------------------------------------------------------- */

    /* -- GPIOB outputs: USR (PB2), MCU-RST (PB11) -- */
    HAL_GPIO_WritePin(GPIOB, USRBTN_Pin | MCUBTN_Pin, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = USRBTN_Pin | MCUBTN_Pin;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* -- GPIOA output: FAULT (PA8) -- */
    HAL_GPIO_WritePin(FLTBTN_GPIO_Port, FLTBTN_Pin, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = FLTBTN_Pin;
    HAL_GPIO_Init(FLTBTN_GPIO_Port, &GPIO_InitStruct);

    /* -------------------------------------------------------------------- */
    /*  MUX Address Selects  DACA0, DACA1  (outputs)                        */
    /* -------------------------------------------------------------------- */
    HAL_GPIO_WritePin(DACA0_GPIO_Port, DACA0_Pin, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = DACA0_Pin;
    HAL_GPIO_Init(DACA0_GPIO_Port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(DACA1_GPIO_Port, DACA1_Pin, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = DACA1_Pin;
    HAL_GPIO_Init(DACA1_GPIO_Port, &GPIO_InitStruct);

    /* -- GPIOD output: DACEN (PD11) -- */
    HAL_GPIO_WritePin(DACEN_GPIO_Port, DACEN_Pin, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = DACEN_Pin;
    HAL_GPIO_Init(DACEN_GPIO_Port, &GPIO_InitStruct);

    /* -------------------------------------------------------------------- */
    /*  DAC Analog Output  PA4  (analog mode — feeds the DAC peripheral)    */
    /* -------------------------------------------------------------------- */
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Pin  = DAC_OUT_Pin;
    HAL_GPIO_Init(DAC_OUT_GPIO_Port, &GPIO_InitStruct);
}

/* ======================================================================== */
/*  GPIO_Config_WriteDI  — write one digital output (1-based index)         */
/* ======================================================================== */
void GPIO_Config_WriteDI(uint8_t index, GPIO_PinState state)
{
    if (index < 1 || index > DIGITAL_OUTPUT_COUNT) {
        return;
    }
    HAL_GPIO_WritePin(s_diPort[index - 1], s_diPin[index - 1], state);
}

/* ======================================================================== */
/*  GPIO_Config_WriteAuxBtn  — write one aux button output (0-based)        */
/* ======================================================================== */
void GPIO_Config_WriteAuxBtn(uint8_t index, GPIO_PinState state)
{
    if (index >= AUX_BUTTON_COUNT) {
        return;
    }
    HAL_GPIO_WritePin(s_auxPort[index], s_auxPin[index], state);
}

/* ======================================================================== */
/*  GPIO_Config_WriteCtrlBtn  — write USR/MCU/FLT output (0-based)          */
/* ======================================================================== */
void GPIO_Config_WriteCtrlBtn(uint8_t index, GPIO_PinState state)
{
    if (index >= CONTROL_BUTTON_COUNT) {
        return;
    }
    HAL_GPIO_WritePin(s_ctrlPort[index], s_ctrlPin[index], state);
}

/* ======================================================================== */
/*  DACA0 / DACA1  — individual write                                       */
/* ======================================================================== */
void GPIO_Config_WriteDacA0(GPIO_PinState state)
{
    HAL_GPIO_WritePin(DACA0_GPIO_Port, DACA0_Pin, state);
}

void GPIO_Config_WriteDacA1(GPIO_PinState state)
{
    HAL_GPIO_WritePin(DACA1_GPIO_Port, DACA1_Pin, state);
}

void GPIO_Config_WriteDacEn(GPIO_PinState state)
{
    HAL_GPIO_WritePin(DACEN_GPIO_Port, DACEN_Pin, state);
}

/* ======================================================================== */
/*  GPIO_Config_SetMuxChannel  — set ADG704 address lines (A1:A0)           */
/* ======================================================================== */
void GPIO_Config_SetMuxChannel(uint8_t channel)
{
    HAL_GPIO_WritePin(DACA0_GPIO_Port, DACA0_Pin,
                      (channel & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DACA1_GPIO_Port, DACA1_Pin,
                      (channel & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/* ======================================================================== */
/*  DAC_Config_Init  — enable DAC1 CH1 on PA4 (direct register access)      */
/* ======================================================================== */
void DAC_Config_Init(void)
{
    RCC->AHB3ENR |= RCC_AHB3ENR_DAC1EN;
    __DSB();

    DAC1->MCR &= ~DAC_MCR_MODE1;   /* buffered, connected to pin */
    DAC1->DHR12R1 = 0;             /* initial 0 V */
    DAC1->CR |= DAC_CR_EN1;        /* enable channel 1 */
}

/* ======================================================================== */
/*  DAC_SetFromSlider  — convert 0-330 slider value to 12-bit DAC code      */
/* ======================================================================== */
void DAC_SetFromSlider(uint16_t sliderValue)
{
    if (sliderValue > DAC_SLIDER_MAX) {
        sliderValue = DAC_SLIDER_MAX;
    }
    uint32_t dacCode = ((uint32_t)sliderValue * DAC_12BIT_MAX) / DAC_SLIDER_MAX;
    DAC1->DHR12R1 = dacCode;
}

/* ======================================================================== */
/*  GPIO_Config_InputInit  — configure 6 digital inputs (pull-down)         */
/* ======================================================================== */
void GPIO_Config_InputInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /*
     * NOTE: Port clocks (GPIOB, GPIOC, GPIOD) are already enabled by
     *       MX_GPIO_Init().
     */

    GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;        /* External 4.7 kΩ pull-ups fitted */
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    /* -- GPIOB inputs: K1NO (PB13), K2NO (PB4), K2NC (PB5), DO1 (PB1) -- */
    GPIO_InitStruct.Pin = K1NO_Pin | K2NO_Pin | K2NC_Pin | DO1_Pin;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* -- GPIOD input: K1NC (PD12) -- */
    GPIO_InitStruct.Pin = K1NC_Pin;
    HAL_GPIO_Init(K1NC_GPIO_Port, &GPIO_InitStruct);

    /* -- GPIOC input: DO2 (PC2) -- */
    GPIO_InitStruct.Pin = DO2_Pin;
    HAL_GPIO_Init(DO2_GPIO_Port, &GPIO_InitStruct);
}

/* ======================================================================== */
/*  GPIO_Config_ReadInput  — read one digital input (0-based index)         */
/* ======================================================================== */
GPIO_PinState GPIO_Config_ReadInput(uint8_t index)
{
    if (index >= DIGITAL_INPUT_COUNT) {
        return GPIO_PIN_RESET;
    }
    return HAL_GPIO_ReadPin(s_dinPort[index], s_dinPin[index]);
}

/* ======================================================================== */
/*  GPIO_Config_ReadAllInputs  — pack 6 inputs into one byte                */
/*  Bit 0 = K1NO,  bit 1 = K1NC,  bit 2 = K2NO,                           */
/*  bit 3 = K2NC,  bit 4 = DO1,   bit 5 = DO2                             */
/* ======================================================================== */
uint8_t GPIO_Config_ReadAllInputs(void)
{
    uint8_t state = 0;
    for (uint8_t i = 0; i < DIGITAL_INPUT_COUNT; i++) {
        if (HAL_GPIO_ReadPin(s_dinPort[i], s_dinPin[i]) == GPIO_PIN_SET) {
            state |= (1U << i);
        }
    }
    return state;
}
