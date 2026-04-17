# EOL Test Fixture Troubleshooting Guide

This guide is written for technicians and operators who need a clear next action when the fixture or desktop app does not behave as expected.

Use this with [error_code_reference.md](error_code_reference.md) for exact code meanings.

## Immediate Actions After Any Failure

If the fixture reaches `Fault_Screen`:

1. Record the fault code.
2. Record the failed step.
3. Record the expected and actual values if required by local process.
4. Export the failed run.
5. Correct the likely cause before starting another run.

If the desktop app reports a failure during One Click:

1. Read the latest Activity Log entry.
2. Determine whether the failure is in firmware upload, fixture start, live run status, LED wait, or export.
3. Correct that issue before restarting One Click.

## General Rules

- Do not bypass communication, reset, or fault-check failures.
- Do not rerun the same failing setup repeatedly without checking the station.
- If the same failure appears on multiple boards, suspect the station before the DUT.
- If the desktop app and fixture disagree about the state of the run, trust the fixture screen first, then export and inspect the CSV.

## Quick Symptom Guide

| Symptom | Likely Area | First Checks |
| --- | --- | --- |
| No fixture COM port appears | PC connection / ST-LINK | Check USB cable, ST-LINK connection, Device Manager, and correct fixture port |
| Programmer not detected | Programmer path | Check programmer cable, USB connection, and that only the intended programmer is attached |
| One Click will not start | App readiness | Confirm fixture port detected, programmer detected, and no other One Click run is active |
| One Click stops at LEDs | Normal manual hold point | Go to the fixture and press `LED's GOOD` or `LED's BAD` |
| Export says not ready | Run still active | Confirm the fixture has reached `Test_Complete_Screen` or `Fault_Screen` |
| Export says already exported | Report state | Check `File Explorer` for the existing CSV |
| Repeated relay failures | Output hardware or readback | Check relay wiring, contact readback, and relay board behavior |
| Button or LED state mismatch | Button/LED interface | Check Q3 inputs, button wiring, and LED hardware |

## Fixture Stage Failures

## Communication Failures (`0x0101`, `0x0102`)

Meaning:

- The fixture did not receive the expected `PONG`
- Or it received the wrong reply

First checks:

1. Confirm the Rite-Hite Connect module is powered.
2. Confirm the correct ESP32 test firmware is loaded.
3. Check STM32 to ESP32 serial wiring.
4. Check for loose connectors.

Escalate if:

- The same fault repeats after power and wiring are confirmed.

## MCU Reset Failures (`0x0201` to `0x0203`)

Meaning:

- The reset pulse path failed
- Or the target did not come back correctly after reset

First checks:

1. Check the reset wiring and hardware path.
2. Confirm the target reboots normally.
3. Confirm the target returns the expected reply after reset.

Escalate if:

- Reset failures continue on known-good targets.

## Fault Check Failures (`0x0301` to `0x0303`)

Meaning:

- The fault pulse path or acknowledgment failed

First checks:

1. Inspect the FAULT control path.
2. Verify wiring from the fixture to the target.
3. Confirm the target responds to the fault-check logic.

## Digital Input Failures (`0x0401` to `0x0403`)

Meaning:

- The target did not acknowledge the digital input test
- Or it reported the wrong state transition

First checks:

1. Check the digital input wiring.
2. Confirm the correct board or harness is attached.
3. Inspect the affected channel connection.

## Analog Input Failures (`0x0501` to `0x0503`)

Meaning:

- No analog reply
- Invalid analog reply
- Measured value outside tolerance

First checks:

1. Check analog wiring and mux path.
2. Confirm the DAC and analog route are operating.
3. Compare the expected and actual reading shown on `Fault_Screen`.

## Output And Relay Failures (`0x0601` to `0x0603`)

Meaning:

- Relay command acknowledgment failed
- Or relay NO/NC readback did not match expectation

Important note:

