# End-of-Line Error Code Reference

This document is the current reference for:

- STM32 fixture fault codes shown on `Fault_Screen`
- Fixture report states used by export and One Click
- Export protocol error strings used by the desktop app

Use this with [troubleshooting_guide.md](troubleshooting_guide.md) for recovery steps.

## How To Read The Fault Codes

The fixture uses one shared fault-code namespace.

| Range | Stage |
| --- | --- |
| `0x0000` | No error |
| `0x01xx` | Communication |
| `0x02xx` | MCU Reset |
| `0x03xx` | Fault Check |
| `0x04xx` | Digital Inputs |
| `0x05xx` | Analog Inputs |
| `0x06xx` | Outputs / Relays |
| `0x07xx` | Buttons / LEDs |

The fixture displays these codes on `Fault_Screen` along with:

- Failed step name
- Expected value or condition
- Actual value or condition

## Fixture Fault Codes

| Code | Stage | Meaning | Typical Fault-Screen Context | First Checks |
| --- | --- | --- | --- | --- |
| `0x0000` | All | No error | No fault screen; stage passed | None |
| `0x0101` | Communication | No reply to `PING` | `Communication`, expected `PONG response`, actual `No response` | Power, UART wiring, test firmware |
| `0x0102` | Communication | Reply received but not `PONG` | `Communication`, expected `PONG response`, actual `Invalid reply` | Protocol mismatch, wrong firmware |
| `0x0201` | MCU Reset | No reconnect after reset | `MCU Reset`, expected `PONG response`, actual `No response` | Reset path, boot behavior |
| `0x0202` | MCU Reset | Reply after reset was not `PONG` | `MCU Reset`, expected `PONG response`, actual `Invalid reply` | Post-reset protocol path |
| `0x0203` | MCU Reset | Reset pulse verify failed | `MCU Reset`, expected reset pulse plus reconnect, actual pulse verify failure | Reset GPIO path |
| `0x0301` | Fault Check | No `OK` reply after fault pulse | `Fault Check`, expected `OK response`, actual `No response` | Fault line wiring |
| `0x0302` | Fault Check | Reply after fault pulse was not `OK` | `Fault Check`, expected `OK response`, actual `Invalid reply` | Fault-check protocol path |
| `0x0303` | Fault Check | Fault pulse verify failed | `Fault Check`, expected fault pulse plus `OK`, actual pulse verify failure | Fault GPIO path |
| `0x0401` | Digital Inputs | No digital-input reply | `Digital Input X`, expected `High then Low`, actual `No response` | Input wiring, target response |
| `0x0402` | Digital Inputs | Target returned fail token | `Digital Input X`, expected `High then Low`, actual `Invalid transition` | Input state path |
| `0x0403` | Digital Inputs | Invalid digital-input reply | `Digital Input X`, expected `High then Low`, actual `Invalid reply` | Protocol token mismatch |
| `0x0501` | Analog Inputs | No analog reply | `Analog Input X - LEVEL`, expected sample reply, actual `No response` | Analog route, target response |
| `0x0502` | Analog Inputs | Invalid analog reply | `Analog Input X - LEVEL`, expected `OK_AI...`, actual `Invalid reply` | Analog protocol token |
| `0x0503` | Analog Inputs | Analog reading out of tolerance | `Analog Input X - LEVEL`, expected tolerance band, actual measured value | Wiring, scaling, measurement path |
| `0x0601` | Outputs | No relay acknowledgment | `Relay K1 ON`, `Relay K1 OFF`, `Relay K2 ON`, or `Relay K2 OFF` with `No response` | Relay command path |
| `0x0602` | Outputs | Invalid relay acknowledgment | Relay step with `Invalid reply` | Relay protocol token |
| `0x0603` | Outputs | Relay state mismatch | Relay step with expected NO/NC state vs actual state | Relay readback wiring |
| `0x0701` | Buttons / LEDs | No acknowledgment during button or LED stage | Button or LED step with `No response` | Button/LED interface, target reply |
| `0x0702` | Buttons / LEDs | Invalid acknowledgment during button or LED stage | Button or LED step with `Invalid reply` | Button/LED protocol token |
| `0x0703` | Buttons / LEDs | Operator rejected LED visual check | `LED Visual Check`, expected `Operator pressed GOOD`, actual `Operator pressed BAD` | LED behavior review |
| `0x0704` | Buttons / LEDs | Button stage timeout | Button step, expected `Pressed and released`, actual `Press timeout` or `Release timeout` | Automatic button-detect path |
| `0x0705` | Buttons / LEDs | Button state mismatch | Button step, expected one button state, actual opposite state | Q3 input readback, button wiring |

## Current Revision Notes

- `0x060x` faults apply to relay validation in the current revision. Digital Output 1 and Digital Output 2 are temporary auto-pass rows.
- The technician does not manually press the hardware button sequence during the normal automatic test. The fixture drives those button states automatically.
- The technician still chooses `LED's GOOD` or `LED's BAD`, which is why `0x0703` still exists as a real operator decision.

## Fixture Report States

The fixture report service uses these run states:

| State | Meaning | What The App Does |
| --- | --- | --- |
| `EMPTY` | No completed run is available | Export is blocked |
| `RUNNING` | Test is still active | Export is blocked |
| `WAITING_VISUAL` | Fixture is waiting for the LED visual decision | Export is blocked and One Click prompts the technician |
| `READY` | Completed run is ready to export | Manual export and One Click export are allowed |
| `EXPORTED` | Run has already been exported | The desktop app treats the run as already exported |

### One Click Phase Field

When the fixture is in auto mode, the desktop app can also receive the active phase:

- `COMMS`
- `DIGITAL_INPUTS`
- `ANALOG_INPUTS`
- `DIGITAL_OUTPUTS`
- `RELAY_OUTPUTS`
- `BUTTONS`
- `LEDS`

Examples:

- `STATUS|RUNNING|COMMS`
- `STATUS|WAITING_VISUAL|LEDS`
- `STATUS|READY|PASS|12|18`

## Export Protocol Errors

These are not fixture `Fault_Screen` codes. They are serial protocol errors used between the desktop app and the export service.

| Message | Meaning | Typical Cause |
| --- | --- | --- |
| `EXPORT_ERROR|NOT_READY` | Run is not ready to export | Test still running or waiting for LED decision |
| `EXPORT_ERROR|BAD_SEQ` | Requested run sequence does not match | App requested the wrong report sequence |
| `EXPORT_ERROR|ALREADY_EXPORTED` | Run already exported | A CSV was already pulled for that run |

## Auto-Test Start Errors

These are also protocol-level messages, not `Fault_Screen` codes.

| Message | Meaning | Typical Cause |
| --- | --- | --- |
| `AUTO_STARTED` | Fixture accepted `START_AUTO` | Normal One Click start |
| `AUTO_ERROR|ALREADY_RUNNING` | Fixture rejected `START_AUTO` because an auto run is active | A previous auto run is still running |

## How Codes Flow Through The System

1. A test engine detects a failure and sets an `EOL_ERR_*` code.
2. The active TouchGFX screen fills the shared fault information fields.
3. The fixture transitions to `Fault_Screen`.
4. The fixture report stores the human-readable failed row.
5. The desktop app exports the report rows to CSV.

Important detail:

- The exported CSV is based on report rows such as test name, expected, actual, and pass/fail.
- The CSV does not have a dedicated fault-code column.
- The hex fault code is primarily shown live on the fixture screen.

## Related Documents

- [troubleshooting_guide.md](troubleshooting_guide.md)
- [user_manual.md](user_manual.md)
- [test_procedure.md](test_procedure.md)
