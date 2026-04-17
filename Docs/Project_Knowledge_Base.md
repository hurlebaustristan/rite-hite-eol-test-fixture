# EOL Test Fixture Project Knowledge Base

This document is the engineering reference for the current Rite-Hite EOL test fixture implementation.

It is intended to answer:

- How the fixture actually works today
- Which module owns each part of the workflow
- What protocol messages and timings are in use
- How the desktop app, STM32 firmware, TouchGFX UI, and ESP32 test firmware fit together

This file is intentionally based on current code behavior, not planned future behavior.

## 1. Project Summary

| Item | Value |
| --- | --- |
| Project | `EOL_TestFixture_Final` |
| Main controller | STM32U575 |
| UI | TouchGFX on STM32 |
| Companion target | ESP32-S3 Rite-Hite Connect test firmware |
| Desktop app | PySide6 / PyInstaller Windows app |
| Primary desktop workflows | Manual export, test-firmware upload, One Click |

## 2. System Responsibilities

### STM32 Fixture

The STM32 is the owner of the test flow. It:

- Runs the TouchGFX touchscreen UI
- Drives fixture GPIO and analog outputs
- Executes all test engines
- Builds the pass/fail report
- Exposes the export UART service
- Accepts `START_AUTO` from the desktop app

### ESP32 Test Firmware

The ESP32 responds to the fixture protocol. It:

- Replies to comm and fault-check tokens
- Measures digital input and analog responses
- Handles relay and output acknowledgments
- Reports button-state and LED-control acknowledgments

### Desktop App

The desktop app is a station tool. It:

- Uploads the Rite-Hite Connect test firmware
- Queries the fixture export service
- Exports report rows to CSV
- Tracks One Click progress live
- Shows the Activity Log and file history

## 3. High-Level Workflows

## Fixture-Only Workflow

1. Operator presses `Begin AUTO` on the fixture.
2. Fixture runs the automatic flow.
3. Technician makes the final LED decision on the touchscreen.
4. Desktop app exports the completed run from `Device Tools`.

## One Click Workflow

1. Desktop app uploads the Rite-Hite Connect test firmware.
2. Desktop app waits 5 seconds for boot.
3. Desktop app sends `START_AUTO` to the fixture.
4. Fixture runs the same real automatic flow used by the touchscreen.
5. Desktop app polls the fixture status every 0.5 seconds.
6. Technician still makes the final LED decision on the touchscreen.
7. Desktop app exports the completed run automatically.

Important detail:

- One Click is not a separate fake test path.
- It reuses the real fixture engines and TouchGFX screen flow.

## 4. Touchscreen Navigation Flow

The fixture screens are:

- `Start_Screen`
- `Test_GPIO_Screen`
- `Comm_Reset_Screen`
- `Inputs_Screen`
- `Outputs_Screen`
- `Buttons_LEDs_Screen`
- `Fault_Screen`
- `Test_Complete_Screen`

### Normal Production Flow

`Start_Screen -> Comm_Reset_Screen -> Inputs_Screen -> Outputs_Screen -> Buttons_LEDs_Screen -> Test_Complete_Screen`

Any test failure redirects to:

`Fault_Screen`

### Manual Service Flow

`Start_Screen -> Test_GPIO_Screen`

## 5. Auto-Test Ownership

The auto-test helper only manages start intent and phase tracking.

### Auto-Test State

The STM32 tracks:

- whether an auto test start was requested
- whether auto mode is active
- the current auto phase

### Auto Phases Reported To The Desktop App

- `COMMS`
- `DIGITAL_INPUTS`
- `ANALOG_INPUTS`
- `DIGITAL_OUTPUTS`
- `RELAY_OUTPUTS`
- `BUTTONS`
- `LEDS`

The phase is surfaced through the export UART status response so the desktop app can drive the One Click progress rail from the real fixture state.

## 6. Stage-By-Stage Firmware Behavior

