# EOL Test Fixture User Manual

This is the primary operating guide for technicians and operators using the Rite-Hite end-of-line test fixture.

It explains:

- What the fixture and PC app do
- How to run the normal production workflow
- How to use the One Click workflow
- How to export and find CSV results
- What the operator must still decide manually

For engineering detail, see [Project_Knowledge_Base.md](Project_Knowledge_Base.md).

## System Overview

The station is made up of three connected pieces:

- The STM32U575 fixture, which runs the touchscreen UI and the test sequence
- The Rite-Hite Connect target module running the ESP32-S3 test firmware
- The Windows desktop app, which uploads test firmware, exports results, and can run the One Click workflow

The fixture is the source of truth for the test result. It creates the pass/fail report row by row while the test runs. The desktop app reads that completed report and saves it to CSV.

## What Is Tested

The automatic production flow covers the following sequence:

1. Communication
2. MCU Reset
3. Fault Check
4. Digital Inputs
5. Analog Inputs
6. Digital Outputs
7. Relay Outputs
8. Buttons
9. LED Visual Check

### Current Revision Notes

- Digital Output 1 and Digital Output 2 are temporary auto-pass report rows.
- The fixture automatically drives the button sequence during the Buttons stage.
- The technician still makes the final LED decision by pressing `LED's GOOD` or `LED's BAD` on the touchscreen.
- Failed runs remain exportable.
- The desktop app production firmware upload feature is not available yet.

## Before You Start

Before running any test:

1. Power the fixture and confirm the touchscreen starts normally.
2. Verify that the DUT or harness is connected correctly.
3. Verify that the correct test firmware is present on the Rite-Hite Connect module, or confirm that the programmer is available so the desktop app can upload it.
4. Confirm that the Windows app opens and the exports folder is available.
5. Review [preflight_checklist.md](preflight_checklist.md) if station readiness is in doubt.

## Fixture Workflow

The fixture itself has two entry paths.

### `Begin AUTO`

This is the normal production path.

Use `Begin AUTO` when you want the fixture to run the complete automatic test sequence.

### `Begin MANUAL`

This is the service and debug path.

Use `Begin MANUAL` only when engineering or maintenance instructs you to use the GPIO service screen.

## Standard Production Workflow

This is the normal workflow when the operator starts the test on the fixture and exports afterward from the PC.

### Step 1: Start The Fixture Run

On the fixture touchscreen:

1. Confirm the start screen is visible.
2. Press `Begin AUTO`.

### Step 2: Let The Automatic Stages Run

The fixture then runs:

- Communication
- MCU Reset
- Fault Check
- Digital Inputs
- Analog Inputs
- Outputs
- Buttons

During these stages:

- Observe the screen.
- Do not disconnect the DUT.
- Do not disconnect the fixture USB or programmer.
- Do not restart the run unless a fault requires a new test attempt after correction.

### Step 3: Make The LED Decision

At the end of the run, the fixture reaches the LED visual review.

At that point:

1. Look at the LED behavior on the fixture.
2. Press `LED's GOOD` if the LED behavior is correct.
3. Press `LED's BAD` if the LED behavior is incorrect.

### Step 4: Record The Result

- If the fixture reaches `Test_Complete_Screen`, the run passed.
- If the fixture reaches `Fault_Screen`, the run failed.

On a failure, record:

- The displayed hex fault code
- The failed step name
- The expected value
- The actual value

### Step 5: Export The CSV

In the Windows desktop app:

1. Go to `Device Tools`.
2. Press `Extract Data from Test Fixture`.
3. Save or confirm the CSV.
4. Verify that the file appears in `File Explorer`.

## One Click Workflow

One Click is the PC-driven workflow that uploads firmware, starts the fixture automatically, tracks the live stage, and exports the CSV automatically at the end.

### What One Click Does

When you press `Start One Click Test`, the app:

1. Uploads the Rite-Hite Connect test firmware
2. Waits for the module to boot
3. Sends `START_AUTO` to the fixture
4. Polls the fixture status until the test completes
5. Waits for the LED visual decision
6. Exports the final CSV automatically

### Before Starting One Click

On the `One Click Test` page, confirm:

- The fixture is detected
- The programmer is detected
- The start button is enabled

### Running One Click

