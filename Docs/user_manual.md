# EOL Test Fixture User Manual

## Purpose

This manual is the primary operator guide for the End-of-Line (EOL) test fixture. It explains how to prepare the station, start a run, understand the screen flow, respond to pass or fail conditions, and export results.

This document is written for operators and technicians. It focuses on normal use of the fixture and avoids firmware-level detail unless it directly affects operator actions.

## Related Documents

Use this file as the main entry point. Refer to the following supporting documents as needed:

- [Preflight Checklist](preflight_checklist.md)
- [Screen Guide](screen_guide.md)
- [Troubleshooting Guide](troubleshooting_guide.md)
- [Test Procedure](test_procedure.md)
- [Error Code Reference](error_code_reference.md)

## Scope

This user manual applies to the current fixture revision for `EOL_TestFixture_Final`.

Current revision notes:

- Digital Output 1 and Digital Output 2 are temporary automatic pass items in the present revision.
- The Buttons / LEDs stage requires operator interaction.
- Failed runs can still be exported after the system reaches `Fault_Screen`.

## Station Requirements

Before starting a run, make sure the following are available:

- Powered and assembled test fixture
- STM32 fixture firmware loaded
- ESP32 EOL test firmware loaded
- Correct DUT or simulated I/O connection
- Correct cable set and wiring
- Windows PC with export software available
- ST-LINK connection for result export
- Operator present for the full run, especially the Buttons / LEDs stage

For a run-by-run readiness check, use [Preflight Checklist](preflight_checklist.md).

## Normal Operating Sequence

The production path is the automatic test flow. On the start screen, select `Begin AUTO`.

Do not use `Begin MANUAL` for normal production testing unless engineering or maintenance has specifically instructed you to use it.

### High-Level Flow

| Screen or Stage | What Happens | Operator Action |
| --- | --- | --- |
| Start Screen | Entry point to automatic or manual modes | Select `Begin AUTO` for production testing |
| Comm / Reset | Communication, MCU reset, and fault check run automatically | Observe status and wait |
| Inputs | Digital and analog input checks run automatically | Observe progress and do not interrupt |
| Outputs | Digital outputs auto-pass in this revision; relay checks run automatically | Observe progress and do not interrupt |
| Buttons / LEDs | Button checks and LED visual confirmation occur | Press and release the highlighted button when prompted, then select `LED's GOOD` or `LED's BAD` |
| Test Complete | Run passed | Record the unit as passed and export the results |
| Fault Screen | Run failed | Record the failure information and export the results |

## Before Starting A Run

1. Power the fixture and confirm the display starts normally.
2. Verify the DUT and all required cables are connected correctly.
3. Confirm the ESP32 has the correct EOL test firmware loaded.
4. Confirm the export PC is available if the run will be archived.
5. Confirm the operator can remain at the station during the Buttons / LEDs stage.
6. Review [Preflight Checklist](preflight_checklist.md) if there is any doubt about station readiness.

## Running The Automatic Test

### Step 1 - Start The Test

On the start screen, press `Begin AUTO`.

This starts the production test flow through the automatic screens in this order:

1. `Comm_Reset_Screen`
2. `Inputs_Screen`
3. `Outputs_Screen`
4. `Buttons_LEDs_Screen`
5. `Test_Complete_Screen` or `Fault_Screen`

### Step 2 - Observe Automatic Stages

The following stages run automatically:

- Communication check
- MCU reset verification
- Fault check verification
- Digital inputs
- Analog inputs
- Outputs and relay validation

During these stages:

- allow the fixture to complete each stage
- watch for labels turning green as steps pass
- do not disconnect the DUT or cables
- do not restart the run unless the procedure specifically requires it

### Step 3 - Complete The Buttons / LEDs Stage

The Buttons / LEDs screen requires operator action.

When the active button label turns yellow:

1. Press the indicated button.
2. Release the same button within the allowed time.
3. Repeat for each prompted button.

After all button checks pass:

1. Observe the LED pattern.
2. Press `LED's GOOD` if the LED behavior is correct.
3. Press `LED's BAD` if the LED behavior is incorrect.

## Pass And Fail Handling

### If The Run Passes

If the fixture reaches `Test_Complete_Screen`:

1. Record the unit as passed.
2. Export the completed run.
3. Confirm the expected result file is created on the PC.

### If The Run Fails

If the fixture reaches `Fault_Screen`:

1. Record the displayed error code.
2. Record the failed test or step name.
3. Record the expected and actual values shown on the screen.
4. Do not bypass the failure or continue testing without correction.
5. Export the failed run according to normal process.
6. Use [Troubleshooting Guide](troubleshooting_guide.md) and [Error Code Reference](error_code_reference.md) to determine the next action.

## Exporting Results

After a completed pass or fail:

1. Open the export software on the PC.
2. Use `Extract Data from NUCLEO-U575ZI-Q`.
3. Retrieve the completed run from the STM32 over ST-LINK USART1.
4. Confirm a CSV file is created and stored in the expected export location.

If export fails:

- verify the run has reached a terminal pass or fail state
- verify the correct serial port is selected
- verify the ST-LINK connection is present
- review [Troubleshooting Guide](troubleshooting_guide.md)

## Operator Rules

- Use `Begin AUTO` for normal production testing.
- Stay at the fixture during the full run.
- Do not ignore communication or reset failures.
- Do not mark a unit as passed unless the fixture reaches `Test_Complete_Screen`.
- Do not remove a failed unit before required information has been recorded.
- Do not use the manual screen for production unless directed by engineering or maintenance.

## When To Use The Supporting Docs

- Use [Screen Guide](screen_guide.md) if you want a screen-by-screen explanation of what to expect.
- Use [Troubleshooting Guide](troubleshooting_guide.md) if a run fails and you need action-oriented recovery steps.
- Use [Error Code Reference](error_code_reference.md) if you need the exact meaning of a fault code.
- Use [Test Procedure](test_procedure.md) for the formal detailed procedure and acceptance criteria.
- Use [Preflight Checklist](preflight_checklist.md) before a shift or before a new run when setup is uncertain.