## Communication Test

Purpose:

- Verify basic serial communication
- Verify the reset path
- Verify the fault-check path

### Messages

- STM32 sends `PING\n`
- ESP32 replies `PONG`
- STM32 sends `READY\n`
- ESP32 replies `OK`

### Timing

- Receive timeout per attempt: 3000 ms
- Delay between `PING` attempts: 3000 ms
- Maximum retries: 3
- MCU reset pulse: 1000 ms
- MCU reset recovery: 1000 ms
- Fault-check pulse: 1000 ms
- Fault-check recovery: 1000 ms
- Red-fault display before `Fault_Screen`: 3000 ms

### Fault Codes

- `0x0101`: no response to `PING`
- `0x0102`: bad `PING` reply
- `0x0201`: no reconnect after reset
- `0x0202`: bad reply after reset
- `0x0203`: reset pulse verify failure
- `0x0301`: no `OK` after fault pulse
- `0x0302`: bad `OK` reply after fault pulse
- `0x0303`: fault pulse verify failure

### UI Tie-In

- `Comm_Reset_ScreenPresenter::activate()` starts the comm test through the model.
- On success, the flow advances to `Inputs_Screen`.
- On failure, fault data is populated and `Fault_Screen` is shown.

## Digital Input Test

Purpose:

- Validate 12 digital channels by driving a high-to-low transition and checking the ESP32 response.

### Command Pattern

For each channel 1 to 12:

- STM32 sends `DIxTEST`
- STM32 drives the corresponding output HIGH, then LOW
- ESP32 replies with `DIxOK` or `DIxFAIL`

### Current Behavior

- Each channel allows up to 2 attempts.
- The expected logical result recorded in the report is `High then Low`.

### Fault Codes

- `0x0401`: no reply
- `0x0402`: fail token returned
- `0x0403`: invalid reply

## Analog Input Test

Purpose:

- Validate 4 analog channels at three levels each.

### Channels

- Channels 1 and 2: current loop
- Channels 3 and 4: voltage

### DAC Slider Setpoints

| Channel Type | Low | Mid | High |
| --- | --- | --- | --- |
| Current loop channels | 62 | 196 | 330 |
| Voltage channels | 0 | 165 | 330 |

### Engineering Targets

| Channel Type | Low | Mid | High |
| --- | --- | --- | --- |
| Current loop channels | 4 mA | 12 mA | 20 mA |
| Voltage channels | 0 V | 5 V | 10 V |

### Tolerances

- Current loop: plus or minus 2 mA
- Voltage: plus or minus 1 V

### Request Pattern

The STM32 requests individual points with:

- `AI1_LOW_REQ`
- `AI1_MID_REQ`
- `AI1_HIGH_REQ`

and equivalent tokens for channels 2 through 4.

The ESP32 replies with:

- `OK_AI<channel>_<level>:<volts>`

The analog reply value is interpreted as the measured ADC-side voltage, then converted to engineering units on the STM32 side.

### Conversion Logic

- Current loop channels use `adcVolts * (1000 / 150)`
- Voltage channels use `adcVolts * 5.1`

### Fault Codes

- `0x0501`: no analog reply
- `0x0502`: invalid analog reply
- `0x0503`: measured value out of tolerance

## Output Test

Purpose:

- Cover digital outputs and relays.

### Current Revision Behavior

Digital Output 1 and Digital Output 2 are temporary auto-pass report rows. The fixture records them as:

- `Digital Output 1` / `Temporary skip` / `Auto-pass (no DO board)`
- `Digital Output 2` / `Temporary skip` / `Auto-pass (no DO board)`

### Live Relay Validation

The relay portion is active and drives:

- `K1_ON`
- `K1_OFF`
- `K2_ON`
- `K2_OFF`

Expected acknowledgments:

- `OK_K1_ON`
- `OK_K1_OFF`
- `OK_K2_ON`
- `OK_K2_OFF`

