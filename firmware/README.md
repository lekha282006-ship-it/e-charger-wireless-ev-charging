# E-Charger Firmware Documentation

The **E-Charger Firmware** manages the operational cycle, real-time sensing, safety cutoff logic, and visual diagnostic reporting for the Wireless Resonant EV Charging prototype.

---

## 📋 Table of Contents
1. [Architecture Overview](#architecture-overview)
2. [Pin Connections](#pin-connections)
3. [Dependencies & Libraries](#dependencies--libraries)
4. [State Machine Logic](#state-machine-logic)
5. [Calibration & Environment Constants](#calibration--environment-constants)
6. [Flashing & Setup Guide](#flashing--setup-guide)
7. [Serial Telemetry Format](#serial-telemetry-format)

---

## 🛠 Architecture Overview

The firmware runs on an **ATmega328P microcontroller (Arduino Uno)**. It is structured around an asynchronous, non-blocking Finite State Machine (FSM) driven by `millis()` timers.

```
       [ Power-On / Setup ]
                 │
                 ▼
       ┌───────────────────┐
       │   STATE_STANDBY   │◄─────────────────────────────┐
       │ (Relay OFF, Green │                              │
       │  LED OFF, Red ON) │                              │
       └─────────┬─────────┘                              │
                 │ Vehicle detected &                     │
                 │ Battery < 80% (14.1V)                  │
                 ▼                                        │
    ┌─────────────────────────┐                           │
    │ STATE_CHARGING_STARTED  │                           │
    │ (Relay ON, Green LED ON)│                           │
    └────────────┬────────────┘                           │
                 │ Immediate                              │
                 ▼                                        │
       ┌───────────────────┐                              │
       │  STATE_CHARGING   │────── Vehicle misaligned ────┤
       │ (Closed-loop SoC% │                              │
       │  monitoring)      │                              │
       └─────────┬─────────┘                              │
                 │ Voltage >= 14.1V                       │
                 │ (SoC >= 80%)                           │
                 ▼                                        │
    ┌─────────────────────────┐                           │
    │  STATE_FULLY_CHARGED    │                           │
    │ (Relay Cutoff, Display) │                           │
    └────────────┬────────────┘                           │
                 │ Delay 5000ms                           │
                 ▼                                        │
       ┌───────────────────┐                              │
       │  STATE_RESETTING  │──────────────────────────────┘
       └───────────────────┘
```

---

## 🔌 Pin Connections

| Arduino Uno Pin | Component | Module Pin | Description |
|-----------------|-----------|------------|-------------|
| **A0** | Voltage Sensor | Signal (S) | Voltage divider analog reading (0-25V scale) |
| **A1** | Current Sensor | OUT | ACS712-05B Hall-effect current sensing |
| **A4 (SDA)** | 16x2 LCD (I2C) | SDA | I2C Data line |
| **A5 (SCL)** | 16x2 LCD (I2C) | SCL | I2C Clock line |
| **D2** | IR Proximity Sensor | OUT | Digital alignment detection (Active LOW) |
| **D7** | 5V Relay Module | IN | Optocoupler-isolated relay switch |
| **D13** | Charging Indicator | Anode (+) | Green LED (via 220Ω resistor) |
| **D6** | Standby Indicator | Anode (+) | Red/Yellow LED (via 220Ω resistor) |
| **5V** | VCC Rail | VCC / VDD | Power rail for sensors and LCD |
| **GND** | Ground Rail | GND | Common system ground |

---

## 📦 Dependencies & Libraries

Ensure the following libraries are installed in your Arduino IDE / environment:

| Library | Version Tested | Purpose |
|---------|----------------|---------|
| `Wire.h` | Built-in | Two-wire I2C protocol communication |
| `LiquidCrystal_I2C.h` | 1.1.2+ (Frank de Brabander) | Controls 16x2 LCD via PCF8574 I2C adapter |

### Installing via Arduino Library Manager:
1. Open **Arduino IDE** -> **Tools** -> **Manage Libraries...**
2. Search for `LiquidCrystal_I2C` by Frank de Brabander or Marco Schwartz.
3. Click **Install**.

---

## ⚙️ Calibration & Environment Constants

All key system thresholds can be fine-tuned in [`src/main.ino`](file:///firmware/src/main.ino):

```cpp
// ADC Reference Voltage (Measure across 5V and GND on your board)
const float ADC_VREF = 5.00;

// Voltage Divider Ratio: (R1 + R2) / R2
// Standard module: R1 = 30kΩ, R2 = 7.5kΩ -> 37.5 / 7.5 = 5.0
const float VOLTAGE_DIVIDER_RATIO = 5.00;

// Battery Cutoff Parameters (From Prototype Table 1)
const float BATTERY_V_EMPTY   = 11.00; // Empty baseline (0%)
const float BATTERY_V_FULL    = 14.40; // Full baseline (100%)
const float CUTOFF_VOLTAGE    = 14.10; // Trigger cutoff at 14.1V
const float CUTOFF_PERCENTAGE = 80.0;  // Cutoff at 80% SoC
```

---

## 🚀 Flashing & Setup Guide

### Method 1: Arduino IDE (GUI)
1. Connect the Arduino Uno to your workstation via USB cable (Type A to B).
2. Launch Arduino IDE.
3. Open `firmware/src/main.ino`.
4. Go to **Tools** -> **Board** -> Select **Arduino Uno**.
5. Go to **Tools** -> **Port** -> Select the active COM port (e.g., `COM3` on Windows or `/dev/ttyUSB0` on Linux).
6. Click the **Upload** button (Ctrl+U).

### Method 2: Arduino CLI (Command Line)
```bash
# Compile sketch
arduino-cli compile --fqbn arduino:avr:uno firmware/src

# Upload to Arduino on COM3
arduino-cli upload -p COM3 --fqbn arduino:avr:uno firmware/src
```

---

## 📊 Serial Telemetry Format

The firmware continuously broadcasts telemetry at **9600 baud rate** over the USB serial interface.

### CSV Header
```csv
Time(s),Voltage(V),Battery(%),Relay,Status
```

### Example Telemetry Stream:
```
0,11.8,45,OFF,Not Charging
30,12.1,48,ON,Charging Started
60,12.4,52,ON,Charging
90,12.7,56,ON,Charging
120,13.0,60,ON,Charging
150,13.3,65,ON,Charging
180,13.6,70,ON,Charging
210,13.9,75,ON,Charging
240,14.1,80,OFF,Charging Completed
```
