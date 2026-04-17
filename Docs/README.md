# EOL Test Fixture Documentation

This folder contains the working documentation set for the Rite-Hite end-of-line test fixture.

The system has three major parts:

- An STM32U575-based fixture that runs the test flow and TouchGFX touchscreen UI
- An ESP32-S3 test target firmware that responds to the fixture protocol
- A Windows PySide6 desktop app that uploads test firmware, exports CSV results, and runs the One Click workflow

This documentation set is written for two audiences:

- Technicians and operators who need to run the station correctly and recover from failures
- Engineers who need to maintain the firmware, TouchGFX screens, serial protocol, and desktop app

## Start Here

- Operators: begin with [user_manual.md](user_manual.md)
- Engineers: begin with [Project_Knowledge_Base.md](Project_Knowledge_Base.md)

## Operator Documents

- [user_manual.md](user_manual.md): Primary station operating guide
- [preflight_checklist.md](preflight_checklist.md): Station readiness and shift-start checklist
- [screen_guide.md](screen_guide.md): Touchscreen and desktop-app screen reference
- [test_procedure.md](test_procedure.md): Formal production and One Click procedures
- [troubleshooting_guide.md](troubleshooting_guide.md): Operator recovery steps and station issues
- [error_code_reference.md](error_code_reference.md): Fault codes, protocol states, and export errors

## Engineering Documents

- [Project_Knowledge_Base.md](Project_Knowledge_Base.md): Architecture, protocol, hardware map, and stage behavior
- [C_Skills_Reference.md](C_Skills_Reference.md): Project-specific firmware and maintenance reference

## Current System Behavior Summary

- The fixture touchscreen supports `Begin AUTO` for the production flow and `Begin MANUAL` for the service GPIO screen.
- The automatic fixture flow runs in this order: Communication, MCU Reset, Fault Check, Digital Inputs, Analog Inputs, Outputs, Buttons, LED Visual Check.
- The fixture automatically drives the button sequence during the Buttons stage. The technician does not manually press the hardware buttons during the normal automatic flow.
- The final LED decision is still manual. The technician must press `LED's GOOD` or `LED's BAD` on the touchscreen.
- Failed runs remain exportable after the fixture reaches `Fault_Screen`.
- The desktop app has three working pages: `File Explorer`, `Device Tools`, and `One Click Test`.
- One Click uploads the Rite-Hite Connect test firmware, starts the fixture automatic test, tracks live progress, waits for the LED decision, and exports the final CSV automatically.
- The One Click progress rail follows the real fixture stage in this order: Upload Firmware, Comms, Digital Inputs, Analog Inputs, Digital Outputs, Relay Outputs, Buttons, LEDs, Extract Data.
- Digital Output 1 and Digital Output 2 are still temporary auto-pass rows in the current firmware revision.
- The desktop app shows a production firmware upload card, but the production upload action is not available yet.

## Documentation Update Rule

If firmware behavior, protocol tokens, screen flow, desktop app workflow, or station wiring changes, update these markdown files in the same change set. The documents in this folder are intended to match the current software exactly, not a planned future state.
