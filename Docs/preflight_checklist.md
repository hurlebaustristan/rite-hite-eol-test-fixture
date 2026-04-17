# EOL Test Fixture Preflight Checklist

## Purpose

Use this checklist before starting a shift, after fixture setup changes, or any time station readiness is uncertain.

For the full workflow, see [User Manual](user_manual.md).

## Run Information

| Field | Entry |
| --- | --- |
| Date | |
| Operator | |
| DUT / Unit ID | |
| Fixture ID | |
| Notes | |

## Fixture And Station Readiness

- [ ] Fixture is powered and starts normally.
- [ ] Display is visible and responsive.
- [ ] No visible hardware damage is present.
- [ ] Required power supplies are present and stable.
- [ ] DUT or simulator is available for the run.

## Wiring And Connections

- [ ] Correct cable set is installed.
- [ ] DUT connections are secure.
- [ ] STM32 fixture connections are secure.
- [ ] ESP32 connections are secure.
- [ ] ST-LINK connection is available for export.
- [ ] No loose, damaged, or misrouted wiring is visible.

## Firmware And Software

- [ ] STM32 fixture firmware is loaded.
- [ ] ESP32 EOL test firmware is loaded.
- [ ] Export PC is available.
- [ ] Export software is available and opens normally.
- [ ] Correct serial connection is available for export.

## Operator Readiness

- [ ] Operator understands that normal production runs use `Begin AUTO`.
- [ ] Operator understands that `Begin MANUAL` is not for normal production use.
- [ ] Operator can remain present during the Buttons / LEDs stage.
- [ ] Operator knows how to record pass, fail, and export results.

## Current Revision Reminders

- [ ] Digital Output 1 and Digital Output 2 are temporary automatic pass items in the current revision.
- [ ] Buttons / LEDs requires manual operator interaction.
- [ ] Failed runs remain exportable after `Fault_Screen`.

## Ready To Start

- [ ] All required checks above are complete.
- [ ] The fixture is ready for a production run.
- [ ] `Begin AUTO` will be used to start the test.

## If Any Item Is Not Ready

- Stop before starting the run.
- Correct the setup issue first.
- Use [Troubleshooting Guide](troubleshooting_guide.md) if needed.
- Escalate to maintenance or engineering if the issue cannot be corrected at the operator level.

## Related Documents

- [User Manual](user_manual.md)
- [Screen Guide](screen_guide.md)
- [Troubleshooting Guide](troubleshooting_guide.md)
- [Test Procedure](test_procedure.md)