1. Press `Start One Click Test`.
2. Watch the progress rail and the Activity Log.
3. Follow any technician prompt shown in the Activity Log.

### One Click Progress Rail

The One Click progress rail follows the real fixture stage:

1. Upload Firmware
2. Comms
3. Digital Inputs
4. Analog Inputs
5. Digital Outputs
6. Relay Outputs
7. Buttons
8. LEDs
9. Extract Data

Important behavior:

- The current stage is shown in red.
- Completed stages turn green.
- The app updates when the STM32 changes stage.
- The app is not using a fake timer or fixed delay to move the progress bar.

### Technician Action During One Click

The app still requires the technician to make the final LED decision on the fixture.

When the fixture reaches the LED stage:

1. The Activity Log shows a technician action prompt.
2. Go to the fixture touchscreen.
3. Press `LED's GOOD` or `LED's BAD`.

### What One Click Locks While Running

While One Click is active, the app blocks:

- Manual extract
- Manual test-firmware upload
- Starting another One Click run

This prevents the active run from being interrupted or exported early.

## Desktop App Pages

## File Explorer

Use `File Explorer` to:

- Confirm that a CSV was created
- Search exported files by filename
- Filter by result outcome
- Open the exports folder
- Change the exports folder
- Reset to the default exports folder

### Default Export Location

By default, exports are stored in the app's `exports` folder. The export location can be changed in the app and is stored locally on the PC.

### CSV Format

Each exported CSV contains these columns:

- Date and Time
- Test name
- Expected Result
- Actual result
- Pass/Fail

File names use this pattern:

`YYYY-MM-DD_HH-MM-SS_run<sequence>_<outcome>.csv`

## Device Tools

Use `Device Tools` for manual PC-side actions.

### Available Actions

- `Extract Data from Test Fixture`
- `Upload Test Firmware`
- `Upload Production Firmware`

### Current Limitation

`Upload Production Firmware` currently opens a `Coming Soon` message. It is not an active production operation yet.

### Last Successful Operation

The Device Tools page also shows the last successful export or last successful test-firmware upload recorded on that PC.

## One Click Test

Use this page when you want the desktop app to run the full workflow.

It includes:

- A connection-readiness card
- A start button
- The live progress rail

## Activity Log

Use the Activity Log to see:

- Upload progress
- Start and stop messages
- One Click stage changes
- LED technician prompts
- Export completion
- Error messages

## Settings

The app stores local station settings such as:

- Language
- Font size
- Confirm before extract
- Auto refresh on tab switch
- Reduced motion
- Dark mode preference
- Export folder path

The current supported interface languages are:

- English
- Spanish
- Hmong

## Keyboard Shortcuts

The current desktop app includes these useful shortcuts:

- `F5`: Refresh file list
- `Ctrl+O`: Open exports folder
- `Ctrl+E`: Extract data from the fixture
- `Ctrl+U`: Upload test firmware
- `Esc`: Exit fullscreen or maximized mode

## What To Do On Pass

If the run passes:

1. Confirm the fixture shows `Test_Complete_Screen`, or confirm that One Click completed successfully.
2. Confirm the CSV exists in `File Explorer`.
3. Record the unit as passed according to plant procedure.

## What To Do On Fail

If the run fails:

1. Record the fault code and failed step from `Fault_Screen`.
2. Export the failed run.
3. Use [troubleshooting_guide.md](troubleshooting_guide.md).
4. Use [error_code_reference.md](error_code_reference.md) for exact fault meaning.
5. Correct the issue before restarting the test.

## Operator Rules

- Use `Begin AUTO` for normal production runs.
- Use `Begin MANUAL` only for service or debug work.
- Do not bypass communication, reset, or fault-check failures.
- Do not mark a unit as passed unless the fixture completes successfully and the final LED decision is `GOOD`.
- Do not remove a failed unit until the required failure information and CSV export have been captured.
- Do not assume the production firmware upload button is active; it is not implemented yet.

## Related Documents

- [preflight_checklist.md](preflight_checklist.md)
- [screen_guide.md](screen_guide.md)
- [test_procedure.md](test_procedure.md)
- [troubleshooting_guide.md](troubleshooting_guide.md)
- [error_code_reference.md](error_code_reference.md)