Readback validation uses:

- K1 NO and NC
- K2 NO and NC

### Fault Codes

- `0x0601`: no relay acknowledgment
- `0x0602`: invalid relay acknowledgment
- `0x0603`: relay state mismatch

## Buttons And LEDs Test

Purpose:

- Validate button-state detection and LED control.

### Important Current Behavior

The current firmware automatically drives the button sequence from the fixture side. The technician does not manually press the hardware buttons in the normal automatic run.

The only required human action is the final LED decision:

- `LED's GOOD`
- `LED's BAD`

### Button Order

- `USR_0`
- `PNL_0`
- `PNL_1`
- `PNL_2`
- `PNL_3`

### Core Tokens

- `BTN_BEGIN`
- `LED_BEGIN`
- `LED_ROTATE_250`
- `LED_STOP_ALL_ON`
- button press and release tokens such as `BTN_USR0_PRESS`
- positive acknowledgments such as `OK_BTN_USR0_PRESS`
- negative acknowledgments such as `BTN_USR0_PRESS_HIGH`

### Timing

- Init timeout: 3000 ms
- Button poll timeout: 100 ms
- Press action timeout: 1500 ms
- Release action timeout: 2000 ms
- Assist pre-press delay: 100 ms
- Press settle: 50 ms
- Release settle: 750 ms

### Visual-Wait Behavior

When the button sequence completes:

- The model changes the auto phase to `LEDS`
- The export status service reports `WAITING_VISUAL|LEDS`
- The desktop app logs a technician prompt
- The fixture waits until `ButtonLedTest_SubmitVisualResult()` is called from the touchscreen `GOOD` or `BAD` button

### Fault Codes

- `0x0701`: no acknowledgment
- `0x0702`: invalid acknowledgment
- `0x0703`: operator rejected LED visual check
- `0x0704`: timeout
- `0x0705`: state mismatch

## 7. TouchGFX Model Integration

The TouchGFX model drives the test engines and screen transitions.

Key behavior:

- `Model::tick()` services `ExportUart_Tick()`
- `Model::tick()` consumes pending auto-test start requests
- `triggerAutoTestScreenTransition()` jumps directly to `Comm_Reset_Screen`
- Each presenter starts its own stage through the model
- Output-test relay progress is used to switch the auto phase from `DIGITAL_OUTPUTS` to `RELAY_OUTPUTS`
- Button/LED progress is used to surface the final visual-wait state

## 8. Report Model

The fixture stores a run report internally.

### Report Characteristics

- Maximum rows: 48
- Row fields:
  - pass/fail
  - test name
  - expected
  - actual

### Report States

- `EMPTY`
- `RUNNING`
- `READY`
- `EXPORTED`

### Run Completion

- Pass finalization increments the run sequence and marks the report `READY`
- Fail finalization also produces a completed exportable report
- Successful export marks the run `EXPORTED`

Important detail:

- Failed runs remain exportable after the fixture reaches `Fault_Screen`

## 9. Export UART Service

The export service runs on the STM32 and is used by the desktop app.

### Supported Commands

- `EXPORT_STATUS`
- `EXPORT_RUN|<sequence>`
- `START_AUTO`

### Common Replies

- `STATUS|EMPTY`
- `STATUS|RUNNING|<phase>`
- `STATUS|WAITING_VISUAL|LEDS`
- `STATUS|READY|PASS|<runSequence>|<rowCount>`
- `STATUS|EXPORTED|FAIL|<runSequence>|<rowCount>`
- `AUTO_STARTED`
- `AUTO_ERROR|ALREADY_RUNNING`
- `EXPORT_BEGIN|...`
- `ROW|...`
- `EXPORT_END|...`

### Export Errors

- `EXPORT_ERROR|NOT_READY`
- `EXPORT_ERROR|BAD_SEQ`
- `EXPORT_ERROR|ALREADY_EXPORTED`

## 10. Desktop App Behavior

