# End-of-Line Test Procedure

## Purpose

This document defines the standard end-of-line (EOL) functional test procedure for the Rite-Hite connected system test fixture implemented in this project. It is intended to provide a consistent, repeatable method for verifying communication, inputs, outputs, user-interface functions, and result export before a unit is released from test.

The procedure is written for use by technicians, engineers, and internal stakeholders who require a clear operational reference for executing the test and responding to failures.

## Related Documents

Use this procedure together with the following operator-facing references:

- [User Manual](user_manual.md)
- [Preflight Checklist](preflight_checklist.md)
- [Screen Guide](screen_guide.md)
- [Troubleshooting Guide](troubleshooting_guide.md)
- [Error Code Reference](error_code_reference.md)

## Scope

This procedure applies to the current fixture revision built around the following major elements:

- STM32 NUCLEO-U575ZI-Q host controller and TouchGFX user interface
- ESP32-S3 test firmware target
- Associated input, output, relay, and user-interface test hardware
- Windows-based export software used to retrieve completed test records from the STM32

This procedure covers the complete operational sequence from setup through result disposition. It also reflects the current software behavior of the fixture, including the following revision-specific conditions:

- Digital output verification is temporarily configured as an automatic pass because the final digital output hardware is not yet installed in the current fixture revision.
- The Buttons / LEDs stage currently expects manual button actuation by the operator within the allowed timeout window.

## Required Equipment / Prerequisites

Before beginning the test, confirm that the following items are available and operational.

| Item | Requirement |
| --- | --- |
| Test fixture | Powered, assembled, and free of visible damage |
| STM32 NUCLEO-U575ZI-Q | Programmed with the current fixture firmware |
| ESP32-S3 board | Programmed with the current EOL test firmware |
| DUT or simulated I/O interface | Connected to the fixture as required for the test |
| Power supplies | Stable and within the required operating range for the fixture and DUT |
| Interconnect cables | Correct cable set installed and verified before starting |
| Windows PC | Available for data export and, if required, ESP32 firmware upload |
| Export software | Present in `Export_Software` and operational |
| ST-LINK connection | Available for STM32 communication and result export over USART1 |
| ESP-Prog connection | Available if ESP32 test firmware upload is required |

Additional prerequisites:

- The operator shall verify that the correct fixture wiring is installed before starting a test cycle.
- The ESP32 test firmware shall be loaded prior to production testing. If firmware must be reloaded, use the approved upload function in the export software before starting the test.
- The operator shall remain present during the Buttons / LEDs stage because manual interaction is currently required.

## Test Procedure

The test shall be executed in the order shown below. Do not skip steps unless the procedure explicitly identifies the step as temporary or automatic for the current fixture revision.

### Step 1 - Pre-Test Setup

| Field | Description |
| --- | --- |
| **Objective** | Prepare the fixture, DUT, and operator workstation for a valid test run. |
| **Required action(s)** | 1. Power the fixture and confirm normal startup.<br>2. Verify that the DUT and all required cables are connected correctly.<br>3. Confirm that the ESP32 board is loaded with the EOL test firmware.<br>4. Confirm that the export software is available on the connected PC if results will be archived after the run.<br>5. Ensure the operator can observe the screen and access the manual buttons for the Buttons / LEDs stage. |
| **Expected outcome** | The fixture powers normally, connections are secure, and the operator can proceed to the Comm / Reset screen without setup-related warnings or obvious hardware faults. |
| **Error(s) or failure condition(s)** | Incorrect or loose cable connection; missing DUT connection; missing ESP32 firmware; unpowered hardware; PC/export path unavailable. |
| **Troubleshooting / corrective action** | Re-seat or replace cables, verify connector orientation, confirm the correct ESP32 firmware is loaded, restore fixture power, and verify the PC connection before starting the automated sequence. |

### Step 2 - Communication Check

| Field | Description |
| --- | --- |
| **Objective** | Verify UART communication between the STM32 host and the ESP32 target. |
| **Required action(s)** | 1. Allow the Comm / Reset screen to start the communication test automatically.<br>2. Observe the fixture while the STM32 sends a communication handshake to the ESP32.<br>3. Wait for the communication stage to complete before proceeding. |
| **Expected outcome** | The ESP32 returns the expected communication response and the communication stage passes without operator intervention. |
| **Error(s) or failure condition(s)** | `0x0101` no response to communication request; `0x0102` invalid communication reply. |
| **Troubleshooting / corrective action** | Verify STM32-to-ESP32 serial wiring, confirm the ESP32 is powered and running the correct test firmware, inspect cable continuity, and retry only after the communication path has been corrected. |

### Step 3 - MCU Reset Verification

