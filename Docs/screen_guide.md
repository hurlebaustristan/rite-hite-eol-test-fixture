# EOL Test Fixture Screen Guide

## Purpose

This guide explains what the operator sees on each main screen in the EOL fixture user interface and what action, if any, is required.

Use this guide together with [User Manual](user_manual.md), [Troubleshooting Guide](troubleshooting_guide.md), and [Error Code Reference](error_code_reference.md).

## Screen Color Meaning

The screen colors are used as status indicators during the run.

| Color | Meaning |
| --- | --- |
| Black | Item is pending or not yet completed |
| Yellow | Operator action is required now |
| Green | Item or stage passed |
| Red | Item or stage failed |

## Start Screen

### Purpose

The start screen is the entry point to the fixture.

### What The Operator Does

- Select `Begin AUTO` for the normal production test sequence.
- Do not select `Begin MANUAL` for normal production unless engineering or maintenance has instructed you to use the manual service screen.

### What Happens Next

- `Begin AUTO` sends the fixture to `Comm_Reset_Screen`.
- `Begin MANUAL` sends the fixture to `Test_GPIO_Screen`, which is intended for manual or service use rather than standard operator workflow.

## Comm_Reset_Screen

### Purpose

This screen runs the first three automated stages:

- Communication
- MCU Reset
- Fault Check

### What The Operator Sees

- Stage labels begin in black.
- A passing stage turns green.
- A failing stage turns red before the system goes to `Fault_Screen`.

### What The Operator Does

- Observe the screen.
- Wait for the fixture to finish the sequence automatically.
- Do not disconnect the DUT or restart the run during this stage.

### Result

- If all three stages pass, the fixture moves to `Inputs_Screen`.
- If any stage fails, the fixture moves to `Fault_Screen`.

## Inputs_Screen

### Purpose

This screen runs:

- Digital input verification
- Analog input verification

### What The Operator Sees

- Digital input labels turn green as they pass.
- Analog input labels turn green as they pass.
- The progress bar advances through the stage.
- A failing item turns red and the fixture transfers to `Fault_Screen`.

### What The Operator Does

- Observe progress only.
- Do not interrupt the run while digital and analog checks are active.

### Result

- If all input checks pass, the fixture moves to `Outputs_Screen`.
- If any check fails, the fixture moves to `Fault_Screen`.

## Outputs_Screen

### Purpose

This screen runs the outputs stage.

### What The Operator Sees

- `Digital Outputs` and `Relay Outputs` are displayed as separate groups.
- Individual labels turn green as items pass.
- The progress bar advances through the stage.

### Current Revision Behavior

- Digital Output 1 and Digital Output 2 are automatic pass items in the current revision.
- Relay checks remain active and can still fail.

### What The Operator Does

- Observe progress only.
- Allow the screen to complete automatically.

### Result

- If all output checks pass, the fixture moves to `Buttons_LEDs_Screen`.
- If a relay phase fails, the fixture moves to `Fault_Screen`.

## Buttons_LEDs_Screen

### Purpose

This screen runs the final operator-assisted stage.

### What The Operator Sees

- The button labels start in black.
- The active button label turns yellow when it is ready to be pressed.
- A completed button turns green.
- During the LED visual check, the LED decision buttons become active.

### What The Operator Does

1. Watch for the yellow button label.
2. Press the indicated button.
3. Release the same button within the allowed time.
4. Repeat for all prompted buttons.
5. Observe the LED pattern when the visual check begins.
6. Press `LED's GOOD` if the LED behavior is correct.
7. Press `LED's BAD` if the LED behavior is not correct.

### Result

- If all button checks pass and `LED's GOOD` is selected, the fixture moves to `Test_Complete_Screen`.
- If there is a timeout, invalid reply, no reply, or operator rejection, the fixture moves to `Fault_Screen`.

## Fault_Screen

### Purpose

This is the terminal fail screen for the active run.

### What The Operator Sees

The screen shows:

- the hex error code
- the failed test or step name
- the expected value or condition
- the actual value or condition

### What The Operator Does

1. Record the error code.
2. Record the failed step.
3. Record the expected and actual values if required by local process.
4. Use [Troubleshooting Guide](troubleshooting_guide.md) and [Error Code Reference](error_code_reference.md).
5. Export the failed run.

## Test_Complete_Screen

### Purpose

This is the terminal pass screen for the active run.

### What The Operator Does

1. Record the unit as passed.
2. Export the completed run.
3. Confirm the CSV file was created.

## Test_GPIO_Screen

### Purpose

This is the manual or service screen reached from `Begin MANUAL`.

### Operator Guidance

- This screen is not part of the normal automatic production flow.
- Use it only when instructed by engineering, maintenance, or a debug procedure.
- Do not use it as a substitute for the automatic production test.

## Related Documents

- [User Manual](user_manual.md)
- [Troubleshooting Guide](troubleshooting_guide.md)
- [Preflight Checklist](preflight_checklist.md)
- [Error Code Reference](error_code_reference.md)
- [Test Procedure](test_procedure.md)
