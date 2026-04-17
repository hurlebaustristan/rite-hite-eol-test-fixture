# EOL Test Fixture Screen Guide

This guide explains what the technician sees on the fixture touchscreen and in the Windows desktop app, and what action is required at each point.

Use this with [user_manual.md](user_manual.md), [test_procedure.md](test_procedure.md), and [troubleshooting_guide.md](troubleshooting_guide.md).

## Color Meaning

### Fixture Touchscreen

| Color | Meaning |
| --- | --- |
| Black | Pending or not yet completed |
| Yellow | Active item or operator-attention state |
| Green | Passed |
| Red | Failed |

### Desktop App One Click Progress Rail

| Color | Meaning |
| --- | --- |
| Red | Current active One Click stage |
| Green | Completed stage |
| Gray | Not started yet |

## Fixture Touchscreen Screens

## Start Screen

### Purpose

Entry point to the fixture.

### What You See

- `Begin AUTO`
- `Begin MANUAL`

### What You Do

- Press `Begin AUTO` for the normal production test.
- Press `Begin MANUAL` only for service, debugging, or engineering checks.

### What Happens Next

- `Begin AUTO` enters `Comm_Reset_Screen`.
- `Begin MANUAL` enters `Test_GPIO_Screen`.

## Comm_Reset_Screen

### Purpose

Runs the first three fixture-controlled stages:

- Communication
- MCU Reset
- Fault Check

### What You See

- The stage labels update as each sub-stage passes or fails.
- The progress bar advances through the three sub-stages.

### What You Do

- Observe only.
- Do not disconnect the DUT, fixture USB, or programmer during this stage.

### Result

- Pass: fixture advances to `Inputs_Screen`
- Fail: fixture transitions to `Fault_Screen`

## Inputs_Screen

### Purpose

Runs:

- Digital Inputs
- Analog Inputs

### What You See

- Individual input items turn green as they pass.
- The progress bar first covers digital inputs, then analog inputs.

### What You Do

- Observe only.
- Do not interrupt the run.

### Result

- Pass: fixture advances to `Outputs_Screen`
- Fail: fixture transitions to `Fault_Screen`

## Outputs_Screen

### Purpose

Runs:

- Digital Outputs
- Relay Outputs

### What You See

- `Digital Outputs` and `Relay Outputs` are shown as separate sections.
- Relay items update as K1 and K2 ON/OFF checks run.

### Current Revision Note

- Digital Output 1 and Digital Output 2 are temporary auto-pass report rows.
- Relay verification is live and can still fail.

### What You Do

- Observe only.

### Result

- Pass: fixture advances to `Buttons_LEDs_Screen`
- Fail: fixture transitions to `Fault_Screen`

## Buttons_LEDs_Screen

### Purpose

Runs the final automatic button sequence and the final manual LED decision.

### What You See

- Button labels for `USR_0`, `PNL_0`, `PNL_1`, `PNL_2`, and `PNL_3`
- LED image / animation area
- `LED's BAD`
- `LED's GOOD`

### Important Current Behavior

- The fixture automatically drives the button sequence by toggling its output lines and checking the ESP32 replies.
- The technician does not manually press the hardware button sequence during the normal automatic test.
- The technician only makes the final visual LED decision.

### What You Do

1. Watch the button stage complete automatically.
2. When the LED review begins, look at the LED pattern on the fixture.
3. Press `LED's GOOD` if the LED behavior is correct.
4. Press `LED's BAD` if the LED behavior is incorrect.

### Result

- `LED's GOOD`: fixture advances to `Test_Complete_Screen`
- `LED's BAD`: fixture transitions to `Fault_Screen`

## Fault_Screen

### Purpose

Final fail screen for the current run.

### What You See

- Hex error code
- Failed test or step name
- Expected value or condition
- Actual value or condition

### What You Do

1. Record the error information required by your process.
2. Use [error_code_reference.md](error_code_reference.md) and [troubleshooting_guide.md](troubleshooting_guide.md).
3. Export the failed run.

## Test_Complete_Screen

### Purpose

Final pass screen for the current run.

### What You Do

1. Record the board or unit as passed.
2. Export the run, or confirm that One Click exported it automatically.

## Test_GPIO_Screen

### Purpose

Manual service and debug screen reached from `Begin MANUAL`.

### What You Can Control

- DI1 through DI12 outputs
- AUX0 through AUX3 button outputs
- USR, MCU, and FAULT control outputs
- DACA0, DACA1, and DACEN
- DAC output slider

### What You Can Read

- K1NO, K1NC, K2NO, K2NC, DO1, and DO2 input readback

### What You Do

- Use only for maintenance, debug, or engineering-directed verification.

## Desktop App Screens

## File Explorer

### Purpose

Shows exported CSV files and lets the technician manage the export folder.

### What You See

- File list of exported CSVs
- Search box
- Outcome filter
- Buttons to refresh, open the folder, change the folder, or reset to default

### What You Do

- Confirm the expected CSV exists after export.
- Open or sort historical exports as needed.

## Device Tools

### Purpose

Provides the manual PC-side operations.

### What You See

- Connection Readiness card
- Last Successful Operation card
- `Extract Data from Test Fixture`
- `Upload Test Firmware`
- `Upload Production Firmware`

### What You Do

- Use `Extract Data from Test Fixture` after a normal touchscreen run to save the latest pass or fail report as a CSV.
- Use `Upload Test Firmware` to load the fixed Rite-Hite Connect test firmware to the module.
- Do not expect `Upload Production Firmware` to work yet; it currently shows a `Coming Soon` message.

## One Click Test

### Purpose

Runs the full PC-assisted workflow in one action.

### What You See

- A connection-readiness card for the fixture and programmer
- A large `Start One Click Test` button
- A live progress rail:
  1. Upload Firmware
  2. Comms
  3. Digital Inputs
  4. Analog Inputs
  5. Digital Outputs
  6. Relay Outputs
  7. Buttons
  8. LEDs
  9. Extract Data

### What You Do

1. Confirm the fixture and programmer are detected.
2. Press `Start One Click Test`.
3. Watch the Activity Log and progress rail.
4. When the app indicates technician action is required, go to the fixture and press `LED's GOOD` or `LED's BAD`.
5. Confirm the CSV is saved at the end.

### How It Behaves

- The progress rail follows the real STM32 fixture phase, not a fake timer.
- The current phase is red.
- The previous phase turns green when the next phase starts.
- Manual extract and manual test-firmware upload are locked while One Click is running.

## Activity Log

### Purpose

Shows the detailed live log for uploads, One Click progress, prompts, and exports.

### What You Use It For

- Confirm what step is active
- See upload progress
- See export completion
- See One Click prompts such as the LED visual confirmation request

## Settings

### Purpose

Stores local app preferences.

### Available Settings

- Language
- Font size
- Confirm before extract
- Auto refresh on tab switch
- Reduced motion
- Dark mode setting
- Custom exports folder path

### Language Support

The app includes:

- English
- Spanish
- Hmong
