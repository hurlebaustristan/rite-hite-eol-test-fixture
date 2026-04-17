# End-of-Line Test Procedure

This document defines the current production procedure for the Rite-Hite EOL test fixture.

It covers:

- The standard fixture-driven workflow
- The desktop-app One Click workflow
- Required operator actions
- Pass/fail handling
- CSV export handling

Use this with [user_manual.md](user_manual.md), [preflight_checklist.md](preflight_checklist.md), and [error_code_reference.md](error_code_reference.md).

## Scope

This procedure applies to the current fixture revision built around:

- STM32U575 fixture firmware and TouchGFX UI
- ESP32-S3 Rite-Hite Connect test firmware
- Windows PySide6 export application

## Roles

### Technician / Operator

- Connect the unit or harness
- Start the run using the selected workflow
- Make the final LED visual decision
- Export or confirm the exported CSV
- Record pass/fail disposition

### Engineer / Maintenance

- Maintain fixture firmware and station wiring
- Use the manual GPIO screen when required
- Resolve repeated or station-level failures

## Preconditions

Before starting:

- The fixture is powered and reaches the start screen
- The DUT or harness is connected correctly
- The desktop app opens normally
- The fixture serial path is available for export
- The programmer path is available if firmware upload is required
- The operator is ready to make the final LED decision

## Test Sequence Overview

The fixture automatic flow runs in this order:

1. Communication
2. MCU Reset
3. Fault Check
4. Digital Inputs
5. Analog Inputs
6. Digital Outputs
7. Relay Outputs
8. Buttons
9. LED Visual Check

### Current Revision Note

- Digital Output 1 and Digital Output 2 are temporary auto-pass rows.
- The fixture automatically drives the button sequence.
- The LED decision is still manual.

## Procedure A: Standard Fixture Workflow

Use this procedure when the run is started from the fixture touchscreen and exported afterward from `Device Tools`.

### Step 1: Prepare The Station

1. Complete the readiness checks in [preflight_checklist.md](preflight_checklist.md).
2. Confirm the fixture start screen is visible.
3. Confirm the correct test firmware is available on the Rite-Hite Connect module, or load it from the desktop app before running.

### Step 2: Start The Automatic Fixture Run

1. On the touchscreen, press `Begin AUTO`.
2. Confirm that the fixture enters `Comm_Reset_Screen`.

### Step 3: Allow Automatic Stages To Complete

The fixture runs the following without operator input:

- Communication
- MCU Reset
- Fault Check
- Digital Inputs
- Analog Inputs
- Outputs
- Buttons

During this time:

- Do not disconnect cables
- Do not power-cycle the station
- Do not start a second test

### Step 4: Perform The LED Visual Decision

When the fixture reaches the LED visual review:

1. Observe the LED pattern.
2. Press `LED's GOOD` if correct.
3. Press `LED's BAD` if incorrect.

### Step 5: Record The Fixture Result

If the run passes:

- The fixture reaches `Test_Complete_Screen`.

If the run fails:

- The fixture reaches `Fault_Screen`.
- Record the fault code, failed step, expected value, and actual value as required by local process.

### Step 6: Export The Run

1. Open the desktop app.
2. Go to `Device Tools`.
3. Press `Extract Data from Test Fixture`.
4. Confirm that one CSV is created for the run.
5. Verify that the file appears in `File Explorer`.

## Procedure B: One Click Workflow

Use this procedure when the desktop app will upload the test firmware, start the run, track progress, and export the CSV automatically.

### Step 1: Confirm Readiness In The App

On the `One Click Test` page:

1. Confirm the fixture is detected.
2. Confirm the programmer is detected.
3. Confirm the `Start One Click Test` button is enabled.

### Step 2: Start One Click

1. Press `Start One Click Test`.
2. Confirm the Activity Log begins showing upload and startup messages.

### Step 3: Let One Click Run

The desktop app performs the following:

1. Uploads the Rite-Hite Connect test firmware
2. Waits for module boot
3. Sends `START_AUTO` to the fixture
4. Polls fixture status during the run
5. Advances the progress rail as the fixture changes stage

The progress rail follows:

1. Upload Firmware
2. Comms
3. Digital Inputs
4. Analog Inputs
5. Digital Outputs
6. Relay Outputs
7. Buttons
8. LEDs
9. Extract Data

### Step 4: Make The LED Decision

When the Activity Log indicates technician action is required:

1. Go to the fixture touchscreen.
2. Observe the LED pattern.
3. Press `LED's GOOD` or `LED's BAD`.

### Step 5: Confirm Automatic Export

At the end of a successful One Click flow:

1. The app exports the completed run automatically.
2. The Activity Log reports completion.
3. The CSV appears in `File Explorer`.

If One Click ends in a fixture fail:

- The app still exports the completed failed run once the fixture report reaches the ready state.

## Pass Criteria

The unit is a PASS only when all of the following are true:

- Communication passes
- MCU Reset passes
- Fault Check passes
- All digital input checks pass
- All analog input checks pass
- Output and relay checks pass under the current revision rules
- The button sequence completes successfully
- The technician presses `LED's GOOD`
- The run reaches `Test_Complete_Screen` or a One Click pass result is reported

## Fail Criteria

The unit is a FAIL if any of the following occur:

- Any fixture stage returns an error
- A relay verification fails
- A button or LED stage returns a protocol, timeout, or state-mismatch error
- The technician presses `LED's BAD`
- The fixture reaches `Fault_Screen`

## Output And Reporting

### Fixture Report Behavior

The fixture builds a report row for each checked item during the run.

Completed runs have one of these report states:

- `READY`: completed and not yet exported
- `EXPORTED`: already exported once

### CSV Output

Each export creates one CSV file containing:

- Date and Time
- Test name
- Expected Result
- Actual result
- Pass/Fail

### File Naming

CSV files are named using:

`YYYY-MM-DD_HH-MM-SS_run<sequence>_<outcome>.csv`

## Operator Warnings

- Do not bypass communication, reset, or fault-check failures.
- Do not assume Digital Output 1 and Digital Output 2 are fully validated in the current revision; they are temporary auto-pass rows.
- Do not walk away before the LED visual decision is made.
- Do not attempt manual extract or manual firmware upload while One Click is running.
- Do not expect the desktop app production-firmware upload action to work yet.

## Records

Recommended run record fields:

- Date and time
- Operator name
- Fixture ID
- DUT or unit identifier
- Pass or fail result
- Fault code if failed
- CSV filename

## Related Documents

- [user_manual.md](user_manual.md)
- [screen_guide.md](screen_guide.md)
- [troubleshooting_guide.md](troubleshooting_guide.md)
- [error_code_reference.md](error_code_reference.md)
