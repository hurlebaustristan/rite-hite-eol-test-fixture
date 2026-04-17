# End-of-Line Error Code Reference

## Purpose

This document is the consolidated reference for the implemented end-of-line (EOL) error codes used by the test fixture software in this project. It explains what each code means, where it is generated, how it ties into the test flow, and what the operator sees on the fault screen.

This file is intentionally written using standard markdown headings, tables, and plain paragraphs so it can be converted cleanly into a Word or PDF document.

## Related Documents

Use this reference together with the following operator-facing documents:

- [User Manual](user_manual.md)
- [Screen Guide](screen_guide.md)
- [Troubleshooting Guide](troubleshooting_guide.md)
- [Preflight Checklist](preflight_checklist.md)
- [Test Procedure](test_procedure.md)

## Scope

This reference applies to the current fixture software revision in `EOL_TestFixture_Final`.

The canonical source of truth for the numeric EOL error codes is:

- `Core/Inc/eol_test_protocol.h`

This document was cross-checked against:

- `Docs/test_procedure.md`
- `Core/Src/comm_test.c`
- `Core/Src/di_test.c`
- `Core/Src/ai_test.c`
- `Core/Src/output_test.c`
- `Core/Src/button_led_test.c`
- `TouchGFX/gui/include/gui/common/FrontendApplication.hpp`
- `TouchGFX/gui/src/containers/Comm_Reset_Screen.cpp`
- `TouchGFX/gui/src/containers/Inputs_Screen.cpp`
- `TouchGFX/gui/src/containers/Outputs_Screen.cpp`
- `TouchGFX/gui/src/containers/Buttons_LEDs_Screen.cpp`
- `TouchGFX/gui/src/containers/Fault_Screen.cpp`
- `Core/Inc/eol_report.h`
- `Core/Src/export_uart.c`

## How The Error Codes Are Structured

The fixture uses one shared error-code namespace so each failure can be shown as a unique hexadecimal code on `Fault_Screen`.

| Code Range | Stage / Meaning |
| --- | --- |
| `0x0000` | No error |
| `0x01xx` | Communication stage |
| `0x02xx` | MCU reset stage |
| `0x03xx` | Fault check stage |
| `0x04xx` | Digital inputs stage |
| `0x05xx` | Analog inputs stage |
| `0x06xx` | Outputs stage |
| `0x07xx` | Buttons / LEDs stage |

In the current implementation, the upper byte identifies the test family. The lower byte identifies the failure type inside that family. In most families, the lower byte loosely follows this pattern:

- `..01`: no response or no reconnect
- `..02`: invalid or unexpected reply
- `..03`: stage-specific verification failure
- `..04`: timeout in the manual buttons / LEDs stage

## How A Code Ties Into The Fixture

The error code path through the fixture is:

1. A core test engine detects a failure and sets an `EOL_ERR_*` code.
2. The active TouchGFX screen maps that failure into shared `FaultInfo` data:
   `errorCode`, `testName`, `expected`, and `actual`.
3. `Fault_Screen` formats the code as hex, for example `0x0101`, and shows the failed step plus the expected and actual values.
4. The run report stores human-readable rows such as test name, expected, actual, and pass/fail status.
5. The export UART protocol sends those report rows to the PC export software. It does not export the raw numeric fault code as a dedicated report field.

## Master Error Code Reference

