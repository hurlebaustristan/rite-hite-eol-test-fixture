/**
  ******************************************************************************
  * @file    uart_config.c
 * @brief   UART4 / USART1 initialisation and helpers.
  *
 *          UART4 configuration:   115 200 baud, 8-N-1, no flow control.
 *          UART4 pins:            PA0 → TX,  PA1 → RX  (AF8).
 *
 *          USART1 configuration:  115 200 baud, 8-N-1, no flow control.
 *          USART1 pins:           PA9 → TX,  PA10 → RX (AF7, ST-LINK VCP).
  *
  *          All clocks, GPIO alternate-function setup, and NVIC
  *          configuration are handled in this file so the full UART4
  *          setup is visible in one place.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "uart_config.h"
#include "stm32u5xx_hal_uart_ex.h"

/* ======================================================================== */
/*  Handle instance                                                         */
/* ======================================================================== */
UART_HandleTypeDef huart4;
UART_HandleTypeDef huart1;

/* ======================================================================== */
/*  UART4 RX ring buffer (continuous 1-byte interrupt reception)            */
/* ======================================================================== */
static volatile uint8_t uart4RxByte = 0;
static volatile uint8_t usart1RxByte = 0;

#define UART4_RX_RING_SIZE 256u
static volatile uint8_t  uart4RxRing[UART4_RX_RING_SIZE];
static volatile uint16_t uart4RxHead = 0;
static volatile uint16_t uart4RxTail = 0;

#define USART1_RX_RING_SIZE 256u
static volatile uint8_t  usart1RxRing[USART1_RX_RING_SIZE];
static volatile uint16_t usart1RxHead = 0;
static volatile uint16_t usart1RxTail = 0;

static void UART4_RxPushByte(uint8_t byte)
{
    const uint16_t nextHead = (uint16_t)((uart4RxHead + 1u) % UART4_RX_RING_SIZE);

    if (nextHead == uart4RxTail)
    {
        /* Overflow: drop oldest byte so latest traffic is retained. */
        uart4RxTail = (uint16_t)((uart4RxTail + 1u) % UART4_RX_RING_SIZE);
    }

    uart4RxRing[uart4RxHead] = byte;
    uart4RxHead = nextHead;
}

static void USART1_RxPushByte(uint8_t byte)
{
    const uint16_t nextHead = (uint16_t)((usart1RxHead + 1u) % USART1_RX_RING_SIZE);

    if (nextHead == usart1RxTail)
    {
        usart1RxTail = (uint16_t)((usart1RxTail + 1u) % USART1_RX_RING_SIZE);
    }

    usart1RxRing[usart1RxHead] = byte;
    usart1RxHead = nextHead;
}

/* ======================================================================== */
/*  UART4_Config_Init                                                       */
/* ======================================================================== */
/**
  * @brief  Initialise UART4 for ESP32 communication (115 200, 8-N-1).
  *
  *         Steps performed:
  *           1. Enable GPIOA clock (may already be on — safe to call again).
  *           2. Configure PA0 (TX) and PA1 (RX) as AF8 push-pull, high-speed.
  *           3. Enable UART4 peripheral clock.
  *           4. Populate the UART handle and call HAL_UART_Init().
  *           5. Enable the UART4 NVIC interrupt (priority 6 — FreeRTOS-safe).
  */
