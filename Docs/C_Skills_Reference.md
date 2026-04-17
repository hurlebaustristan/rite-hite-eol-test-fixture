# C Programming Skills Reference

A reference guide for AI-assisted C coding in embedded/STM32 projects.

---

## Table of Contents

1. [Data Types](#data-types)
2. [Operators](#operators)
3. [Control Flow](#control-flow)
4. [Functions](#functions)
5. [Pointers](#pointers)
6. [Arrays and Strings](#arrays-and-strings)
7. [Structs and Unions](#structs-and-unions)
8. [Enumerations](#enumerations)
9. [Preprocessor Directives](#preprocessor-directives)
10. [Memory Management](#memory-management)
11. [Bit Manipulation](#bit-manipulation)
12. [Embedded C Patterns](#embedded-c-patterns)
13. [Volatile and Const Qualifiers](#volatile-and-const-qualifiers)
14. [Callbacks and Function Pointers](#callbacks-and-function-pointers)
15. [Interrupt Service Routines (ISR)](#interrupt-service-routines)
16. [Common STM32 HAL Patterns](#common-stm32-hal-patterns)
17. [FreeRTOS Patterns](#freertos-patterns)
18. [Error Handling](#error-handling)
19. [Coding Best Practices](#coding-best-practices)

---

## Data Types

### Standard Types

```c
char        c = 'A';         // 1 byte, signed or unsigned
int         i = 100;         // typically 4 bytes
short       s = 300;         // 2 bytes
long        l = 100000L;     // 4 bytes
long long   ll = 1000000LL;  // 8 bytes
float       f = 3.14f;       // 4 bytes
double      d = 3.14159;     // 8 bytes
```

### Fixed-Width Types (stdint.h) — preferred in embedded

```c
#include <stdint.h>

uint8_t   byte_val  = 0xFF;         // 0 to 255
int8_t    signed8   = -128;         // -128 to 127
uint16_t  word_val  = 0xFFFF;       // 0 to 65535
int16_t   signed16  = -32768;
uint32_t  dword_val = 0xFFFFFFFF;   // 0 to 4294967295
int32_t   signed32  = -2147483648;
uint64_t  qword_val = 0xFFFFFFFFFFFFFFFFULL;
```

### Boolean

```c
#include <stdbool.h>

bool flag = true;
bool done = false;
```

---

## Operators

### Arithmetic

```c
int a = 10 + 3;   // 13
int b = 10 - 3;   // 7
int c = 10 * 3;   // 30
int d = 10 / 3;   // 3   (integer division)
int e = 10 % 3;   // 1   (modulo)
```

### Comparison

```c
==   // equal to
!=   // not equal to
>    // greater than
<    // less than
>=   // greater than or equal
<=   // less than or equal
```

### Logical

```c
&&   // AND
||   // OR
!    // NOT
```

### Bitwise

```c
&    // AND
|    // OR
^    // XOR
~    // NOT (complement)
<<   // left shift
>>   // right shift
```

### Assignment Shortcuts

```c
x += 5;   // x = x + 5
x -= 5;   // x = x - 5
x *= 2;   // x = x * 2
x /= 2;   // x = x / 2
x %= 3;   // x = x % 3
x &= 0xF0;
x |= 0x0F;
x ^= 0xFF;
x <<= 1;
x >>= 1;
```

### Ternary Operator

```c
int max = (a > b) ? a : b;
```

---

## Control Flow

### if / else if / else

```c
if (x > 10) {
    // ...
} else if (x == 5) {
    // ...
} else {
    // ...
}
```

### switch / case

```c
switch (state) {
    case STATE_IDLE:
        // ...
        break;
    case STATE_RUN:
        // ...
        break;
    default:
        // ...
        break;
}
```

### for loop

```c
for (int i = 0; i < 10; i++) {
    // ...
}
```

### while loop

```c
while (condition) {
    // ...
}
```

### do-while loop

```c
do {
    // executes at least once
} while (condition);
```

### break and continue

```c
for (int i = 0; i < 10; i++) {
    if (i == 5) break;      // exit loop
    if (i % 2 == 0) continue; // skip to next iteration
}
```

---

## Functions

### Declaration and Definition

```c
// Declaration (prototype)
int add(int a, int b);

// Definition
int add(int a, int b) {
    return a + b;
}
```

### void Function

```c
void reset_counter(void) {
    counter = 0;
}
```

### Passing by Pointer (modify caller's variable)

```c
void increment(int *value) {
    (*value)++;
}

int x = 5;
increment(&x);  // x is now 6
```

### Returning Multiple Values via Pointers

```c
void get_min_max(int *arr, int len, int *min, int *max) {
    *min = arr[0];
    *max = arr[0];
    for (int i = 1; i < len; i++) {
        if (arr[i] < *min) *min = arr[i];
        if (arr[i] > *max) *max = arr[i];
    }
}
```

### Inline Functions

```c
static inline uint32_t clamp(uint32_t val, uint32_t lo, uint32_t hi) {
    if (val < lo) return lo;
    if (val > hi) return hi;
    return val;
}
```

---

## Pointers

### Basics

```c
int  x   = 10;
int *ptr = &x;       // ptr holds address of x
int  val = *ptr;     // dereference: val = 10
*ptr = 20;           // x is now 20
```

### Pointer Arithmetic

```c
uint8_t buf[4] = {0x01, 0x02, 0x03, 0x04};
uint8_t *p = buf;
p++;          // points to buf[1]
p += 2;       // points to buf[3]
```

### Pointer to const vs const pointer

```c
const int *p1 = &x;    // can't modify *p1, but p1 can change
int * const p2 = &x;   // can modify *p2, but p2 can't change
const int * const p3 = &x; // neither can change
```

### NULL Pointer Check

```c
if (ptr != NULL) {
    *ptr = 5;
}
```

---

## Arrays and Strings

### Arrays

```c
int arr[5] = {1, 2, 3, 4, 5};
int matrix[3][3] = {{1,2,3},{4,5,6},{7,8,9}};

// Array passed to function (decays to pointer)
void process(uint8_t *data, uint16_t len);
```

### Strings (null-terminated char arrays)

```c
#include <string.h>

char name[32] = "TestFixture";

strlen(name);               // length (not counting '\0')
strcpy(dest, src);          // copy string
strncpy(dest, src, n);      // safe copy
strcat(dest, src);          // concatenate
strcmp(s1, s2);             // 0 if equal
snprintf(buf, size, fmt);   // safe formatted string
```

---

## Structs and Unions

### Struct

```c
typedef struct {
    uint8_t  id;
    uint16_t voltage_mv;
    float    temperature;
    bool     pass;
} TestResult_t;

TestResult_t result = {0};
result.id = 1;
result.voltage_mv = 3300;

// Pointer to struct
TestResult_t *pResult = &result;
pResult->voltage_mv = 3300;   // arrow operator
```

### Union (shares memory)

```c
typedef union {
    uint32_t word;
    uint8_t  bytes[4];
} Word32_t;

Word32_t data;
data.word = 0xDEADBEEF;
// data.bytes[0] = 0xEF (little-endian)
```

---

## Enumerations

```c
typedef enum {
    STATE_IDLE = 0,
    STATE_TESTING,
    STATE_PASS,
    STATE_FAIL,
    STATE_COUNT   // useful for array sizing
} TestState_t;

TestState_t currentState = STATE_IDLE;
```

---

## Preprocessor Directives

### Macros

```c
#define MAX_SAMPLES     256
#define LED_PIN         GPIO_PIN_5
#define PIN_HIGH(port, pin)   HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET)
#define PIN_LOW(port, pin)    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET)
```

### Macro with do-while for multi-statement safety

```c
#define LOG_ERROR(msg) do { \
    uart_print("[ERR] ");   \
    uart_print(msg);        \
    uart_print("\r\n");     \
} while(0)
```

### Conditional Compilation

```c
#ifdef DEBUG
    #define DEBUG_PRINT(x) uart_print(x)
#else
    #define DEBUG_PRINT(x)
#endif

#ifndef MY_HEADER_H
#define MY_HEADER_H
// header content
#endif
```

### include guards

```c
#ifndef APP_CONFIG_H
#define APP_CONFIG_H

// declarations...

#endif /* APP_CONFIG_H */
```

---

## Memory Management

### Stack vs Heap

In embedded systems, **prefer stack (local variables) and static allocation** over dynamic heap allocation.

```c
// Stack allocation (preferred in embedded)
uint8_t buffer[128];

// Static allocation (persists across calls)
static uint32_t call_count = 0;

// Heap (use with extreme caution in embedded)
#include <stdlib.h>
uint8_t *buf = (uint8_t *)malloc(128);
if (buf != NULL) {
    // use buf
    free(buf);
}
```

### memset / memcpy / memmove

```c
#include <string.h>

memset(buffer, 0, sizeof(buffer));          // zero out
memcpy(dest, src, num_bytes);               // copy (no overlap)
memmove(dest, src, num_bytes);              // copy (safe overlap)
int result = memcmp(buf1, buf2, num_bytes); // compare
```

---

## Bit Manipulation

```c
uint32_t reg = 0;

// Set bit n
reg |= (1UL << n);

// Clear bit n
reg &= ~(1UL << n);

// Toggle bit n
reg ^= (1UL << n);

// Test bit n
if (reg & (1UL << n)) { /* bit is set */ }

// Extract bits [hi:lo]
uint32_t field = (reg >> lo) & ((1UL << (hi - lo + 1)) - 1);

// Create bitmask for bits [hi:lo]
#define BITMASK(hi, lo)  (((1UL << ((hi)-(lo)+1))-1) << (lo))
```

---

## Embedded C Patterns

### Register Access (direct)

```c
// Using CMSIS macros (preferred with STM32 HAL)
GPIOA->ODR |=  GPIO_ODR_OD5;    // set PA5
GPIOA->ODR &= ~GPIO_ODR_OD5;    // clear PA5
GPIOA->ODR ^=  GPIO_ODR_OD5;    // toggle PA5
```

### Circular Buffer

```c
#define BUF_SIZE 64

typedef struct {
    uint8_t  data[BUF_SIZE];
    uint16_t head;
    uint16_t tail;
    uint16_t count;
} CircBuf_t;

void circbuf_push(CircBuf_t *cb, uint8_t byte) {
    if (cb->count < BUF_SIZE) {
        cb->data[cb->head] = byte;
        cb->head = (cb->head + 1) % BUF_SIZE;
        cb->count++;
    }
}

bool circbuf_pop(CircBuf_t *cb, uint8_t *byte) {
    if (cb->count == 0) return false;
    *byte = cb->data[cb->tail];
    cb->tail = (cb->tail + 1) % BUF_SIZE;
    cb->count--;
    return true;
}
```

### State Machine

```c
typedef void (*StateHandler_t)(void);

void state_idle(void)    { /* ... */ }
void state_testing(void) { /* ... */ }
void state_pass(void)    { /* ... */ }
void state_fail(void)    { /* ... */ }

StateHandler_t stateTable[] = {
    state_idle,
    state_testing,
    state_pass,
    state_fail
};

// In main loop:
stateTable[currentState]();
```

---

## Volatile and Const Qualifiers

```c
// volatile: tells compiler not to optimize away reads/writes
// Used for hardware registers, ISR-shared variables
volatile uint32_t tick_count = 0;
volatile bool     data_ready = false;

// const: value cannot be modified
const uint8_t DEVICE_ADDR = 0x48;

// Constant data in flash (embedded)
const uint8_t lookup_table[256] = { /* ... */ };

// Pointer to volatile hardware register
volatile uint32_t * const GPIOA_ODR = (volatile uint32_t *)0x48000014;
```

---

## Callbacks and Function Pointers

```c
// Function pointer type
typedef void (*Callback_t)(uint8_t event);

// Register and call
Callback_t on_complete = NULL;

void register_callback(Callback_t cb) {
    on_complete = cb;
}

void trigger_event(uint8_t ev) {
    if (on_complete != NULL) {
        on_complete(ev);
    }
}

// Usage
void my_handler(uint8_t event) {
    // handle event
}

register_callback(my_handler);
```

---

## Interrupt Service Routines

```c
// ISR in STM32 (defined in stm32u5xx_it.c)
void TIM6_DAC_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim6);
}

// HAL callback (called from HAL IRQ handler)
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM6) {
        tick_count++;  // volatile variable
    }
}

// Rules for ISRs:
// - Keep them SHORT and FAST
// - No blocking calls (no HAL_Delay)
// - Use volatile for shared variables
// - Set flags; process in main loop or RTOS task
```

---

## Common STM32 HAL Patterns

### GPIO

```c
// Read pin
GPIO_PinState state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);

// Write pin
HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

// Toggle
HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
```

### UART

```c
// Transmit (blocking)
HAL_UART_Transmit(&huart2, (uint8_t *)buf, len, HAL_MAX_DELAY);

// Receive (blocking)
HAL_UART_Receive(&huart2, (uint8_t *)buf, len, timeout_ms);

// Transmit (interrupt-driven)
HAL_UART_Transmit_IT(&huart2, (uint8_t *)buf, len);

// Callback
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) {
        tx_done = true;
    }
}
```

### SPI

```c
// Transmit and Receive (full-duplex)
HAL_SPI_TransmitReceive(&hspi1, tx_buf, rx_buf, len, timeout_ms);

// Transmit only
HAL_SPI_Transmit(&hspi1, tx_buf, len, timeout_ms);

// Chip select control (manual NSS)
HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET); // CS LOW
HAL_SPI_Transmit(&hspi1, tx_buf, len, HAL_MAX_DELAY);
HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);   // CS HIGH
```

### I2C

```c
// Write to device register
HAL_I2C_Mem_Write(&hi2c1, dev_addr << 1, reg_addr, I2C_MEMADD_SIZE_8BIT, data, len, timeout);

// Read from device register
HAL_I2C_Mem_Read(&hi2c1, dev_addr << 1, reg_addr, I2C_MEMADD_SIZE_8BIT, data, len, timeout);
```

### Delay

```c
HAL_Delay(100);  // 100 ms blocking delay
// In RTOS context use: osDelay(100);
```

---

## FreeRTOS Patterns

```c
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// Create task
xTaskCreate(vMyTask, "MyTask", 256, NULL, osPriorityNormal, &taskHandle);

// Task function template
void vMyTask(void *argument) {
    for (;;) {
        // do work
        osDelay(10);  // yield for 10 ms
    }
}

// Queue send/receive
QueueHandle_t xQueue = xQueueCreate(10, sizeof(uint32_t));

uint32_t value = 42;
xQueueSend(xQueue, &value, portMAX_DELAY);

uint32_t received;
xQueueReceive(xQueue, &received, portMAX_DELAY);

// Mutex
SemaphoreHandle_t xMutex = xSemaphoreCreateMutex();

xSemaphoreTake(xMutex, portMAX_DELAY);
// critical section
xSemaphoreGive(xMutex);

// Semaphore from ISR
BaseType_t xHigherPriorityTaskWoken = pdFALSE;
xSemaphoreGiveFromISR(xSemaphore, &xHigherPriorityTaskWoken);
portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
```

---

## Error Handling

```c
// HAL status checking
HAL_StatusTypeDef status;
status = HAL_SPI_Transmit(&hspi1, buf, len, 100);
if (status != HAL_OK) {
    // handle error
    Error_Handler();
}

// Return code pattern
typedef enum {
    APP_OK    = 0,
    APP_ERROR = 1,
    APP_TIMEOUT = 2
} AppStatus_t;

AppStatus_t read_sensor(float *out) {
    if (out == NULL) return APP_ERROR;
    // ...
    return APP_OK;
}

// Assert macro (debug only)
#ifdef DEBUG
    #define ASSERT(expr) do { if (!(expr)) { Error_Handler(); } } while(0)
#else
    #define ASSERT(expr)
#endif
```

---

## Coding Best Practices

### Naming Conventions

```
Variables:    camelCase       e.g. sampleCount, isReady
Constants:    UPPER_SNAKE     e.g. MAX_RETRIES, DEFAULT_TIMEOUT
Types:        PascalCase_t    e.g. TestResult_t, AppState_t
Functions:    lower_snake     e.g. read_adc(), send_packet()
Macros:       UPPER_SNAKE     e.g. BIT_SET(), PIN_HIGH()
File-private: static prefix   e.g. static void helper_func(void)
```

### Header File Template

```c
#ifndef MODULE_NAME_H
#define MODULE_NAME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* Public defines */
#define MODULE_VERSION   1

/* Public types */
typedef struct { /* ... */ } ModuleData_t;

/* Public function prototypes */
void     MODULE_Init(void);
bool     MODULE_Process(ModuleData_t *data);
uint32_t MODULE_GetStatus(void);

#ifdef __cplusplus
}
#endif

#endif /* MODULE_NAME_H */
```

### Source File Template

```c
/* Includes */
#include "module_name.h"

/* Private defines */
#define INTERNAL_CONST   100

/* Private types */

/* Private variables */
static uint32_t s_counter = 0;

/* Private function prototypes */
static void helper(void);

/* Public functions */
void MODULE_Init(void) {
    s_counter = 0;
}

/* Private functions */
static void helper(void) {
    /* ... */
}
```

### General Rules

- Always initialize variables before use.
- Use `sizeof()` instead of hardcoded sizes.
- Use `static` for file-scope variables and private functions.
- Avoid magic numbers — use named constants or enums.
- Check return values of HAL functions.
- Never use `HAL_Delay()` inside ISRs or RTOS tasks (use `osDelay()`).
- Keep functions small and single-purpose.
- Comment non-obvious logic, not what is obvious from the code.
- Prefer `uint32_t` over `unsigned int` in embedded code for clarity.
- Use `#pragma once` or include guards in every header.

---

*Reference maintained for AI-assisted development of the EOL TestFixture project.*
