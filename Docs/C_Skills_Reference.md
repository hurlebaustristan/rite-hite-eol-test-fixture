# Project Firmware And Maintenance Reference

This file replaces the old generic C-language cheat sheet with a project-specific maintenance reference for the EOL test fixture.

Use it when you need to answer questions such as:

- Which file owns a given part of the fixture behavior
- Where to change protocol messages, fault codes, or timeouts
- How the TouchGFX UI is tied into the test engines
- Which desktop-app file owns export, settings, or One Click behavior

For the full architectural picture, see [Project_Knowledge_Base.md](Project_Knowledge_Base.md).

## 1. File Ownership Map

## STM32 Core Firmware

| File Area | Responsibility |
| --- | --- |
| `Core/Inc/eol_test_protocol.h` | Shared protocol strings, timing constants, and `EOL_ERR_*` fault codes |
| `Core/Src/comm_test.c` | Communication, reset, and fault-check state machine |
| `Core/Src/di_test.c` | Digital-input stage |
| `Core/Src/ai_test.c` | Analog-input stage |
| `Core/Src/output_test.c` | Output and relay stage |
| `Core/Src/button_led_test.c` | Automatic button sequence and final LED decision wait |
| `Core/Src/eol_report.c` | Run report rows and report state machine |
| `Core/Src/export_uart.c` | `EXPORT_STATUS`, `EXPORT_RUN`, and `START_AUTO` service |
| `Core/Src/auto_test.c` | Auto-test start flag and phase tracking |
| `Core/Src/gpio_config.c` | Fixture GPIO ownership and DAC / mux support |
| `Core/Src/uart_config.c` | UART4 transport to the ESP32 |

## TouchGFX Layer

| File Area | Responsibility |
| --- | --- |
| `TouchGFX/gui/src/model/Model.cpp` | Tick loop, engine start calls, auto-test phase updates |
| `TouchGFX/gui/src/common/FrontendApplication.cpp` | Shared fault info and forced screen transition helpers |
| `TouchGFX/gui/src/*Presenter.cpp` | Per-screen activation and interaction wiring |
| `TouchGFX/gui/src/containers/*.cpp` | UI response to test progress, pass, and fail |
| `TouchGFX/gui/src/buttons_leds_screen_screen/Buttons_LEDs_ScreenView.cpp` | `LED's GOOD` and `LED's BAD` actions |

## Desktop App

| File Area | Responsibility |
| --- | --- |
| `Export_Software/main.py` | UI, One Click worker, progress rail, activity log, page layout |
| `Export_Software/serial_client.py` | Fixture serial protocol client |
| `Export_Software/esp32_uploader.py` | Rite-Hite Connect test-firmware upload path |
| `Export_Software/storage.py` | CSV writing, settings, last-successful-operation persistence |
| `Export_Software/translations.py` | English, Spanish, and Hmong strings |
| `Export_Software/theme.py` | Visual styling |

## ESP32 Test Firmware

| File Area | Responsibility |
| --- | --- |
| `ArduinoIDE/EOL_RiteHite_Final/EOL_RiteHite_Final.ino` | Protocol token handling, expander control, analog reads, relay and button acknowledgments |

## 2. Common Change Map

## If You Need To Change A Fault Code

Update:

1. `Core/Inc/eol_test_protocol.h`
2. The stage engine that raises the code
3. Any TouchGFX fault-text mapping for that stage
4. `Docs/error_code_reference.md`
5. `Docs/troubleshooting_guide.md` if operator action changes

## If You Need To Change The One Click Progress Rail

Update:

1. STM32 auto-test phase ownership in `auto_test.c`
2. Phase transitions in `Model.cpp`
3. Status parsing in `serial_client.py`
4. Progress mapping in `main.py`
5. `Docs/screen_guide.md` and `Docs/user_manual.md`

## If You Need To Change Export Behavior

Update:

1. `eol_report.c` if report content changes
2. `export_uart.c` if protocol framing changes
3. `serial_client.py` if desktop parsing changes
4. `storage.py` if CSV structure changes
5. Docs covering export format and operator workflow

## If You Need To Change The Button / LED Stage

Update:

1. `button_led_test.c`
2. `Model.cpp`
3. `Buttons_LEDs_ScreenPresenter.cpp`
4. `Buttons_LEDs_ScreenView.cpp`
5. `export_uart.c` if visual-wait semantics change
6. One Click docs if the technician action changes

## 3. Current Protocol Quick Reference

## STM32 To ESP32 Core Tokens