void UART4_Config_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* ---- 1. GPIOA clock ---- */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* ---- 2. Configure PA0 (TX) and PA1 (RX) as Alternate Function 8 ---- */

    /* -- GPIOA output: UART4_TX (PA0) -- */
    GPIO_InitStruct.Pin       = UART4_TX_Pin;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = UART4_GPIO_AF;
    HAL_GPIO_Init(UART4_TX_GPIO_Port, &GPIO_InitStruct);

    /* -- GPIOA input: UART4_RX (PA1) -- */
    GPIO_InitStruct.Pin       = UART4_RX_Pin;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;         /* pull-up avoids noise */
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = UART4_GPIO_AF;
    HAL_GPIO_Init(UART4_RX_GPIO_Port, &GPIO_InitStruct);

    /* ---- 3. UART4 peripheral clock ---- */
    __HAL_RCC_UART4_CLK_ENABLE();

    /* ---- 4. UART4 parameter configuration (8-N-1, 115 200 baud) ---- */
    huart4.Instance                    = UART4;
    huart4.Init.BaudRate               = UART4_BAUDRATE;
    huart4.Init.WordLength             = UART_WORDLENGTH_8B;
    huart4.Init.StopBits               = UART_STOPBITS_1;
    huart4.Init.Parity                 = UART_PARITY_NONE;
    huart4.Init.Mode                   = UART_MODE_TX_RX;
    huart4.Init.HwFlowCtl             = UART_HWCONTROL_NONE;
    huart4.Init.OverSampling           = UART_OVERSAMPLING_16;
    huart4.Init.OneBitSampling         = UART_ONE_BIT_SAMPLE_DISABLE;
    huart4.Init.ClockPrescaler         = UART_PRESCALER_DIV1;
    huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if (HAL_UART_Init(&huart4) != HAL_OK)
    {
        /* Initialisation failed — stay here for debugger inspection */
        while (1) { }
    }

    /* ---- 5. Enable UART4 NVIC interrupt ---- */
    /*
     * Priority 6  (numerically > configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY = 5)
     * so FreeRTOS API calls from the HAL callback are safe.
     */
    if (HAL_UARTEx_SetTxFifoThreshold(&huart4, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        while (1) { }
    }
    if (HAL_UARTEx_SetRxFifoThreshold(&huart4, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        while (1) { }
    }
    if (HAL_UARTEx_EnableFifoMode(&huart4) != HAL_OK)
    {
        while (1) { }
    }

    HAL_NVIC_SetPriority(UART4_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(UART4_IRQn);

    UART4_ClearRxBuffer();
    if (HAL_UART_Receive_IT(&huart4, (uint8_t *)&uart4RxByte, 1) != HAL_OK)
    {
        while (1) { }
    }
}

void USART1_Config_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        while (1) { }
    }

    GPIO_InitStruct.Pin       = USART1_TX_Pin;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = USART1_GPIO_AF;
    HAL_GPIO_Init(USART1_TX_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin       = USART1_RX_Pin;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = USART1_GPIO_AF;
    HAL_GPIO_Init(USART1_RX_GPIO_Port, &GPIO_InitStruct);

    __HAL_RCC_USART1_CLK_ENABLE();

    huart1.Instance                    = USART1;
    huart1.Init.BaudRate               = USART1_BAUDRATE;
    huart1.Init.WordLength             = UART_WORDLENGTH_8B;
    huart1.Init.StopBits               = UART_STOPBITS_1;
    huart1.Init.Parity                 = UART_PARITY_NONE;
    huart1.Init.Mode                   = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl              = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling           = UART_OVERSAMPLING_16;
    huart1.Init.OneBitSampling         = UART_ONE_BIT_SAMPLE_DISABLE;
    huart1.Init.ClockPrescaler         = UART_PRESCALER_DIV1;
    huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        while (1) { }
    }

    if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        while (1) { }
    }
    if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        while (1) { }
    }
    if (HAL_UARTEx_EnableFifoMode(&huart1) != HAL_OK)
    {
        while (1) { }
    }

    HAL_NVIC_SetPriority(USART1_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);

    USART1_ClearRxBuffer();
    if (HAL_UART_Receive_IT(&huart1, (uint8_t *)&usart1RxByte, 1) != HAL_OK)
    {
        while (1) { }
    }
}

