# Prototype Testing & Experimental Results

This document presents empirical test data, charging performance curves, LCD diagnostic telemetry, and safety cutoff validation collected from the working prototype of the **E-Charger: Resonant Wireless EV Charging System**.

---

## 📋 Table of Contents
1. [Empirical Charging Parameters (Table 1)](#1-empirical-charging-parameters-table-1)
2. [LCD Telemetry & Display State Transitions](#2-lcd-telemetry--display-state-transitions)
3. [Voltage & State-of-Charge (SoC) Dynamics](#3-voltage--state-of-charge-soc-dynamics)
4. [Air Gap & Misalignment Performance Analysis](#4-air-gap--misalignment-performance-analysis)
5. [Automated Relay Cutoff & Safety Response Testing](#5-automated-relay-cutoff--safety-response-testing)

---

## 1. Empirical Charging Parameters (Table 1)

The table below reflects real-time telemetry gathered across a continuous 240-second charging evaluation using the prototype hardware (Arduino Uno + Voltage Sensor + 5V Relay Module + Resonant Wireless Coils).

![Table 1: Real-Time Charging Parameters](assets/charging-parameters-table.png)

### Table 1: Real-Time Charging Parameters of Proposed Prototype

| Time (s) | Battery Voltage (V) | Battery Percentage (%) | Relay Switch State | Charging Status | LCD Display Output |
|:--------:|:-------------------:|:----------------------:|:------------------:|:---------------:|:-------------------|
| **0** | 11.8 | 45% | **OFF** | Not Charging | `System Ready` / `Waiting...` |
| **30** | 12.1 | 48% | **ON** | Charging Started | `Charging...` / `Charge: 48%` |
| **60** | 12.4 | 52% | **ON** | Charging | `Charging...` / `Charge: 52%` |
| **90** | 12.7 | 56% | **ON** | Charging | `Charging...` / `Charge: 56%` |
| **120** | 13.0 | 60% | **ON** | Charging | `Charging...` / `Charge: 60%` |
| **150** | 13.3 | 65% | **ON** | Charging | `Charging...` / `Charge: 65%` |
| **180** | 13.6 | 70% | **ON** | Charging | `Charging...` / `Charge: 70%` |
| **210** | 13.9 | 75% | **ON** | Charging | `Charging...` / `Charge: 75%` |
| **240** | 14.1 | 80% | **OFF** | Charging Completed | `Fully Charged` / `Resetting...` |

> **Key Observation:** At $t = 240\text{ s}$, when the battery terminal voltage reaches **$14.1\text{ V}$** ($80\%$ capacity threshold), the Arduino microcontroller instantly triggers the relay into the **OFF** state, isolating the battery and preventing overcharge stress.

---

## 2. LCD Telemetry & Display State Transitions

The 16x2 I2C LCD provides continuous operational feedback to the driver/operator. Below are the verified display states recorded during prototype testing:

![LCD Display States](assets/lcd-display-states.png)

### State Breakdown:
1. **Standby State (Top Left):**
   ```
   System Ready    
   Waiting...      
   ```
   *System is powered on; vehicle is either approaching or not yet in charging position; relay is open (OFF).*

2. **Mid-Charge State (Top Right):**
   ```
   Charging...     
   Charge: 50%     
   ```
   *Vehicle detected by IR sensor; relay closed (ON); resonant inductive transfer active.*

3. **High-Charge State (Bottom Left):**
   ```
   Charging...     
   Charge: 80%     
   ```
   *Terminal voltage approaching upper safety threshold ($14.1\text{V}$).*

4. **Cutoff / Completed State (Bottom Right):**
   ```
   Fully Charged   
   Resetting...    
   ```
   *Overcharge cutoff triggered; relay immediately de-energized (OFF); system enters cool-down reset sequence.*

---

## 3. Voltage & State-of-Charge (SoC) Dynamics

```mermaid
xychart-beta
    title "Battery Terminal Voltage (V) vs. Time (s)"
    x-axis [0, 30, 60, 90, 120, 150, 180, 210, 240]
    y-axis "Voltage (V)" 11.0 --> 14.5
    line [11.8, 12.1, 12.4, 12.7, 13.0, 13.3, 13.6, 13.9, 14.1]
```

```mermaid
xychart-beta
    title "Battery State of Charge (%) vs. Time (s)"
    x-axis [0, 30, 60, 90, 120, 150, 180, 210, 240]
    y-axis "Battery SoC (%)" 40 --> 90
    bar [45, 48, 52, 56, 60, 65, 70, 75, 80]
```

### Dynamic Characteristics:
- **Average Charging Rate ($\frac{dV}{dt}$):** $\approx 0.01\text{ V/s}$ across constant current phase.
- **Relay Latency:** Cutoff command executed within $< 30\text{ ms}$ upon sampling $14.10\text{ V}$.
- **ADC Stability:** 10-sample rolling averaging successfully eliminated high-frequency switching ripple ($100\text{ kHz}$) from skewing readings.

---

## 4. Air Gap & Misalignment Performance Analysis

Coupling efficiency $\eta_{transfer}$ was characterized across various vertical gaps ($z$) and lateral offsets ($\Delta x$):

| Vertical Air Gap ($z$) | Lateral Offset ($\Delta x$) | Coupling Factor ($k$) | DC Transfer Efficiency ($\eta$) | Operational Status |
|:----------------------:|:--------------------------:|:--------------------:|:------------------------------:|:------------------:|
| **15 mm** | 0 mm | 0.38 | 84.5% | Optimal Coupling |
| **20 mm** | 0 mm | 0.34 | 82.1% | Nominal Working Point |
| **25 mm** | 0 mm | 0.29 | 79.4% | Nominal Prototype Clearance |
| **30 mm** | 0 mm | 0.23 | 74.8% | Acceptable Charging Zone |
| **35 mm** | 0 mm | 0.17 | 66.2% | Degraded Transfer |
| **40 mm** | 0 mm | 0.12 | 53.0% | Minimum Operational Limit |
| **25 mm** | **±10 mm** | 0.26 | 76.5% | Misalignment Tolerated |
| **25 mm** | **±20 mm** | 0.18 | 61.3% | Sub-Optimal Alignment |

---

## 5. Automated Relay Cutoff & Safety Response Testing

| Test ID | Trigger Scenario | Expected Response | Observed Response | Result |
|:-------:|------------------|-------------------|-------------------|:------:|
| **TR-01** | Battery voltage reaches 14.1V | Relay de-energizes immediately | Relay turned OFF within 28 ms | **PASS** |
| **TR-02** | Vehicle departs during active charge | IR sensor reads HIGH; Relay OFF | Charging aborted instantly | **PASS** |
| **TR-03** | Battery terminal disconnected | Open-circuit detection | Fault status flagged; Relay OFF | **PASS** |
| **TR-04** | Rapid voltage ripple on coil lines | Filter capacitors attenuate noise | ADC reading remains stable ±0.05V | **PASS** |
| **TR-05** | LCD display refresh under load | Non-blocking display update | No loop stalling; 1000ms cadence | **PASS** |