- In the current revision, Digital Output 1 and Digital Output 2 are temporary auto-pass rows.
- Real failures in this stage are normally relay failures.

First checks:

1. Check relay wiring.
2. Check K1 and K2 readback signals.
3. Confirm contact polarity is correct.

## Buttons And LED Failures (`0x0701` to `0x0705`)

Meaning:

- No reply, invalid reply, timeout, operator reject, or state mismatch during the Buttons / LEDs stage

Important note:

- The fixture automatically drives the button sequence.
- The operator only makes the final LED visual decision.

First checks:

1. Check the button and LED interface wiring.
2. Check the Q3 input readings on the ESP32 side.
3. Confirm the LED pattern actually matches expected behavior before choosing `LED's BAD`.
4. If the code is `0x0705`, inspect the measured button state path because the target saw the opposite state from what the fixture expected.

## Desktop App Problems

## Fixture Not Detected

Symptoms:

- Connection Readiness shows no serial ports
- `Extract Data from Test Fixture` cannot start
- One Click does not allow start

Checks:

1. Reconnect the fixture USB cable.
2. Confirm the ST-LINK virtual COM port appears in Windows.
3. If multiple ST-LINK ports exist, choose the correct one manually when prompted.

## Programmer Not Detected

Symptoms:

- `Upload Test Firmware` cannot start
- One Click start is blocked

Checks:

1. Reconnect the programmer cable.
2. Confirm the intended programmer is attached.
3. Remove extra programmer devices if multiple similar ports are present.

## One Click Will Not Start

Checks:

1. Confirm the fixture is detected.
2. Confirm the programmer is detected.
3. Confirm no other One Click run is active.
4. Check the Activity Log for the most recent error.

## One Click Appears Stuck

Most common causes:

- Waiting for the module to boot after firmware upload
- Waiting for the fixture to finish a stage
- Waiting for the LED visual decision

Checks:

1. Look at the active red stage in the One Click progress rail.
2. Read the latest Activity Log entries.
3. If the app is waiting at `LEDs`, go to the fixture and make the LED decision.
4. If the app times out after 5 minutes, inspect the fixture screen and target hardware manually.

## Manual Extract Is Blocked During One Click

This is expected behavior.

The app locks manual extract and manual test-firmware upload while One Click is active to prevent a corrupted or partial run flow.

## Export Problems

## Export Reports `NOT_READY`

Meaning:

- The fixture report is still running or waiting for the LED decision.

Action:

1. Confirm the run has fully completed.
2. If the fixture is waiting for the LED decision, finish that step first.

## Export Reports `ALREADY_EXPORTED`

Meaning:

- That run has already been exported once.

Action:

1. Check `File Explorer` for the existing CSV.
2. If another copy is required, export the file from the folder rather than expecting the fixture to regenerate the same run.

## Export Reports `BAD_SEQ`

Meaning:

- The requested report sequence does not match what the fixture holds.

Action:

1. Refresh the app state.
2. Query the fixture again.
3. Retry export from the latest available run only.

## Activity Log And UI Issues

If the app UI behaves unexpectedly:

1. Read the latest Activity Log message first.
2. If the window layout looks wrong, exit fullscreen or maximized mode with `Esc`, then restore the desired view.
3. Restart the app if the UI no longer responds.

If the issue repeats, escalate with:

- Screenshot of the app
- Relevant Activity Log lines
- Whether the issue occurred in Device Tools or One Click

## When To Escalate

Escalate to engineering or maintenance when:

- The same failure repeats on multiple units
- Communication, reset, or fault-check failures continue after basic checks
- Relay readback does not match physical behavior
- Button/LED mismatches continue on known-good hardware
- The desktop app repeatedly cannot detect the fixture or programmer on a known-good PC
- The station shows signs of wiring damage or unstable power

## Related Documents

- [user_manual.md](user_manual.md)
- [screen_guide.md](screen_guide.md)
- [test_procedure.md](test_procedure.md)
- [error_code_reference.md](error_code_reference.md)