/* ======================================================================== */
/*  UART4_Transmit  — blocking send wrapper                                 */
/* ======================================================================== */
/**
  * @brief  Blocking transmit over UART4.
  * @param  pData    Pointer to the data buffer.
  * @param  size     Number of bytes to send.
  * @param  timeout  Timeout in milliseconds (HAL_MAX_DELAY for infinite).
  * @retval HAL status (HAL_OK on success).
  */
HAL_StatusTypeDef UART4_Transmit(const uint8_t *pData, uint16_t size,
                                 uint32_t timeout)
{
    return HAL_UART_Transmit(&huart4, pData, size, timeout);
}

HAL_StatusTypeDef USART1_Transmit(const uint8_t *pData, uint16_t size,
                                  uint32_t timeout)
{
    return HAL_UART_Transmit(&huart1, pData, size, timeout);
}

/* ======================================================================== */
/*  UART4_Receive_IT  — interrupt-driven receive wrapper                    */
/* ======================================================================== */
/**
  * @brief  Start an interrupt-driven receive on UART4.
  * @param  pData  Pointer to the receive buffer.
  * @param  size   Number of bytes to receive before the callback fires.
  * @retval HAL status (HAL_OK on success).
  */
HAL_StatusTypeDef UART4_Receive_IT(uint8_t *pData, uint16_t size)
{
    return HAL_UART_Receive_IT(&huart4, pData, size);
}

bool UART4_ReadByte(uint8_t *outByte)
{
    bool hasData = false;

    if (outByte == 0)
    {
        return false;
    }

    __disable_irq();
    if (uart4RxHead != uart4RxTail)
    {
        *outByte = uart4RxRing[uart4RxTail];
        uart4RxTail = (uint16_t)((uart4RxTail + 1u) % UART4_RX_RING_SIZE);
        hasData = true;
    }
    __enable_irq();

    return hasData;
}

bool USART1_ReadByte(uint8_t *outByte)
{
    bool hasData = false;

    if (outByte == 0)
    {
        return false;
    }

    __disable_irq();
    if (usart1RxHead != usart1RxTail)
    {
        *outByte = usart1RxRing[usart1RxTail];
        usart1RxTail = (uint16_t)((usart1RxTail + 1u) % USART1_RX_RING_SIZE);
        hasData = true;
    }
    __enable_irq();

    return hasData;
}

void UART4_ClearRxBuffer(void)
{
    __disable_irq();
    uart4RxHead = 0;
    uart4RxTail = 0;
    __enable_irq();
}

void USART1_ClearRxBuffer(void)
{
    __disable_irq();
    usart1RxHead = 0;
    usart1RxTail = 0;
    __enable_irq();
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART4)
    {
        UART4_RxPushByte(uart4RxByte);
        (void)HAL_UART_Receive_IT(&huart4, (uint8_t *)&uart4RxByte, 1);
    }
    else if (huart->Instance == USART1)
    {
        USART1_RxPushByte(usart1RxByte);
        (void)HAL_UART_Receive_IT(&huart1, (uint8_t *)&usart1RxByte, 1);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART4)
    {
        __HAL_UART_CLEAR_FLAG(&huart4, UART_CLEAR_PEF | UART_CLEAR_FEF |
                                       UART_CLEAR_NEF | UART_CLEAR_OREF);
        __HAL_UART_FLUSH_DRREGISTER(&huart4);
        huart4.ErrorCode = HAL_UART_ERROR_NONE;
        (void)HAL_UART_Receive_IT(&huart4, (uint8_t *)&uart4RxByte, 1);
    }
    else if (huart->Instance == USART1)
    {
        __HAL_UART_CLEAR_FLAG(&huart1, UART_CLEAR_PEF | UART_CLEAR_FEF |
                                       UART_CLEAR_NEF | UART_CLEAR_OREF);
        __HAL_UART_FLUSH_DRREGISTER(&huart1);
        huart1.ErrorCode = HAL_UART_ERROR_NONE;
        (void)HAL_UART_Receive_IT(&huart1, (uint8_t *)&usart1RxByte, 1);
    }
}
