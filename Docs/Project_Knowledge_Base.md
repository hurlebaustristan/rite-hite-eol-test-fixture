# EOL Test Fixture — Project Knowledge Base

> **Last updated:** February 26, 2026
> This document captures all meaningful project details for the EOL (End-of-Line)
> Test Fixture, covering both the STM32U575 firmware and the ESP32-S3 companion
> firmware. Use this as the single source of truth for the AI agent and developers.

---

## Table of Contents

1. [Project Overview](#1-project-overview)
2. [Hardware Summary](#2-hardware-summary)
3. [STM32U575 Pin Map](#3-stm32u575-pin-map)
4. [ESP32-S3 Pin Map](#4-esp32-s3-pin-map)
5. [Boot / Init Sequence](#5-boot--init-sequence)
6. [GPIO Configuration](#6-gpio-configuration)
7. [DAC Configuration (PA4)](#7-dac-configuration-pa4)
8. [ADG704 Multiplexer](#8-adg704-multiplexer)
9. [UART Communication (STM32 ↔ ESP32)](#9-uart-communication-stm32--esp32)
10. [EOL Test Protocol](#10-eol-test-protocol)
11. [Communication Test State Machine](#11-communication-test-state-machine)
12. [TouchGFX Architecture](#12-touchgfx-architecture)
13. [Screen List & Navigation Flow](#13-screen-list--navigation-flow)
14. [Comm_Reset_Screen Logic](#14-comm_reset_screen-logic)
15. [Fault_Screen Logic](#15-fault_screen-logic)
16. [Test_GPIO_Screen (Manual Mode)](#16-test_gpio_screen-manual-mode)
17. [Digital Input Monitoring](#17-digital-input-monitoring)
18. [W25Q64 External Flash](#18-w25q64-external-flash)
19. [ESP32-S3 Firmware](#19-esp32-s3-firmware)
20. [Error Code Reference](#20-error-code-reference)
21. [File Index](#21-file-index)
22. [Completed & Validated Milestones](#22-completed--validated-milestones)

---

## 1. Project Overview

| Item             | Value                                                      |
|------------------|------------------------------------------------------------|
| **Project Name** | EOL_TestFixture_Final                                      |
| **Purpose**      | Automated End-of-Line production test fixture for Rite Hite products |
| **Main MCU**     | STM32U575ZI (Cortex-M33, 160 MHz)                         |
| **Companion MCU**| ESP32-S3-WROOM-1-N16R8                                     |
| **Board**        | NUCLEO-U575ZI-Q with RVA35HI display shield                |
| **Display**      | 320 × 480 pixels, Portrait, 16 bpp (RGB565)               |
| **GUI Framework**| TouchGFX 4.26.0                                            |
| **RTOS**         | FreeRTOS (X-CUBE-FREERTOS)                                 |
| **Toolchains**   | STM32CubeIDE, EWARM, MDK-ARM, GCC Makefile                |
| **ESP32 IDE**    | Arduino IDE                                                |

The STM32U575 drives the touchscreen UI and exercises all DUT (Device Under Test)
I/O. The ESP32-S3 handles supplementary communication tasks and responds to
handshake commands from the STM32.

---

## 2. Hardware Summary

### STM32U575 Clocking
- **HSI PLL**: M=2, N=40, P/Q/R=2 → **160 MHz** SYSCLK
- **FreeRTOS tick**: TIM6-based (1 ms)

### Peripherals in Use
| Peripheral | Purpose                              |
|------------|--------------------------------------|
| GPDMA1     | DMA transfers                        |
| FMC        | External memory (display)            |
| OCTOSPI1   | W25Q64 QSPI flash (assets)          |
| I2C2       | Touch controller                     |
| DMA2D      | TouchGFX hardware acceleration       |
| DCACHE1    | Data cache                           |
| CRC        | Used by TouchGFX                     |
| UART4      | ESP32-S3 communication               |
| DAC1 CH1   | Analog output on PA4 (0–3.3 V)      |

### Performance / Debug GPIOs
| Signal      | Pin  | Purpose                  |
|-------------|------|--------------------------|
| VSYNC_FREQ  | PF13 | VSync frequency output   |
| RENDER_TIME | PF14 | Render time measurement  |
| FRAME_RATE  | PF15 | Frame rate toggle        |
| MCU_ACTIVE  | PG7  | MCU activity indicator   |

---

## 3. STM32U575 Pin Map

### Digital Outputs (DI1–DI12) — Push-Pull, Init LOW

| Name  | Pin  | Port  | Row |
|-------|------|-------|-----|
| DI1   | PF5  | GPIOF | 1   |
| DI2   | PF3  | GPIOF | 1   |
| DI3   | PD2  | GPIOD | 1   |
| DI4   | PC12 | GPIOC | 1   |
| DI5   | PC11 | GPIOC | 1   |
| DI6   | PC10 | GPIOC | 2   |
| DI7   | PC9  | GPIOC | 2   |
| DI8   | PC8  | GPIOC | 2   |
| DI9   | PA3  | GPIOA | 2   |
| DI10  | PC3  | GPIOC | 2   |
| DI11  | PC1  | GPIOC | 3   |
| DI12  | PC0  | GPIOC | 3   |

### Auxiliary Panel Buttons (AUX0–AUX3) — Push-Pull, Init LOW

| Name    | Pin | Port  |
|---------|-----|-------|
| AUX0BTN | PE0 | GPIOE |
| AUX1BTN | PE3 | GPIOE |
| AUX2BTN | PF8 | GPIOF |
| AUX3BTN | PF2 | GPIOF |

### Control Buttons — Push-Pull, Init LOW

| Name   | Pin  | Port  | Index |
|--------|------|-------|-------|
| USRBTN | PB2  | GPIOB | 0     |
| MCUBTN | PB11 | GPIOB | 1     |
| FLTBTN | PA8  | GPIOA | 2     |

### ADG704 MUX Address + Enable

| Name  | Pin  | Port  | Function |
|-------|------|-------|----------|
| DACA0 | PB3  | GPIOB | A0       |
| DACA1 | PC6  | GPIOC | A1       |
| DACEN | PD11 | GPIOD | Enable   |

### DAC Analog Output

| Name    | Pin | Port  | Mode   |
|---------|-----|-------|--------|
| DAC_OUT | PA4 | GPIOA | Analog |

### Digital Inputs (Read-back from DUT, external 4.7 kΩ pull-ups)

| Index | Name | Pin   | Port  | Signal                    |
|-------|------|-------|-------|---------------------------|
| 0     | K1NO | PB13  | GPIOB | Relay 1 Normally Open     |
| 1     | K1NC | PD12  | GPIOD | Relay 1 Normally Closed   |
| 2     | K2NO | PB4   | GPIOB | Relay 2 Normally Open     |
| 3     | K2NC | PB5   | GPIOB | Relay 2 Normally Closed   |
| 4     | DO1  | PB1   | GPIOB | Digital Output 1 readback |
| 5     | DO2  | PC2   | GPIOC | Digital Output 2 readback |

### UART4 (ESP32 Communication)

| Function | Pin | Port  | AF  |
|----------|-----|-------|-----|
| UART4_TX | PA0 | GPIOA | AF8 |
| UART4_RX | PA1 | GPIOA | AF8 |

---

## 4. ESP32-S3 Pin Map

| Net Name     | Pin # | IO   | Direction         |
|--------------|-------|------|-------------------|
| CELL_TX      | 18    | IO10 | ESP32 → STM32 RX  |
| CELL_RX      | 19    | IO11 | ESP32 ← STM32 TX  |
| MCU_RST      | 3     | EN   | Reset             |
| ISO_PORTA_1  | 4     | IO4  | Isolated port A   |
| ISO_PORTA_2  | 5     | IO5  | Isolated port A   |
| ISO_PORTD_3  | 6     | IO6  | Isolated port D   |
| RS485_DRE    | 7     | IO7  | RS485 driver en   |
| IOBUS_SDA    | 8     | IO15 | I/O bus data      |
| IOBUS_SCL    | 9     | IO16 | I/O bus clock     |
| RS485_RX     | 38    | IO1  | RS485 receive     |
| RS485_TX     | 39    | IO2  | RS485 transmit    |
| DEBUG_TXD    | 37    | TXDO | Debug UART TX     |
| DEBUG_RXD    | 36    | RXDO | Debug UART RX     |
| MCU_PANIC    | 30    | IO38 | MCU panic signal  |
| ETH_ATTN     | 24    | IO47 | Ethernet attention|
| ETH_CS       | 23    | IO21 | Ethernet chip sel |
| ETH_SCK      | 22    | IO14 | Ethernet SPI clk  |
| ETH_MOSI     | 21    | IO13 | Ethernet SPI MOSI |
| ETH_MISO     | 20    | IO12 | Ethernet SPI MISO |
| ETH_RST      | —     | IO13 | Ethernet reset    |
| USB_N        | 13    | IO19 | USB D−            |
| USB_P        | 14    | IO20 | USB D+            |

---

## 5. Boot / Init Sequence

The STM32U575 `main()` executes the following in order:

```
1.  MPU_Config()
2.  HAL_Init()
3.  SystemPower_Config()
4.  SystemClock_Config()      → 160 MHz from HSI PLL
5.  MX_GPIO_Init()            → CubeMX-generated (port clocks enabled)
6.  MX_GPDMA1_Init()
7.  MX_ICACHE_Init()
8.  MX_FMC_Init()             → Display interface
9.  MX_OCTOSPI1_Init()        → W25Q64 QSPI flash
10. MX_CRC_Init()
11. MX_DMA2D_Init()
12. MX_DCACHE1_Init()
13. MX_I2C2_Init()            → Touch controller
14. MX_TouchGFX_Init()
15. MX_TouchGFX_PreOSInit()
    — USER CODE BEGIN 2 —
16. GPIO_Config_Init()        → All 12 DI + 4 AUX + 3 CTRL + MUX + DACEN as outputs
17. GPIO_Config_InputInit()   → 6 digital inputs (K1NO..DO2)
18. DAC_Config_Init()         → DAC1 CH1 on PA4 (register-level)
19. UART4_Config_Init()       → 115200 baud, 8-N-1, NVIC priority 6
    — USER CODE END 2 —
20. osKernelInitialize()
21. MX_FREERTOS_Init()
22. osKernelStart()           → Scheduler runs; TouchGFX task begins
```

---

## 6. GPIO Configuration

### API Summary (`gpio_config.h` / `gpio_config.c`)

| Function                         | Description                                    |
|----------------------------------|------------------------------------------------|
| `GPIO_Config_Init()`            | Init all outputs push-pull LOW; PA4 analog     |
| `GPIO_Config_InputInit()`       | Init 6 digital inputs (no pull, ext pull-ups)  |
| `GPIO_Config_WriteDI(idx, st)`  | Write DI1–DI12 (1-based index)                 |
| `GPIO_Config_WriteAuxBtn(i,st)` | Write AUX0–AUX3 (0-based)                      |
| `GPIO_Config_WriteCtrlBtn(i,s)` | Write USR(0)/MCU(1)/FLT(2)                     |
| `GPIO_Config_WriteDacA0(st)`    | MUX address bit A0 (PB3)                       |
| `GPIO_Config_WriteDacA1(st)`    | MUX address bit A1 (PC6)                       |
| `GPIO_Config_WriteDacEn(st)`    | MUX enable (PD11)                              |
| `GPIO_Config_SetMuxChannel(ch)` | Set ADG704 A1:A0 from channel 0–3              |
| `GPIO_Config_ReadInput(idx)`    | Read single digital input (0-based)            |
| `GPIO_Config_ReadAllInputs()`   | 6-bit packed byte (bit0=K1NO .. bit5=DO2)      |

### Counts
- `DIGITAL_OUTPUT_COUNT` = 12
- `AUX_BUTTON_COUNT` = 4
- `CONTROL_BUTTON_COUNT` = 3
- `DIGITAL_INPUT_COUNT` = 6

---

## 7. DAC Configuration (PA4)

- **Peripheral**: DAC1 Channel 1 (direct register access, not HAL DAC module)
- **Pin**: PA4 (configured as `GPIO_MODE_ANALOG` in `GPIO_Config_Init()`)
- **Init**: `DAC_Config_Init()` enables `RCC_AHB3ENR_DAC1EN`, clears `MCR_MODE1`
  (buffered, connected to pin), sets `DHR12R1 = 0`, enables `CR_EN1`
- **Slider range**: 0–330 (UI slider value maps to 0.00–3.30 V)
- **Conversion**: `dacCode = (sliderValue × 4095) / 330`
- **Clamped**: Values > 330 are clamped to 330
- **Constants**: `DAC_SLIDER_MAX = 330`, `DAC_12BIT_MAX = 4095`

---

## 8. ADG704 Multiplexer

The ADG704 is a 4-channel analog multiplexer controlled by:

| Pin   | STM32 Pin | Function      |
|-------|-----------|---------------|
| A0    | PB3       | Address bit 0 |
| A1    | PC6       | Address bit 1 |
| EN    | PD11      | Enable        |

`GPIO_Config_SetMuxChannel(channel)` sets A0 = `channel & 0x01`, A1 = `channel & 0x02`.

In the Test_GPIO_Screen (manual mode), each address/enable line can be independently
toggled via the `DACA0_PB3`, `DACA1_PC6`, and `DACEN_PD11` toggle buttons.

---

## 9. UART Communication (STM32 ↔ ESP32)

| Parameter       | Value                   |
|-----------------|-------------------------|
| Peripheral      | UART4                   |
| Baud rate       | 115200                  |
| Format          | 8-N-1, no flow control  |
| STM32 TX pin    | PA0 (AF8)               |
| STM32 RX pin    | PA1 (AF8)               |
| ESP32 TX pin    | IO10 (→ STM32 PA1 RX)   |
| ESP32 RX pin    | IO11 (← STM32 PA0 TX)   |
| Logic level     | 3.3 V (direct wiring)   |
| NVIC priority   | 6 (FreeRTOS-safe)       |
| STM32 TX mode   | Blocking (`HAL_UART_Transmit`) |
| STM32 RX mode   | Interrupt-driven (`HAL_UART_Receive_IT`) |
| ESP32 Serial    | Serial1 (HardwareSerial) |

---

## 10. EOL Test Protocol

### Messages (ASCII, newline-terminated)

| Direction          | Message   | Length |
|--------------------|-----------|--------|
| STM32 → ESP32      | `PING\n`  | 5      |
| ESP32 → STM32      | `PONG\n`  | 5      |

### Timing Constants

| Constant                | Value  | Description                          |
|-------------------------|--------|--------------------------------------|
| `EOL_PING_TIMEOUT_MS`  | 500 ms | Per-attempt receive timeout          |
| `EOL_PING_RETRY_DELAY_MS` | 3000 ms | Delay between PING attempts       |
| `EOL_PING_MAX_RETRIES` | 3      | Total attempts before failure        |
| `EOL_FAULT_DISPLAY_MS` | 3000 ms | Red text display before Fault screen |

### Test Stages

| Stage                   | Value | Description                        |
|-------------------------|-------|------------------------------------|
| `EOL_STAGE_IDLE`       | 0     | Not running                        |
| `EOL_STAGE_COMM`       | 1     | PING/PONG handshake                |
| `EOL_STAGE_MCU_RESET`  | 2     | Reset ESP32, verify reconnect      |
| `EOL_STAGE_FAULT_CHECK`| 3     | Assert/de-assert fault line        |
| `EOL_STAGE_DONE`       | 4     | All Comm_Reset tests passed        |

> **Note:** MCU_RESET and FAULT_CHECK stages are currently **stubbed** (placeholders).

---

## 11. Communication Test State Machine

Defined in `comm_test.c`, driven by `CommTest_Tick()` called from `Model::tick()` (~60 Hz).

### Sub-States

```
COMM_SUB_WAIT_BEFORE_PING  ──[3 s elapsed]──►  COMM_SUB_SEND_PING
       │                                              │
       │                                    Send "PING\n" (blocking)
       │                                    Start interrupt RX for 5 bytes
       │                                              │
       │                                              ▼
       │                                     COMM_SUB_WAIT_PONG
       │                                       │           │
       │                                  [rxDone]    [500 ms timeout]
       │                                       │           │
       │                                       ▼           ▼
       │                                    COMM_SUB_EVALUATE
       │                                   ╱      │        ╲
       │                              PONG OK  retries   exhausted
       │                                 │      left         │
       │                                 ▼       │           ▼
       │                             PASS ✓      │     FAIL ✗
       │                          (EOL_ERR_NONE) │  (0x0101 or 0x0102)
       │                                         ▼
       ◄─────────── attempt++ ──── COMM_SUB_WAIT_BEFORE_PING
```

### API

| Function                    | Description                              |
|-----------------------------|------------------------------------------|
| `CommTest_Start()`         | Reset and begin at COMM stage            |
| `CommTest_Tick()`          | Advance one step (non-blocking)          |
| `CommTest_HasResult()`     | True when a new result is available      |
| `CommTest_GetResult()`     | Consume and return `eol_test_result_t`   |
| `CommTest_CurrentStage()`  | Return current stage enum                |

### UART RX Callback

`HAL_UART_RxCpltCallback()` is implemented in `comm_test.c` — it sets `ctx.rxDone = true`
when UART4 receives the expected number of bytes.

---

## 12. TouchGFX Architecture

### MVP Pattern

```
Model  ←──tick()──  FrontendApplication::handleTickEvent()
  │
  ▼
ModelListener (virtual interface)
  │
  ├── notifyDigitalInputState(uint8_t state)
  └── notifyCommTestResult(uint8_t stage, uint16_t errorCode)
  │
  ▼
Presenter (active screen's presenter implements ModelListener)
  │
  ▼
View (screen-specific view, owns the container)
  │
  ▼
Container (widget layout + business logic in user code)
```

### Model (`Model.hpp` / `Model.cpp`)

- `tick()` — called every frame (~60 Hz):
  - Reads all 6 digital inputs → `notifyDigitalInputState()`
  - If `commTestRunning`: drives `CommTest_Tick()`, checks for result → `notifyCommTestResult()`
- `startCommTest()` — called by `Comm_Reset_ScreenPresenter::activate()`
- `commTestRunning` flag — auto-cleared when `CommTest_CurrentStage() == EOL_STAGE_DONE`

### FrontendApplication

Extends the generated `FrontendApplicationBase` with:
- `gotoFault_ScreenScreenNoTransition()` — hand-written screen transition (not in generated base)
- `static FaultInfo faultInfo` — shared struct to pass error details between screens

### FaultInfo Structure

```cpp
struct FaultInfo {
    uint16_t errorCode;     // e.g. 0x0101
    char     testName[32];  // e.g. "Communication"
    char     expected[32];  // e.g. "PONG response"
    char     actual[32];    // e.g. "No response"
};
```

---

## 13. Screen List & Navigation Flow

### All Screens (8 total)

| # | Screen Name          | Container               | Purpose                       |
|---|----------------------|-------------------------|-------------------------------|
| 1 | Start_Screen         | Start_Screen_Container  | Entry point, BEGIN buttons    |
| 2 | Test_GPIO_Screen     | Test_GPIO_Container     | Manual GPIO test              |
| 3 | Comm_Reset_Screen    | Comm_Reset_Screen       | Auto: comm + reset + fault    |
| 4 | Inputs_Screen        | Inputs_Screen           | Auto: input verification      |
| 5 | Outputs_Screen       | Outputs_Screen          | Auto: output verification     |
| 6 | Buttons_LEDs_Screen  | Buttons_LEDs_Screen     | Auto: buttons & LEDs test     |
| 7 | Fault_Screen         | Fault_Screen            | Error display (any test fail) |
| 8 | Test_Complete_Screen | Test_Complete_Screen    | All tests passed              |

### Automatic Test Flow

```
Start_Screen
  │
  ├── [Begin MANUAL] ──► Test_GPIO_Screen
  │
  └── [Begin AUTO] ──► Comm_Reset_Screen
                             │
                    ┌────────┴────────┐
                    │                 │
              [PASS: progress     [FAIL: 3 s red
               bar completes]      text display]
                    │                 │
                    ▼                 ▼
             Inputs_Screen     Fault_Screen
                    │
                    ▼
             Outputs_Screen
                    │
                    ▼
           Buttons_LEDs_Screen
                    │
                    ▼
          Test_Complete_Screen
```

All transitions use `NoTransition` (instant switch, no animation).

---

## 14. Comm_Reset_Screen Logic

### UI Layout (from TouchGFX Designer)

- **Section 1: Communication** — "Communication" label + "Connected" status text + board image
- **Section 2: MCU Reset** — "MCU Reset" label + "Button Pressed" + "Reconnected" text + image
- **Section 3: Fault Check** — "Fault Check" label + "Button Pressed" + "Reconnected" text + image
- **Progress bar** — `boxprogressbar_Comm_Reset_Buttons` (0–100, triggers screen transition at 100%)

### Behavior

1. **On screen activate**: Presenter calls `model->startCommTest()` → PING/PONG begins
2. **On COMM pass** (`EOL_ERR_NONE`):
   - "Communication" + "Connected" text → **GREEN**
   - Progress bar → 33%
3. **On COMM fail**:
   - "Communication" + "Connected" text → **RED**
   - 3-second timer starts
   - After 3 s: populate `FaultInfo` → navigate to Fault_Screen
4. **FaultInfo population on fail**:
   - `0x0101` → testName="Communication", expected="PONG response", actual="No response"
   - `0x0102` → testName="Communication", expected="PONG response", actual="Invalid reply"

### Files

- Container: `gui/containers/Comm_Reset_Screen.hpp/.cpp`
- View: `gui/comm_reset_screen_screen/Comm_Reset_ScreenView.hpp/.cpp`
- Presenter: `gui/comm_reset_screen_screen/Comm_Reset_ScreenPresenter.hpp/.cpp`

---

## 15. Fault_Screen Logic

### UI Layout

- Background image: `Function_Test_Fail.png`
- **Error_Code** — `TextAreaWithOneWildcard` (e.g. "0x0101")
- **Test_Failed** — `TextAreaWithOneWildcard` (e.g. "Communication")
- **Expected_Value** — `TextAreaWithOneWildcard` (e.g. "PONG response")
- **Actual_Value** — `TextAreaWithOneWildcard` (e.g. "No response")

### Behavior

1. Presenter `activate()` calls `view.displayFault()`
2. View forwards to `fault_Screen1.showFault()`
3. Container reads `FrontendApplication::faultInfo` and populates all 4 wildcard text areas
4. Error code formatted as hex: `snprintf(tmp, "0x%04X", errorCode)`

### Files

- Container: `gui/containers/Fault_Screen.hpp/.cpp`
- View: `gui/fault_screen_screen/Fault_ScreenView.hpp/.cpp`
- Presenter: `gui/fault_screen_screen/Fault_ScreenPresenter.hpp/.cpp`

---

## 16. Test_GPIO_Screen (Manual Mode)

The manual test screen provides direct interactive control of all outputs and
monitoring of all inputs.

### Controls (22 toggle buttons + 1 slider)

| Row | Widgets                                | GPIO Functions                          |
|-----|----------------------------------------|-----------------------------------------|
| 1   | DI1–DI5 (PF5, PF3, PD2, PC12, PC11) | `GPIO_Config_WriteDI(1–5, state)`       |
| 2   | DI6–DI10 (PC10, PC9, PC8, PA3, PC3)  | `GPIO_Config_WriteDI(6–10, state)`      |
| 3   | DI11–DI12, AUX0–AUX2                  | WriteDI + WriteAuxBtn                   |
| 4   | AUX3, USR, MCU, FLT                   | WriteAuxBtn + WriteCtrlBtn              |
| 5   | DACA0, DACEN, DACA1                   | WriteDacA0/WriteDacEn/WriteDacA1        |
| 6   | DAC_PA4 slider (0–330)                | `DAC_SetFromSlider(value)`              |

### Digital Input Display

6 text areas (K1NO, K1NC, K2NO, K2NC, DO1, DO2) update every tick:
- **GREEN** = HIGH (GPIO_PIN_SET)
- **RED** = LOW (GPIO_PIN_RESET)
- Only redraws when the packed state changes (avoids flicker)

### Files

- Container: `gui/containers/Test_GPIO_Container.hpp/.cpp`

---

## 17. Digital Input Monitoring

`GPIO_Config_ReadAllInputs()` returns a 6-bit packed byte:

| Bit | Signal | Pin  | Meaning                     |
|-----|--------|------|-----------------------------|
| 0   | K1NO   | PB13 | Relay 1 Normally Open       |
| 1   | K1NC   | PD12 | Relay 1 Normally Closed     |
| 2   | K2NO   | PB4  | Relay 2 Normally Open       |
| 3   | K2NC   | PB5  | Relay 2 Normally Closed     |
| 4   | DO1    | PB1  | Digital Output 1 readback   |
| 5   | DO2    | PC2  | Digital Output 2 readback   |

Called every `Model::tick()` cycle and broadcast via `notifyDigitalInputState()`.
External 4.7 kΩ pull-up resistors are fitted; GPIO configured with no internal pull.

---

## 18. W25Q64 External Flash

| Parameter    | Value                          |
|--------------|--------------------------------|
| Part         | W25Q64 (Winbond)               |
| Capacity     | 8 MB (0x800000)                |
| Interface    | Quad SPI via OCTOSPI1          |
| Sector size  | 4 KB                           |
| Page size    | 256 bytes                      |
| Purpose      | TouchGFX image/font assets     |

Driver in `Core/Src/w25q64.c` (707 lines) provides: reset, quad-enable, write
enable/disable, sector/32K/64K/chip erase, read, page-write, memory-mapped mode,
status register access.

---

## 19. ESP32-S3 Firmware

**File**: `ArduinoIDE/EOL_RiteHite_Final/EOL_RiteHite_Final.ino`

### Configuration

| Parameter       | Value              |
|-----------------|--------------------|
| TX Pin          | IO10               |
| RX Pin          | IO11               |
| Baud            | 115200             |
| Serial          | Serial1 (UART1)    |
| Debug Serial    | Serial0 (USB)      |

### Protocol Handling

```
loop():
  Read bytes from CellSerial until '\n' or buffer full
  If received "PING" → send "PONG\n", log to USB debug
  Else → log unknown command
  Reset buffer
```

### Future Expansion Points

The ESP32-S3 schematic shows connectivity for:
- RS485 (IO1 RX, IO2 TX, IO7 DRE)
- I/O Bus (IO15 SDA, IO16 SCL, attention lines)
- Ethernet SPI (IO12–IO14, IO21, IO47)
- Isolated ports (IO4–IO6)
- USB (IO19/IO20)
- MCU_PANIC signal (IO38)

These are not yet implemented in firmware.

---

## 20. Error Code Reference

| Code     | Stage       | Name                      | Expected        | Actual          |
|----------|-------------|---------------------------|-----------------|-----------------|
| `0x0000` | —           | No error                  | —               | —               |
| `0x0101` | COMM        | PING no response          | PONG response   | No response     |
| `0x0102` | COMM        | PING bad reply            | PONG response   | Invalid reply   |
| `0x0201` | MCU_RESET   | No reconnect after reset  | PONG response   | No response     |
| `0x0202` | MCU_RESET   | Bad reply after reset     | PONG response   | Invalid reply   |
| `0x0203` | MCU_RESET   | Reset pulse verify fail   | PB11 pulse + PONG | Pulse verify failed |
| `0x0301` | FAULT_CHECK | No reconnect after fault  | OK response     | No response     |
| `0x0302` | FAULT_CHECK | Bad reply after fault     | OK response     | Invalid reply   |
| `0x0303` | FAULT_CHECK | Fault pulse verify fail   | PA8 pulse + OK  | Pulse verify failed |
| `0x0401` | DIGITAL_IN  | DI no response            | High then Low   | No response     |
| `0x0402` | DIGITAL_IN  | DI fail token             | High then Low   | Invalid transition |
| `0x0403` | DIGITAL_IN  | DI invalid reply          | High then Low   | Invalid reply   |

Error codes are grouped by stage: `0x01xx` = Communication, `0x02xx` = MCU Reset,
`0x03xx` = Fault Check, `0x04xx` = Digital Inputs.

---

## 21. File Index

### Core Firmware (STM32U575)

| File                         | Purpose                                |
|------------------------------|----------------------------------------|
| `Core/Inc/gpio_config.h`    | Pin definitions, GPIO/DAC API          |
| `Core/Src/gpio_config.c`    | GPIO init, output writes, input reads  |
| `Core/Inc/uart_config.h`    | UART4 pin/baud definitions             |
| `Core/Src/uart_config.c`    | UART4 init, TX/RX wrappers             |
| `Core/Inc/eol_test_protocol.h` | Protocol messages, error codes, stages |
| `Core/Inc/comm_test.h`      | Comm test engine API                   |
| `Core/Src/comm_test.c`      | Non-blocking Comm_Reset_Screen test flow (COMM/MCU/Fault) |
| `Core/Inc/di_test.h`        | Inputs_Screen digital input test engine API |
| `Core/Src/di_test.c`        | Non-blocking DI1-DI12 test sequence (`DIxTEST`) |
| `Core/Inc/w25q64.h`         | W25Q64 flash driver header             |
| `Core/Src/w25q64.c`         | W25Q64 QSPI flash driver               |
| `Core/Src/main.c`           | System init, boot sequence             |

### TouchGFX UI

| File                                        | Purpose                                  |
|---------------------------------------------|------------------------------------------|
| `gui/model/Model.hpp/.cpp`                 | Data model, tick driver, comm test start  |
| `gui/model/ModelListener.hpp`              | Virtual callback interface                |
| `gui/common/FrontendApplication.hpp/.cpp`  | FaultInfo struct, fault screen transition |
| `gui/containers/Test_GPIO_Container.cpp`   | Manual test: 22 toggles + slider + inputs |
| `gui/containers/Comm_Reset_Screen.hpp/.cpp`| Auto test: comm result handling, fault nav|
| `gui/containers/Inputs_Screen.hpp/.cpp`    | Auto test: DI1-DI12 progress + fault handling |
| `gui/containers/Fault_Screen.hpp/.cpp`     | Error display with wildcard text areas    |
| `gui/comm_reset_screen_screen/*`           | View + Presenter for Comm_Reset          |
| `gui/inputs_screen_screen/*`               | View + Presenter for Inputs_Screen       |
| `gui/fault_screen_screen/*`                | View + Presenter for Fault_Screen        |

### ESP32-S3 Firmware

| File                                                    | Purpose                    |
|---------------------------------------------------------|----------------------------|
| `ArduinoIDE/EOL_RiteHite_Final/EOL_RiteHite_Final.ino` | Fixture protocol, DI sensing, PING/READY, DIxTEST watcher |

---

## 22. Completed & Validated Milestones

### Comm_Reset_Screen (Completed and working)

- `COMM` stage implemented and validated:
  - STM32 sends `PING`
  - ESP32 replies `PONG`
  - UI turns `Communication` / `Connected` green
  - Progress advances to `33`
- `MCU Reset` stage implemented and validated:
  - STM32 pulses `PB11` HIGH then LOW (1 s pulse)
  - UI turns `Button Pressed` green immediately after pulse release
  - 1 s recovery delay
  - Reconnect handshake uses `PING` / `PONG`
  - On success, `MCU Reset` section turns green and progress advances to `66`
- `Fault Check` stage implemented and validated:
  - STM32 pulses `PA8` HIGH then LOW (1 s pulse)
  - UI turns `Fault Check -> Button Pressed` green immediately after pulse release
  - 1 s recovery delay
  - Handshake uses `READY` / `OK`
  - On success, full `Fault Check` row turns green and progress reaches `100`
- Completion UX:
  - Comm_Reset_Screen holds for ~1 second after full green completion
  - Then navigates to `Inputs_Screen`
- Fault handling:
  - `Fault_Screen` fields (`Error_Code`, `Test_Failed`, `Expected_Value`, `Actual_Value`) populate correctly
  - Added/validated `0x01xx`, `0x02xx`, and `0x03xx` fault mappings

### Inputs_Screen Digital Inputs 1-12 (Completed and working)

- Simple token-based DI test flow implemented (current repo architecture):
  - STM32 sends `DIxTEST`
  - STM32 drives DIx HIGH for 1 s, then LOW for 1 s using `GPIO_Config_WriteDI()`
  - ESP32 watches the matching DI channel transition and replies `DIxOK` or `DIxFAIL`
- `Inputs_Screen` UI behavior implemented:
  - DI test starts automatically when `Inputs_Screen` activates
  - `Digital_Input_1` .. `Digital_Input_12` turn green one-by-one on pass
  - Progress bar fills to `75%` after all 12 digital inputs pass
  - Remaining `25%` is reserved for future analog input automation
- Digital input failure handling implemented:
  - Immediate navigation to `Fault_Screen`
  - Fault data populated with:
    - `Test Failed = Digital Input X`
    - `Expected = High then Low`
    - `Actual = No response / Invalid transition / Invalid reply`
  - Uses `0x0401`, `0x0402`, `0x0403`
- TouchGFX generated progress callback guard updated:
  - `Inputs_Screen` does not auto-transition to `Outputs_Screen` while progress is below `100`
  - Prevents unwanted screen change at `75%` after DI tests

### Notes for future changes

- `TouchGFX/generated/.../Comm_Reset_ScreenBase.cpp` and `TouchGFX/generated/.../Inputs_ScreenBase.cpp` contain hand-applied progress callback guards.
- If TouchGFX code is regenerated, re-verify these guards are still present.

*This document is auto-maintained. Update when new test stages, pins, or screens are added.*