| Field | Description |
| --- | --- |
| **Objective** | Confirm that the fixture can pulse the ESP32 reset path and that the ESP32 reconnects correctly afterward. |
| **Required action(s)** | 1. Allow the fixture to assert the reset pulse automatically.<br>2. Wait for the ESP32 recovery interval and reconnection check.<br>3. Observe whether the stage completes and advances automatically. |
| **Expected outcome** | The reset pulse is applied correctly, the ESP32 reconnects, and the expected handshake response is received after reset. |
| **Error(s) or failure condition(s)** | `0x0201` no reconnect after reset; `0x0202` invalid reply after reset; `0x0203` reset pulse verification failure. |
| **Troubleshooting / corrective action** | Check the reset drive circuit, confirm the reset line reaches the ESP32, verify the ESP32 firmware boots normally after reset, and inspect for hardware faults or wiring errors on the reset path. |

### Step 4 - Fault Check Verification

| Field | Description |
| --- | --- |
| **Objective** | Verify that the fixture fault-check pulse path operates correctly and that the ESP32 acknowledges the event. |
| **Required action(s)** | 1. Allow the fixture to pulse the fault-check output automatically.<br>2. Wait for the ESP32 acknowledgment and automatic stage completion. |
| **Expected outcome** | The fault pulse is generated, the ESP32 responds with the expected acknowledgment, and the stage passes. |
| **Error(s) or failure condition(s)** | `0x0301` no acknowledgment after fault pulse; `0x0302` invalid acknowledgment; `0x0303` fault pulse verification failure. |
| **Troubleshooting / corrective action** | Inspect the fault output path, verify the STM32 drive line and the receiving ESP32 logic, confirm wiring continuity, and retest after the pulse path is corrected. |

### Step 5 - Digital Inputs Test

| Field | Description |
| --- | --- |
| **Objective** | Verify the full digital input set by driving each channel through its required state transition and confirming the ESP32 detects it correctly. |
| **Required action(s)** | 1. Allow the Inputs screen to start the digital input sequence automatically.<br>2. Verify that each digital input channel completes its transition check and turns green on the screen.<br>3. Do not interrupt the sequence while the fixture steps through all digital inputs. |
| **Expected outcome** | Each digital input channel transitions from high to low as required, is acknowledged by the ESP32, and is shown as passed on the Inputs screen. |
| **Error(s) or failure condition(s)** | `0x0401` no response for a digital input test; `0x0402` invalid state transition reported by the ESP32; `0x0403` malformed or unexpected reply. |
| **Troubleshooting / corrective action** | Inspect the input cable set, confirm the correct digital input board is installed, verify the STM32 drive output reaches the target input, confirm ESP32 firmware response integrity, and retest the affected channel after correcting the wiring or hardware fault. |

### Step 6 - Analog Inputs Test

| Field | Description |
| --- | --- |
| **Objective** | Verify all analog input channels at the required low, mid, and high calibration points. |
| **Required action(s)** | 1. Allow the Inputs screen to advance automatically into the analog sequence after the digital inputs complete.<br>2. Observe each analog channel as the fixture applies the programmed setpoints.<br>3. Allow the system to measure and evaluate all analog points without interruption. |
| **Expected outcome** | Current-loop channels and voltage channels both measure within tolerance at all required test points, and each analog input passes on the screen. |
| **Error(s) or failure condition(s)** | `0x0501` no analog reply; `0x0502` malformed or mismatched analog reply; `0x0503` measured analog value outside tolerance. |
| **Troubleshooting / corrective action** | Verify analog wiring and channel routing, confirm the DAC/multiplexer path is functioning, inspect scaling and sense components, verify the ESP32 measurement path, and compare the measured value to the expected engineering target before retesting. |

### Step 7 - Outputs Test

| Field | Description |
| --- | --- |
| **Objective** | Verify the current fixture outputs stage, including temporary digital output handling and relay state validation. |
| **Required action(s)** | 1. Allow the Outputs screen to begin automatically after the Inputs stage completes.<br>2. Observe that Digital Output 1 and Digital Output 2 auto-pass in the current fixture revision.<br>3. Allow the relay sequence to run automatically for K1 and K2 in both commanded states.<br>4. Verify the screen advances only after the relay outputs complete successfully. |
| **Expected outcome** | Digital outputs auto-pass for the current revision, and relay contacts are verified in both ON and OFF states with correct NO/NC behavior. |
| **Error(s) or failure condition(s)** | `0x0601` no relay command acknowledgment; `0x0602` invalid relay acknowledgment; `0x0603` relay state mismatch. |
| **Troubleshooting / corrective action** | For relay failures, confirm relay wiring, verify contact readback polarity, inspect the relay drive path from the ESP32, and confirm that the associated output channels toggle as expected. For digital outputs, note that the current software behavior is a temporary automatic pass and does not validate final production DO hardware. |

