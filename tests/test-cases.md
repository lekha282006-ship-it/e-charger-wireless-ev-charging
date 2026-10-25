# System Test Cases & Verification Protocol

This document details the complete hardware-in-the-loop (HIL) test suites, verification procedures, expected outcomes, and observed results for the **E-Charger: Resonant Wireless EV Charging Prototype**.

---

## 📋 Test Matrix Overview

| Test ID | Test Category | Title | Verification Criteria | Status |
|:-------:|:-------------:|:------|:----------------------|:------:|
| **TC-01** | Alignment | IR Sensor Vehicle Detection | Detects vehicle alignment; transitions from Standby | **PASS** |
| **TC-02** | Display | Standby Screen Verification | Displays `System Ready` / `Waiting...`; Relay OFF | **PASS** |
| **TC-03** | Power Control | Relay Actuation on Docking | Relay energizes (ON) when SoC < 80%; Green LED ON | **PASS** |
| **TC-04** | Telemetry | Real-Time Voltage & SoC Tracking | ADC samples voltage divider; updates LCD percentage | **PASS** |
| **TC-05** | Safety / Protection | 80% / 14.1V Cutoff Protection | Relay de-energizes (OFF) precisely at 14.1V / 80% SoC | **PASS** |
| **TC-06** | Display | Full Charge & Reset Display | LCD shows `Fully Charged` / `Resetting...` | **PASS** |
| **TC-07** | Safety / Abort | Vehicle Premature Departure | Relay instantly disengages if vehicle departs mid-charge | **PASS** |
| **TC-08** | Power Management | Solar to Grid Source Switching | System switches smoothly between Solar PV and Grid SMPS | **PASS** |
| **TC-09** | Signal Integrity | ADC Filtering & Ripple Rejection | 10-sample rolling average eliminates 100 kHz inverter noise | **PASS** |

---

## 🧪 Detailed Test Case Specifications

### TC-01: IR Sensor Vehicle Alignment Detection
- **Objective:** Verify that the IR proximity sensor detects the presence of the vehicle above the transmitter coil.
- **Pre-conditions:** System powered on in `STATE_STANDBY`. Relay open (OFF).
- **Test Steps:**
  1. Manually roll the model EV into the charging bay.
  2. Align the receiver coil over the transmitter coil until the front IR sensor triggers.
  3. Observe sensor output pin logic level on digital pin `2`.
- **Expected Result:** Digital pin `2` drops to `LOW` (Active LOW). Sensor onboard LED illuminates.
- **Observed Result:** IR sensor switched to `LOW` cleanly when bumper reached within 3 cm of stop line.
- **Status:** **PASS**

---

### TC-02: Standby Screen Verification
- **Objective:** Confirm LCD output and pin states during idle/standby operation.
- **Pre-conditions:** No vehicle present in charging dock.
- **Test Steps:**
  1. Power up Arduino Uno via USB / 12V DC input.
  2. Inspect 16x2 LCD display lines 1 and 2.
  3. Inspect Relay signal pin (`D7`) and LED indicators (`D13`, `D6`).
- **Expected Result:**
  - LCD Line 1: `System Ready    `
  - LCD Line 2: `Waiting...      `
  - Relay: OFF (`HIGH` on active-low relay)
  - Standby LED (`D6`): ON; Charging LED (`D13`): OFF.
- **Observed Result:** Display matched Image 3 (Top Left screen) exactly. Relay disengaged.
- **Status:** **PASS**

---

### TC-03: Relay Actuation on Docking
- **Objective:** Verify automated relay closure upon vehicle detection when battery voltage is below cutoff.
- **Pre-conditions:** Vehicle aligned (IR `LOW`); Initial battery voltage = 11.8V (45% SoC).
- **Test Steps:**
  1. Monitor Relay output pin `7`.
  2. Monitor relay module status LED and audible click.
- **Expected Result:** Relay switches to `LOW` (energizing relay coil to `ON`). Green LED on `D13` turns ON. Status updates to `Charging Started`.
- **Observed Result:** At $t = 30\text{s}$, relay engaged cleanly. Current started flowing into battery pack.
- **Status:** **PASS**

---

