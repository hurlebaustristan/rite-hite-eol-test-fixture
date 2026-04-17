# EOL Test Fixture Preflight Checklist

Use this checklist at shift start, after maintenance, after a firmware change, after a wiring change, or any time station readiness is uncertain.

For detailed operating instructions, see [user_manual.md](user_manual.md).

## Station Record

| Field | Entry |
| --- | --- |
| Date | |
| Operator / Technician | |
| Fixture ID | |
| DUT or Sample Board ID | |
| PC Name | |
| Notes | |

## Fixture Readiness

- [ ] Fixture powers on normally.
- [ ] Touchscreen is visible and responds to touch.
- [ ] `Begin AUTO` and `Begin MANUAL` are visible on the start screen.
- [ ] No loose wiring, damaged connectors, or visible hardware damage are present.
- [ ] The DUT, harness, or simulation wiring for this run is installed correctly.
- [ ] Any required bench power supplies are on and stable.

## Firmware And Software Readiness

- [ ] STM32 fixture firmware is loaded and the fixture reaches the normal start screen.
- [ ] The Rite-Hite Connect module has the correct test firmware loaded, or the programmer is available to load it from the desktop app.
- [ ] The Windows export app opens normally.
- [ ] The desktop app language, folder path, and settings are acceptable for the shift.
- [ ] The exports folder is reachable and writable.

## Connection Readiness

### Test Fixture / ST-LINK Path

- [ ] The fixture USB or ST-LINK serial connection is available to the PC.
- [ ] The desktop app can detect the fixture serial port, or the technician knows which ST-LINK COM port to select manually.

### Programmer Path

- [ ] The Rite-Hite Connect programmer cable is connected if firmware upload is required.
- [ ] The desktop app shows the programmer as detected before attempting `Upload Test Firmware` or `Start One Click Test`.

## Workflow Selection

Choose the workflow that will be used for this station cycle.

- [ ] Standard fixture workflow: start the run from the touchscreen and export with `Device Tools` after pass or fail.
- [ ] One Click workflow: use the desktop app to upload test firmware, start the fixture automatically, track progress, and export the CSV automatically.

## Operator Reminders

- [ ] `Begin AUTO` is the normal production path on the fixture.
- [ ] `Begin MANUAL` is the service GPIO screen and is not part of normal production testing.
- [ ] The fixture automatically drives the button sequence during the Buttons stage.
- [ ] The technician must still make the LED visual decision by pressing `LED's GOOD` or `LED's BAD` on the fixture touchscreen.
- [ ] Failed runs can still be exported after `Fault_Screen`.

## Current Revision Notes

- [ ] Digital Output 1 and Digital Output 2 are temporary auto-pass report rows in the current revision.
- [ ] The desktop app production firmware upload card is present, but the production upload action is not available yet.
- [ ] One Click locks out manual extract and manual test-firmware upload while the One Click run is active.

## Ready To Start

- [ ] All checks above are complete.
- [ ] The DUT or sample board is connected correctly.
- [ ] The selected workflow is understood by the operator.
- [ ] The station is ready for a production run.

## If Anything Is Not Ready

- Stop before starting the run.
- Correct the setup issue first.
- Use [troubleshooting_guide.md](troubleshooting_guide.md) if needed.
- Escalate to maintenance or engineering if the issue is outside normal operator recovery.