The desktop app has three primary pages:

- `File Explorer`
- `Device Tools`
- `One Click Test`

It also includes:

- Activity Log drawer
- Settings dialog
- Last successful operation summary

### Persistent Local Files

The app stores:

- `exports` folder for CSV output
- `settings.json`
- `last_successful_operation.json`

### App Settings

- `dark_mode`
- `language`
- `font_size`
- `confirm_before_extract`
- `auto_refresh_on_tab_switch`
- `reduced_motion`
- optional custom `exports_dir`

### CSV Output

CSV columns are:

- `Date and Time`
- `Test name`
- `Expected Result`
- `Actual result`
- `Pass/Fail`

### One Click Worker

The One Click worker currently uses:

- Poll interval: 0.5 seconds
- Poll timeout: 300 seconds
- Rite-Hite Connect boot wait: 5 seconds

The worker flow is:

1. Upload test firmware
2. Wait for boot
3. Start auto test
4. Poll fixture status
5. Wait for LED decision
6. Export report
7. Write CSV

### One Click UI Behavior

- The progress rail follows the real fixture phase.
- Current step is red.
- Completed steps turn green.
- Manual extract and manual test-firmware upload are disabled during a One Click run.
- Status and prompt detail is written to the Activity Log.

## 11. Desktop App Limitations

- Production firmware upload is present in the UI but not implemented. The button shows a `Coming Soon` dialog.
- One Click depends on both the fixture serial path and the programmer path being available.
- The LED decision remains a fixture-side manual action by design.

## 12. Code-Defined Hardware Map

This section only lists signals that are directly defined in the current codebase.

## STM32 Fixture GPIO Map

### Digital Outputs

| Signal | Pin |
| --- | --- |
| DI1 | PF5 |
| DI2 | PF3 |
| DI3 | PD2 |
| DI4 | PC12 |
| DI5 | PC11 |
| DI6 | PC10 |
| DI7 | PC9 |
| DI8 | PC8 |
| DI9 | PA3 |
| DI10 | PC3 |
| DI11 | PC1 |
| DI12 | PC0 |

### Auxiliary Buttons

| Signal | Pin |
| --- | --- |
| AUX0BTN | PE0 |
| AUX1BTN | PE3 |
| AUX2BTN | PF8 |
| AUX3BTN | PF2 |

### Control Buttons

| Signal | Pin |
| --- | --- |
| USRBTN | PB2 |
| MCUBTN | PB11 |
| FLTBTN | PA8 |

### DAC / Mux Control

| Signal | Pin |
| --- | --- |
| DACA0 | PB3 |
| DACA1 | PC6 |
| DACEN | PD11 |
| DAC_OUT | PA4 |

### Digital Readback Inputs

| Signal | Pin |
| --- | --- |
| K1NO | PB13 |
| K1NC | PD12 |
| K2NO | PB4 |
| K2NC | PB5 |
| DO1 | PB1 |
| DO2 | PC2 |

## ESP32 Test Firmware Pins Used In Code

| Signal | Pin |
| --- | --- |
| Serial1 RX from fixture | GPIO11 |
| Serial1 TX to fixture | GPIO10 |
| I2C SDA | GPIO15 |
| I2C SCL | GPIO16 |
| I/O bus reset | GPIO9 |
| Q3 interrupt | GPIO17 |
| Q4 interrupt | GPIO18 |
| ADS alert | GPIO8 |

## 13. Known Revision Constraints

- The outputs screen still contains temporary DO1 and DO2 auto-pass placeholders.
- The final production-firmware upload workflow is not implemented in the desktop app.
- The exported CSV contains human-readable report rows but not a dedicated raw fault-code column.

## 14. Documentation Maintenance Rule

If any of the following change, update this file in the same change set:

- protocol tokens
- fault codes
- stage timings
- GPIO ownership
- TouchGFX screen flow
- One Click behavior
- desktop app export behavior
