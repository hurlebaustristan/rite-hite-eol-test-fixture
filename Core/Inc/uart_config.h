/**
  ******************************************************************************
 * @file    uart_config.h
 * @brief   UART helper configuration for UART4 (ESP32) and USART1 (ST-LINK VCP).
  *
 *          UART4 pins:   PA0 → UART4_TX   (AF8)
 *                        PA1 → UART4_RX   (AF8)
 *
 *          USART1 pins:  PA9  → USART1_TX (AF7, ST-LINK VCP RX)
 *                        PA10 → USART1_RX (AF7, ST-LINK VCP TX)
  *
  *          Config: 115 200 baud, 8-N-1, no flow control.
  *          Mode:   Interrupt-driven (NVIC enabled).
  *
  *          The ESP32 and STM32U575 are both 3.3 V logic —
  *          direct wiring with a shared GND is sufficient.
  ******************************************************************************
  */

#ifndef UART_CONFIG_H
#define UART_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32u5xx_hal.h"
#include <stdbool.h>

/* ======================================================================== */
/*  Pin Definitions — UART4  (PA0 TX, PA1 RX, Alternate Function 8)         */
/* ======================================================================== */
#define UART4_TX_Pin            GPIO_PIN_0
#define UART4_TX_GPIO_Port      GPIOA

#define UART4_RX_Pin            GPIO_PIN_1
#define UART4_RX_GPIO_Port      GPIOA

#define UART4_GPIO_AF           GPIO_AF8_UART4

/* ======================================================================== */
/*  Pin Definitions — USART1 (PA9 TX, PA10 RX, Alternate Function 7)        */
/* ======================================================================== */
#define USART1_TX_Pin           GPIO_PIN_9
#define USART1_TX_GPIO_Port     GPIOA

#define USART1_RX_Pin           GPIO_PIN_10
#define USART1_RX_GPIO_Port     GPIOA

#define USART1_GPIO_AF          GPIO_AF7_USART1

/* ======================================================================== */
/*  Communication Parameters                                                */
/* ======================================================================== */
#define UART4_BAUDRATE          115200U
#define USART1_BAUDRATE         115200U

/* ======================================================================== */
/*  Handle — accessible from ISR and application code                       */
/* ======================================================================== */
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart1;

/* ======================================================================== */
/*  Public API                                                              */
/* ======================================================================== */

/**
  * @brief  Initialise UART4 for ESP32 communication (115 200, 8-N-1).
  *         Configures PA0/PA1 as AF8, enables the UART4 peripheral clock,
  *         and sets up the NVIC interrupt.
  *         Call AFTER MX_GPIO_Init() so the GPIOA clock is already running.
  */
void UART4_Config_Init(void);
void USART1_Config_Init(void);

/**
  * @brief  Blocking transmit over UART4.
  * @param  pData    Pointer to the data buffer.
  * @param  size     Number of bytes to send.
  * @param  timeout  Timeout in milliseconds (HAL_MAX_DELAY for infinite).
  * @retval HAL status (HAL_OK on success).
  */
HAL_StatusTypeDef UART4_Transmit(const uint8_t *pData, uint16_t size,
                                 uint32_t timeout);
HAL_StatusTypeDef USART1_Transmit(const uint8_t *pData, uint16_t size,
                                  uint32_t timeout);

/**
  * @brief  Start an interrupt-driven receive on UART4.
  * @param  pData  Pointer to the receive buffer.
  * @param  size   Number of bytes to receive before the callback fires.
  * @retval HAL status (HAL_OK on success).
  */
HAL_StatusTypeDef UART4_Receive_IT(uint8_t *pData, uint16_t size);

/**
  * @brief  Read one byte from the UART4 RX ring buffer (if available).
  * @param  outByte Pointer to destination byte.
  * @retval true if a byte was read, false if buffer was empty.
  */
bool UART4_ReadByte(uint8_t *outByte);

/**
  * @brief  Clear all pending bytes from the UART4 RX ring buffer.
  */
void UART4_ClearRxBuffer(void);
bool USART1_ReadByte(uint8_t *outByte);
void USART1_ClearRxBuffer(void);

#ifdef __cplusplus
}
#endif

#endif /* UART_CONFIG_H */