- `PING`
- `READY`
- `DIxTEST`
- `AIx_LOW_REQ`
- `AIx_MID_REQ`
- `AIx_HIGH_REQ`
- `K1_ON`
- `K1_OFF`
- `K2_ON`
- `K2_OFF`
- `BTN_BEGIN`
- `LED_BEGIN`
- `LED_ROTATE_250`
- `LED_STOP_ALL_ON`

## Expected ESP32 Replies

- `PONG`
- `OK`
- `DIxOK`
- `DIxFAIL`
- `OK_AI...:<volts>`
- `OK_K1_ON`, `OK_K1_OFF`, `OK_K2_ON`, `OK_K2_OFF`
- `OK_BTN_...`
- negative button acknowledgments such as `BTN_USR0_PRESS_HIGH`
- `OK_LED_BEGIN`
- `OK_LED_ROTATE_250`
- `OK_LED_STOP_ALL_ON`

## Desktop App To Fixture Tokens

- `START_AUTO`
- `EXPORT_STATUS`
- `EXPORT_RUN|<sequence>`

## Fixture To Desktop App Replies

- `AUTO_STARTED`
- `AUTO_ERROR|ALREADY_RUNNING`
- `STATUS|...`
- `EXPORT_BEGIN|...`
- `ROW|...`
- `EXPORT_END|...`
- `EXPORT_ERROR|...`

## 4. Current Test Ownership Notes

## Communication / Reset / Fault Check

- Single state machine in `comm_test.c`
- Fault family: `0x01xx`, `0x02xx`, `0x03xx`

## Digital Inputs

- 12 channels
- Expected result text: `High then Low`
- Fault family: `0x04xx`

## Analog Inputs

- 4 channels
- 3 levels per channel
- Fault family: `0x05xx`

## Outputs

- DO1 and DO2 are temporary auto-pass rows
- Relay verification is live
- Fault family: `0x06xx`

## Buttons / LEDs

- Button actuation is automatic
- LED visual decision is manual
- Fault family: `0x07xx`

## 5. TouchGFX Integration Rules

The current project uses the TouchGFX model as the coordinator between the non-blocking test engines and the screen presenters.

Practical rules:

- Start engines from presenters through model methods
- Surface per-stage progress back through the model or container callbacks
- Keep the report and fault ownership in the firmware engine, not in the container
- Use `Fault_Screen` only after fault data has been fully populated
- Keep One Click phase reporting aligned with actual engine state, not UI guesses

## 6. Desktop App Integration Rules

## Serial Client

`serial_client.py` is the source of truth for the desktop app's fixture protocol parsing.

If the export UART format changes, update this file first.

## Storage

`storage.py` owns:

- exports directory selection
- CSV column names
- settings persistence
- last successful operation persistence

## One Click

`main.py` owns:

- the One Click worker
- the app-side phase mapping
- live Activity Log updates
- UI lockouts during One Click

The desktop app must continue to treat `WAITING_VISUAL` as a real stop point that requires a technician action on the fixture.

## 7. Safe Change Checklist

Before committing a behavior change, ask:

- Does it change a protocol token?
- Does it change a timing constant?
- Does it change what the operator must do?
- Does it change what One Click displays or blocks?
- Does it change what gets written to CSV?
- Does it change a fault code or fault text?

If the answer is yes to any of these, update the matching docs in `Docs`.

## 8. Current Known Constraints

- Production firmware upload is not implemented in the desktop app.
- Digital Output 1 and Digital Output 2 are not yet real validated output tests in the fixture flow.
- The exported CSV contains human-readable test rows but not a raw fault-code column.

## 9. Recommended Validation After Changes

## For STM32 Firmware Changes

- Build the TouchGFX target
- Confirm the expected screen transitions still occur
- Confirm `Fault_Screen` text still matches the new behavior
- Confirm export still works for pass and fail

## For ESP32 Protocol Changes

- Confirm the token strings still match the STM32 expectations exactly
- Confirm negative acknowledgments still map cleanly to fault codes
- Confirm analog reply formatting still matches STM32 parsing

## For Desktop App Changes

- Run `py -m py_compile` on the app modules
- Verify manual extract
- Verify test-firmware upload
- Verify One Click start, live progress, LED wait, and final export

## 10. Documentation Rule

Do not leave docs describing old behavior when the code has moved on.

For this project, documentation is considered part of the deliverable whenever any of these change:

- fixture workflow
- operator actions
- export protocol
- One Click behavior
- active fault codes
- hardware ownership
