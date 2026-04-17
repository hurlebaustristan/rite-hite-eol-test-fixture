# EOL Test Fixture Troubleshooting Guide

## Purpose

This guide provides operator-focused actions for common failure situations during an EOL fixture run. It is intended to answer the question, "What should I do next?"

For exact fault-code meanings, use [Error Code Reference](error_code_reference.md). For the normal operating workflow, use [User Manual](user_manual.md).

## Immediate Actions After Any Failure

When the fixture reaches `Fault_Screen`:

1. Record the displayed error code.
2. Record the failed stage or step name.
3. Record the expected and actual values shown on the screen if required by local process.
4. Do not continue the run without correcting the issue.
5. Export the failed run if your process requires failure archival.
6. Start a new run only after the likely cause has been checked or corrected.

## General Troubleshooting Rules

- Do not bypass communication or reset failures.
- Do not repeatedly rerun the same failing unit without checking setup, wiring, or firmware.
- If the same failure repeats on multiple units, suspect the fixture, wiring, or station setup before suspecting every DUT.
- If a failure cannot be corrected with normal operator checks, escalate to maintenance or engineering.

## Quick Reference By Error Family

| Error Family | Stage | Common Meaning | First Operator Checks |
| --- | --- | --- | --- |
| `0x01xx` | Communication | No response or invalid handshake reply | Check STM32 to ESP32 communication path, power, firmware, and cable connection |
| `0x02xx` | MCU Reset | Reset path or reconnect failure | Check reset wiring, reset path integrity, and ESP32 boot behavior |
| `0x03xx` | Fault Check | Fault pulse or acknowledgment failure | Check fault line wiring and response path |
| `0x04xx` | Digital Inputs | No response, bad transition, or bad reply | Check digital input wiring and correct board/channel setup |
| `0x05xx` | Analog Inputs | No reply, bad reply, or out-of-range measurement | Check analog wiring, scaling, and channel routing |
| `0x06xx` | Outputs | Relay acknowledgment or relay state mismatch | Check relay wiring, readback polarity, and output response |
| `0x07xx` | Buttons / LEDs | Button timeout, bad reply, no reply, or LED reject | Check operator action, button wiring, LED behavior, and firmware |

## Action Guide By Failure Type

### Communication Failures (`0x0101`, `0x0102`)

Likely causes:

- ESP32 not powered
- wrong ESP32 firmware
- UART connection issue
- loose or incorrect cable

Operator actions:

1. Verify the ESP32 is powered.
2. Verify the correct EOL firmware is loaded.
3. Check the STM32 to ESP32 wiring and cable connection.
4. Rerun only after the communication path has been checked.

Escalate when:

- communication still fails after wiring and firmware checks
- multiple units fail at the same step

### MCU Reset Failures (`0x0201` to `0x0203`)

Likely causes:

- reset line not reaching the ESP32
- ESP32 not rebooting correctly
- reset pulse path wiring issue

Operator actions:

1. Check reset wiring and connections.
2. Confirm the ESP32 boots normally after reset.
3. Check for obvious fixture wiring or hardware issues in the reset path.

Escalate when:

- the reset path repeatedly fails
- pulse verification continues to fail

### Fault Check Failures (`0x0301` to `0x0303`)

Likely causes:

- fault pulse path issue
- incorrect acknowledgment path
- wiring or hardware fault

Operator actions:

1. Check the fault output path and receiving connection.
2. Verify the related wiring is secure.
3. Rerun only after the path has been checked.

Escalate when:

- repeated fault-check failures occur on multiple units

### Digital Input Failures (`0x0401` to `0x0403`)

Likely causes:

- incorrect or loose input wiring
- wrong board or wrong connection point
- response mismatch from the ESP32

Operator actions:

1. Check the digital input cable set.
2. Verify the correct board and fixture connections are installed.
3. Confirm the affected input path is connected to the intended channel.
4. Rerun after correcting the connection issue.

Escalate when:

- the same input repeatedly fails after rewiring checks

### Analog Input Failures (`0x0501` to `0x0503`)

Likely causes:

- analog wiring issue
- channel routing issue
- analog hardware path issue
- valid reading outside tolerance

Operator actions:

1. Check analog wiring and channel routing.
2. Confirm the expected analog source path is selected.
3. If the failure is out-of-range, compare the measured value on the fault screen with the expected range.
4. Rerun only after correcting the suspected wiring or hardware issue.

Escalate when:

- analog values remain out of range after setup checks
- multiple channels fail unexpectedly

### Output And Relay Failures (`0x0601` to `0x0603`)

Likely causes:

- relay wiring issue
- relay readback mismatch
- incorrect acknowledgment or no acknowledgment

Operator actions:

1. Check relay wiring and contact readback connections.
2. Confirm the output path is responding as expected.
3. Remember that the digital output section is a temporary auto-pass in this revision, so active failures are most likely in relay validation.

Escalate when:

- relay readback does not match expected behavior
- the same relay phase fails repeatedly

### Buttons / LEDs Failures (`0x0701` to `0x0704`)

Likely causes:

- wrong button pressed
- button not released in time
- wiring issue to the button interface
- LED pattern incorrect
- no or invalid acknowledgment from the ESP32

Operator actions:

1. Confirm the highlighted button was the one actually pressed.
2. Confirm the button was both pressed and released within the allowed time.
3. Check button wiring if timeouts or reply failures repeat.
4. If `LED's BAD` was selected, confirm the LED pattern truly does not match expected behavior before retrying.

Escalate when:

- the correct button is used but timeouts continue
- LED behavior remains incorrect after verification

## Export Problems

The export path uses a separate error scheme from the EOL screen fault codes.

Common export issues include:

- no completed run available yet
- wrong sequence requested
- run already exported
- wrong serial port
- communication failure to ST-LINK virtual COM port

Operator actions:

1. Confirm the run has reached `Test_Complete_Screen` or `Fault_Screen`.
2. Confirm the correct serial port is selected.
3. Confirm the ST-LINK connection is active.
4. Retry export only if the run is not already marked as exported.

## When To Escalate

Escalate to maintenance or engineering when:

- the same failure repeats across multiple units
- communication, reset, or fault-check failures continue after setup checks
- wiring appears correct but the failure does not clear
- export repeatedly fails with a known-good station setup
- there is visible hardware damage or abnormal behavior

## Related Documents

- [User Manual](user_manual.md)
- [Screen Guide](screen_guide.md)
- [Preflight Checklist](preflight_checklist.md)
- [Error Code Reference](error_code_reference.md)
- [Test Procedure](test_procedure.md)