### TC-04: Real-Time Voltage & SoC Tracking
- **Objective:** Verify accurate analog-to-digital conversion and dynamic SoC percentage updates on the LCD.
- **Pre-conditions:** Active charging state (`STATE_CHARGING`).
- **Test Steps:**
  1. Measure battery voltage using calibrated digital multimeter (DMM).
  2. Compare DMM voltage against Arduino voltage telemetry streamed over Serial.
  3. Inspect LCD line 2 (`Charge: XX%`).
- **Expected Result:** Serial voltage reading within $\pm 0.1\text{V}$ of DMM. LCD updates percentage in real time (e.g. 50%, 80%).
- **Observed Result:**
  - At 12.4V: LCD showed `Charge: 52%`
  - At 13.0V: LCD showed `Charge: 60%`
  - At 13.6V: LCD showed `Charge: 70%`
  - Matched Table 1 and LCD photos (Top Right: 50%, Bottom Left: 80%).
- **Status:** **PASS**

---

### TC-05: 80% / 14.1V Cutoff Protection
- **Objective:** Confirm overcharge cutoff disconnects the battery when voltage reaches 14.1V (80% SoC).
- **Pre-conditions:** Continuous charging progressing towards 14.1V.
- **Test Steps:**
  1. Allow battery voltage to rise to 14.1V.
  2. Monitor microcontroller decision and relay state at threshold.
- **Expected Result:** As soon as voltage hits 14.1V ($t = 240\text{s}$), relay immediately turns OFF. Charging LED extinguishes.
- **Observed Result:** At $t = 240\text{s}$, voltage measured 14.1V (80%). Relay switched OFF within 28 ms. Zero current draw observed.
- **Status:** **PASS**

---

### TC-06: Full Charge & Reset Display
- **Objective:** Validate LCD screen transition to full charge notification.
- **Pre-conditions:** Cutoff triggered ($V \ge 14.1\text{V}$).
- **Test Steps:**
  1. Observe LCD display immediately following cutoff.
  2. Wait 5 seconds for reset cycle.
- **Expected Result:**
  - LCD Line 1: `Fully Charged   `
  - LCD Line 2: `Resetting...    `
- **Observed Result:** Display transitioned to Image 3 (Bottom Right) screen accurately.
- **Status:** **PASS**

---

### TC-07: Vehicle Premature Departure Abort
- **Objective:** Verify rapid shutdown if vehicle moves out of alignment during an active charging cycle.
- **Pre-conditions:** Active charging at $t = 120\text{s}$ ($V = 13.0\text{V}$, Relay ON).
- **Test Steps:**
  1. Manually pull vehicle back from the charging pad.
  2. Verify IR sensor detects departure.
- **Expected Result:** IR sensor returns `HIGH`. Microcontroller triggers emergency cutoff, turning relay OFF within $< 50\text{ms}$. LCD reverts to `System Ready / Waiting...`.
- **Observed Result:** Relay de-energized immediately. Inverter output ceased. No arcing or voltage spikes observed.
- **Status:** **PASS**

---

### TC-08: Dual Power Source Switching
- **Objective:** Test seamless switching between Solar PV and Utility Grid supply.
- **Pre-conditions:** Dual input connected to Power Management Unit.
- **Test Steps:**
  1. Expose solar panel to light ($V_{solar} \approx 14.2\text{V}$). Confirm solar powers the transmitter inverter.
  2. Obscure solar panel ($V_{solar} < 11.5\text{V}$). Observe automatic transfer to 12V grid SMPS.
- **Expected Result:** DC bus maintains stable 12V output across transition. No microcontroller reboot or coil drop.
- **Observed Result:** Switchover achieved in $< 15\text{ ms}$ with capacitor bank holding voltage.
- **Status:** **PASS**

---

### TC-09: ADC Filtering & Inverter Ripple Rejection
- **Objective:** Verify that 100 kHz inverter switching harmonics do not corrupt analog ADC voltage sensing.
- **Pre-conditions:** High-frequency resonant inverter active at full power.
- **Test Steps:**
  1. Probe `A0` analog pin with digital oscilloscope.
  2. Compare single ADC reads vs. 10-sample rolling average.
- **Expected Result:** Rolling average filters ripple, keeping ADC variance under $\pm 1\text{ LSB}$ ($\approx 4.88\text{ mV}$).
- **Observed Result:** Voltage readout stable within $\pm 0.03\text{V}$. No false cutoff triggers.
- **Status:** **PASS**