### Step 8 - Buttons / LEDs Test

| Field | Description |
| --- | --- |
| **Objective** | Verify button detection and LED behavior using the current operator-assisted method. |
| **Required action(s)** | 1. Allow the Buttons / LEDs screen to begin automatically after the Outputs stage.<br>2. Observe the active button label. When a button is ready to be pressed, its text turns yellow.<br>3. Press and release the indicated button by hand within the allowed timeout window.<br>4. Repeat for `USR_0`, `PNL_0`, `PNL_1`, `PNL_2`, and `PNL_3` in sequence.<br>5. After all button labels turn green, visually inspect the rotating LED pattern.<br>6. Press `LED's GOOD` if the LED pattern is correct, or `LED's BAD` if the LED pattern is incorrect. |
| **Expected outcome** | Each button is detected on press and release within the allowed time, each label turns green after successful confirmation, the LED pattern runs during the visual check, and the operator records the LED result using the on-screen button. |
| **Error(s) or failure condition(s)** | `0x0701` no ESP32 acknowledgment during button or LED control; `0x0702` invalid reply during button or LED control; `0x0703` operator rejected LED visual check; `0x0704` manual press or release timeout. |
| **Troubleshooting / corrective action** | Confirm the correct button is being pressed when highlighted, verify the operator pressed and released the button within the allowed window, inspect the button wiring to the ESP32 I/O expander, confirm the LED rotation is active, and verify the ESP32 test firmware is current. |

### Step 9 - Test Result Review and Data Export

| Field | Description |
| --- | --- |
| **Objective** | Preserve the completed test record and confirm the final status of the unit. |
| **Required action(s)** | 1. If the test reaches the Test Complete screen, record the unit as passed.<br>2. If the test reaches the Fault screen, record the unit as failed and note the displayed error code and failed step.<br>3. Open the export software on the PC.<br>4. Use the `Extract Data from NUCLEO-U575ZI-Q` function to retrieve the completed run from the STM32 over ST-LINK USART1.<br>5. Confirm that a CSV file is created for the tested board and archived in the configured exports folder. |
| **Expected outcome** | One CSV file is generated for the completed test run, with one row per test item and a per-row pass/fail result. |
| **Error(s) or failure condition(s)** | No completed run available for export; export attempted while the test is still running; run already exported; serial port selection error; export communication failure. |
| **Troubleshooting / corrective action** | Confirm the run has reached a terminal pass or fail state, verify the ST-LINK virtual COM port is available, ensure the export software is connected to the correct port, and repeat the export only if the run has not already been marked as exported. |

## Pass/Fail Criteria

The unit shall be considered **PASS** only when all required test stages complete successfully and the operator records a good LED visual result.

The unit shall be considered **FAIL** if any of the following occur:

- Any automated stage returns an error code
- Any digital input, analog input, or relay verification step fails
- Any manual button press or release times out
- The operator selects `LED's BAD`
- The fixture transitions to the Fault screen at any point in the sequence

### Run-Level Acceptance Summary

| Stage | Pass Condition | Fail Condition |
| --- | --- | --- |
| Communication | Expected handshake reply received | No response or invalid reply |
| MCU Reset | Reset pulse verified and ESP32 reconnects | No reconnect, invalid reply, or pulse verification failure |
| Fault Check | Fault pulse verified and acknowledgment received | No acknowledgment, invalid reply, or pulse verification failure |
| Digital Inputs | All channels pass required high-to-low transition | Any channel returns no response, invalid transition, or invalid reply |
| Analog Inputs | All channels measure within tolerance at required setpoints | Any point returns no response, invalid reply, or out-of-range value |
| Outputs | Relay validation completes successfully; temporary DO auto-pass accepted for this revision | Any relay command or relay state verification fails |
| Buttons / LEDs | All buttons confirmed, operator approves LED behavior | Acknowledgment failure, timeout, or operator rejection |
| Export | Completed run archived successfully | Export unavailable or export communication failure |

## Notes / Warnings

- The current fixture software contains temporary revision behavior. Specifically, Digital Output 1 and Digital Output 2 are automatically passed because the final digital output hardware is not yet installed.
- The current Buttons / LEDs test requires manual operator interaction. The operator must remain at the fixture during that stage.
- A failed run can still be exported from the Fault screen after the fixture reaches a terminal failure state.
- Exported results are intended to provide traceable test evidence for each board tested. One CSV file is generated per completed run.
- Do not bypass communication or reset failures. These failures indicate a fundamental issue with fixture-to-target operation and invalidate downstream test results.
- If fixture wiring, test firmware, or hardware revision changes, this procedure shall be reviewed and updated before release to production use.