| Code | Stage | Meaning | Typical Trigger In Fixture | Set In | Fault Screen View |
| --- | --- | --- | --- | --- | --- |
| `0x0000` | All stages | No error | A stage completed successfully | `comm_test.c`, `di_test.c`, `ai_test.c`, `output_test.c`, `button_led_test.c` | No fault screen. The stage stays green and the test advances. |
| `0x0101` | Communication | No response to `PING` | The STM32 does not receive `PONG` after the allowed communication attempts | `Core/Src/comm_test.c` | `Communication` / Expected: `PONG response` / Actual: `No response` |
| `0x0102` | Communication | Bad `PING` reply | A reply is received, but it is not `PONG` | `Core/Src/comm_test.c` | `Communication` / Expected: `PONG response` / Actual: `Invalid reply` |
| `0x0201` | MCU Reset | No reconnect after reset | The reset pulse completes but the ESP32 does not reconnect with the expected post-reset `PONG` | `Core/Src/comm_test.c` | `MCU Reset` / Expected: `PONG response` / Actual: `No response` |
| `0x0202` | MCU Reset | Bad reconnect reply after reset | A reply arrives after reset, but it is not `PONG` | `Core/Src/comm_test.c` | `MCU Reset` / Expected: `PONG response` / Actual: `Invalid reply` |
| `0x0203` | MCU Reset | Reset pulse verification failure | The STM32 detects a mismatch while verifying the PB11 reset pulse path | `Core/Src/comm_test.c` | `MCU Reset` / Expected: `PB11 pulse + PONG after reset` / Actual: `Pulse verify failed` |
| `0x0301` | Fault Check | No acknowledgment after fault pulse | The fixture pulses the fault line but does not receive the expected `OK` response | `Core/Src/comm_test.c` | `Fault Check` / Expected: `OK response` / Actual: `No response` |
| `0x0302` | Fault Check | Bad acknowledgment after fault pulse | A reply is received after the fault pulse, but it is not `OK` | `Core/Src/comm_test.c` | `Fault Check` / Expected: `OK response` / Actual: `Invalid reply` |
| `0x0303` | Fault Check | Fault pulse verification failure | The STM32 detects a mismatch while verifying the PA8 fault pulse path | `Core/Src/comm_test.c` | `Fault Check` / Expected: `PA8 pulse + OK response` / Actual: `Pulse verify failed` |
| `0x0401` | Digital Inputs | No response during a digital input check | The ESP32 does not return a `DIxOK` or `DIxFAIL` token for the active channel | `Core/Src/di_test.c` | `Digital Input <channel>` / Expected: `High then Low` / Actual: `No response` |
| `0x0402` | Digital Inputs | Fail token returned | The ESP32 returns `DIxFAIL`, indicating the expected transition was not seen correctly | `Core/Src/di_test.c` | `Digital Input <channel>` / Expected: `High then Low` / Actual: `Invalid transition` |
| `0x0403` | Digital Inputs | Invalid digital input reply | The reply token is malformed, wrong, or does not match the active digital input test | `Core/Src/di_test.c` | `Digital Input <channel>` / Expected: `High then Low` / Actual: `Invalid reply` |
| `0x0501` | Analog Inputs | No analog sample reply | The ESP32 does not return a sample for the requested analog channel and level | `Core/Src/ai_test.c` | `Analog Input <channel> - <level>` / Expected: `Sample reply` / Actual: `No response` |
| `0x0502` | Analog Inputs | Invalid analog sample reply | The reply is malformed or does not match the expected prefix such as `OK_AI1_LOW:` | `Core/Src/ai_test.c` | `Analog Input <channel> - <level>` / Expected: `OK_AI<channel>_<level>:<V>` / Actual: `Invalid reply` |
| `0x0503` | Analog Inputs | Analog value out of range | The reply is valid, but the measured engineering value is outside the allowed tolerance band | `Core/Src/ai_test.c` | `Analog Input <channel> - <level>` / Expected: formatted tolerance band / Actual: formatted measured value |
| `0x0601` | Outputs | No relay acknowledgment | The active relay command does not receive an acknowledgment token such as `OK_K1_ON` | `Core/Src/output_test.c` | `Relay K1 ON`, `Relay K1 OFF`, `Relay K2 ON`, or `Relay K2 OFF` / Expected: matching `OK_Kx_*` token / Actual: `No response` |
| `0x0602` | Outputs | Invalid relay acknowledgment | A relay reply is received, but it does not match the expected token for the active phase | `Core/Src/output_test.c` | `Relay K1 ON`, `Relay K1 OFF`, `Relay K2 ON`, or `Relay K2 OFF` / Expected: matching `OK_Kx_*` token / Actual: `Invalid reply` |
| `0x0603` | Outputs | Relay state mismatch | The relay readback does not match the expected NO or NC state for the commanded phase | `Core/Src/output_test.c` | `Relay K1 ON`, `Relay K1 OFF`, `Relay K2 ON`, or `Relay K2 OFF` / Expected: commanded readback state / Actual: measured readback state |
| `0x0701` | Buttons / LEDs | No acknowledgment during button or LED control | The ESP32 does not acknowledge the active button or LED step | `Core/Src/button_led_test.c` | Step name varies, for example `USR_0 Press`, `PNL_1 Release`, or `LED Begin` / Expected: active token or `Pressed and released` / Actual: `No response` |
| `0x0702` | Buttons / LEDs | Invalid acknowledgment during button or LED control | A reply is received, but it is malformed or does not match the active step | `Core/Src/button_led_test.c` | Step name varies, for example `USR_0 Press`, `LED Rotate`, or `LED Stop` / Expected: active token or `Pressed and released` / Actual: `Invalid reply` |
| `0x0703` | Buttons / LEDs | Operator rejected the LED visual check | The automated portion reached the LED visual confirmation step and the operator selected `LED's BAD` | `Core/Src/button_led_test.c` | `LED Visual Check` / Expected: `Operator pressed GOOD` / Actual: `Operator pressed BAD` |
| `0x0704` | Buttons / LEDs | Manual press or release timeout | The operator did not press or release the highlighted button within the allowed time window | `Core/Src/button_led_test.c` | Active button step such as `USR_0 Press` or `PNL_3 Release` / Expected: `Pressed and released` / Actual: `Press timeout` or `Release timeout` |

## Stage-By-Stage Tie-In To The Test Flow

### Communication, MCU Reset, And Fault Check

The first screen in the fixture flow is `Comm_Reset_Screen`. These three stages are implemented in `Core/Src/comm_test.c` and correspond to the `0x01xx`, `0x02xx`, and `0x03xx` families.

- Communication verifies `PING` to `PONG`.
- MCU Reset verifies the PB11 reset pulse and the ESP32 reconnect path.
- Fault Check verifies the PA8 fault pulse and the expected `OK` acknowledgment.

If any of these stages fail, the screen briefly turns the relevant text red, stores the error in `FaultInfo`, and then transitions to `Fault_Screen`.

### Digital Inputs

The digital input stage runs on `Inputs_Screen` and is implemented in `Core/Src/di_test.c`. The fixture checks each digital input channel and expects the transition represented on the fault screen as `High then Low`.

The `0x04xx` family distinguishes between:

- no response from the ESP32
- a fail token that indicates a bad transition
- a malformed or unexpected reply

### Analog Inputs

The analog input stage also runs on `Inputs_Screen`, using `Core/Src/ai_test.c`. Each analog channel is checked at multiple levels. The on-screen fault text uses the channel and level, for example `Analog Input 2 - MID`.

The `0x05xx` family distinguishes between:

- no reply to the sample request
- a malformed or mismatched sample reply
- a valid sample that is outside the tolerance window

For out-of-range cases, the fault screen shows a formatted expected range and the formatted measured value.

### Outputs

The outputs stage runs on `Outputs_Screen` and is implemented in `Core/Src/output_test.c`.

In the current fixture revision:

- Digital Output 1 and Digital Output 2 are temporarily automatic pass items.
- The active failure-generating portion of the outputs stage is the relay verification sequence.

The `0x06xx` family therefore ties to relay command and relay readback failures for:

- `Relay K1 ON`
- `Relay K1 OFF`
- `Relay K2 ON`
- `Relay K2 OFF`

### Buttons / LEDs

The final active test stage runs on `Buttons_LEDs_Screen` and is implemented in `Core/Src/button_led_test.c`.

This stage includes:

- button press and release confirmation for `USR_0`, `PNL_0`, `PNL_1`, `PNL_2`, and `PNL_3`
- LED control steps such as `LED Begin`, `LED Rotate`, and `LED Stop`
- the final operator decision at `LED Visual Check`

The `0x07xx` family distinguishes between:

- no acknowledgment from the ESP32
- invalid acknowledgment content
- operator rejection of the LED visual result
- timeout during manual press or release

## What The Operator Sees On The Fault Screen

`TouchGFX/gui/include/gui/common/FrontendApplication.hpp` defines the shared `FaultInfo` payload:

- `errorCode`
- `testName`
- `expected`
- `actual`

`TouchGFX/gui/src/containers/Fault_Screen.cpp` formats the numeric code as hexadecimal using `0x%04X` and displays all four fields on the screen.

In practice, the operator sees:

- the hex error code, for example `0x0503`
- the failed test or step name
- the expected reply, transition, or state
- the actual reply, timeout result, or measured state

This is why the same numeric code can still show different step names or expected values inside one family. For example, `0x0601` always means output-stage no response, but the actual fault text still depends on whether the active relay phase was `Relay K1 ON`, `Relay K1 OFF`, `Relay K2 ON`, or `Relay K2 OFF`.

## How The Codes Relate To Reporting And Export

The numeric EOL code is primarily a live fixture diagnostic used by the active test flow and `Fault_Screen`.

The run report stored by `Core/Inc/eol_report.h` records:

- row pass or fail status
- test name
- expected text
- actual text

It does not define a dedicated per-row numeric error-code field.

`Core/Src/export_uart.c` exports the completed run as text rows using the export protocol:

- `STATUS|...`
- `EXPORT_BEGIN|...`
- `ROW|index|status|testName|expected|actual`
- `EXPORT_END|...`

This means:

- the operator sees the raw hex code on the fixture during the fault
- the exported PC-side record preserves the readable test row details
- the exported file is intended for traceability, not as the primary source of the numeric fault code

## Current Revision Notes

- `0x060x` errors currently apply to relay verification. The digital output portion of the outputs screen is still a temporary automatic pass in this revision.
- `0x0704` reflects the current operator-assisted button workflow.
- Failed runs remain exportable after the fixture reaches `Fault_Screen`.

## Separate Non-EOL Error Scheme

The project also contains export-protocol error strings such as `EXPORT_ERROR|NOT_READY`, `EXPORT_ERROR|BAD_SEQ`, and `EXPORT_ERROR|ALREADY_EXPORTED`.

These are not part of the EOL fault-code enum in `eol_test_protocol.h`. They belong to the PC export path and should be treated as a separate interface-level error scheme.
