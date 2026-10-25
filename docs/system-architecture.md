# System Architecture Specification

This document provides a comprehensive architectural specification for the **E-Charger: Resonant Wireless EV Charging Prototype**, detailing the power flow, magnetic coupling, control loops, and subsystem interfaces.

---

## 🏛 High-Level System Architecture

The system is organized into four interconnected domains:
1. **Primary Power Generation & Conversion Domain (Ground Station)**
2. **Resonant Magnetic Air-Gap Coupling Interface**
3. **Secondary Power Harvesting & Storage Domain (Vehicle Station)**
4. **Closed-Loop Monitoring, Protection & Telemetry Domain**

![System Architecture Block Diagram](assets/system-block-diagram.png)

---

## 📊 End-to-End Architectural Diagram

```mermaid
flowchart TD
    subgraph GROUND_STATION ["Ground Base Station (Primary Subsystem)"]
        Grid["⚡ Utility Grid Supply\n(230V / 110V AC)"]
        Solar["☀️ Solar Photovoltaic Panel\n(12V-18V DC Renewable)"]
        
        PMU["🔀 Power Management & Source Selection Unit\n• Dual Input Prioritization\n• AC-DC Rectifier\n• DC-DC Buck-Boost"]
        
        HFI["⚡ High-Frequency Resonant Inverter\n• Full-Bridge MOSFET Driver\n• 85 kHz - 150 kHz Operating Range\n• Zero-Voltage Switching (ZVS)"]
        
        TxCoil["🌀 Transmitter Resonant Coil\n• Litz Wire Planar Spiral Coil\n• Series Resonant Capacitor (Cp)\n• Ferrite Flux Concentrator"]
    end

    subgraph MAGNETIC_INTERFACE ["Wireless Magnetic Coupling Interface"]
        Flux["🧲 High-Frequency Alternating Magnetic Field\n• Air Gap: 20 mm - 40 mm\n• Coupling Coefficient: k ≈ 0.2 - 0.4\n• SAE J2954 Compliant Resonance"]
    end

    subgraph VEHICLE_STATION ["Vehicle Receiver & Power Storage Subsystem"]
        RxCoil["🌀 Receiver Resonant Coil\n• Secondary Planar Spiral Coil\n• Secondary Resonant Capacitor (Cs)\n• Under-Chassis Shielding"]
        
        Rect["🔌 High-Speed Rectifier & Filter Circuit\n• Fast Recovery Schottky Bridge\n• LC Low-Pass Smoothing Filter"]
        
        Relay["🔘 Safety Cutoff Relay Switch\n• Songle 5V Optoisolated\n• Hard Physical Disconnect at 80% / 14.1V"]
        
        BMS["🛡️ Battery Management System\n• Overvoltage Protection (OVP)\n• Undervoltage Protection (UVP)\n• Thermal Cutoff"]
        
        Battery["🔋 EV Battery Pack / Load\n• 12V Nominal Energy Storage"]
    end

    subgraph CONTROL_DOMAIN ["Monitoring, Control & Diagnostics Subsystem"]
        IR["📡 IR Proximity Alignment Sensor\n(Detects Vehicle Parking Alignment)"]
        VSense["📊 Voltage Divider Sensor\n(0-25V Range, Precision Resistors)"]
        CSense["⚡ ACS712 Current Sensor\n(Instantaneous Current Telemetry)"]
        
        MCU["🖥️ Central Microcontroller (Arduino Uno ATmega328P)\n• Finite State Machine (FSM)\n• 10-bit ADC Oversampling\n• Real-Time SoC Estimation\n• Relay Actuation Logic"]
        
        LCD["📺 16x2 Character I2C LCD Display\n• PCF8574 Backpack\n• System Ready / Charging / Full Status"]
        
        SerialOut["💻 USB Serial Telemetry Logger\n• 9600 Baud CSV Stream\n• Real-Time Diagnostics"]
    end

    %% Power Interconnections
    Grid --> PMU
    Solar --> PMU
    PMU --> HFI
    HFI --> TxCoil
    TxCoil ==>|Magnetic Induction| Flux
    Flux ==>|Induced EMF| RxCoil
    RxCoil --> Rect
    Rect --> Relay
    Relay --> BMS
    BMS --> Battery

    %% Sensing & Control Lines
    IR -->|Digital Alignment Pin 2| MCU
    Battery -.->|Terminal Voltage Pin A0| VSense
    VSense --> MCU
    Rect -.->|Current Pin A1| CSense
    CSense --> MCU
    MCU -->|Relay Control Pin 7| Relay
    MCU -->|I2C SDA / SCL| LCD
    MCU -->|UART TX/RX| SerialOut
    MCU -.->|Feedback Enable/PWM| HFI
    MCU -.->|Source Select Control| PMU
```

---

## 🔁 Finite State Machine (FSM) State Architecture

The microcontroller executes a non-blocking 5-state finite state machine that governs the charging lifecycle and ensures battery safety.

```mermaid
stateDiagram-v2
    [*] --> STATE_STANDBY: Power On / Setup Complete

    STATE_STANDBY --> STATE_CHARGING_STARTED: Vehicle Detected (IR LOW)\n& Battery < 80% (14.1V)
    
    note right of STATE_STANDBY
        LCD: "System Ready / Waiting..."
        Relay: OFF (Open)
        Green LED: OFF, Red LED: ON
    end note

    STATE_CHARGING_STARTED --> STATE_CHARGING: Relay Energized (Closed)\nGreen LED: ON, Red LED: OFF
    
    note right of STATE_CHARGING
        LCD: "Charging... / Charge: XX%"
        Continuous Voltage & SoC Sampling
    end note

    STATE_CHARGING --> STATE_STANDBY: Vehicle Misaligned / Removed\n(Emergency Abort, Relay Cutoff)
    
    STATE_CHARGING --> STATE_FULLY_CHARGED: Voltage >= 14.1V\nOR SoC >= 80% (Relay Cutoff)
    
    note right of STATE_FULLY_CHARGED
        LCD: "Fully Charged / Resetting..."
        Relay: OFF (Open)
        Green LED: OFF, Red LED: ON
    end note

    STATE_FULLY_CHARGED --> STATE_RESETTING: Timer >= 5000ms
    
    STATE_RESETTING --> STATE_STANDBY: Vehicle Undocks OR Cooldown Complete
```

---

## 🧩 Detailed Subsystem Breakdown

### 1. Primary Transmitter Subsystem
- **Dual Source Ingestion:** Accommodates high-reliability AC utility grid supply alongside clean renewable Solar PV power.
- **Power Management Unit (PMU):** Implements dynamic source selection prioritizing solar energy whenever solar irradiance exceeds the minimal operational threshold ($P_{solar} \ge P_{threshold}$); otherwise, seamlessly blends or switches to grid AC power.
- **High-Frequency Inverter:** High-speed MOSFET H-bridge switching at resonant frequency $f_0 \approx 100\text{ kHz}$. Zero-Voltage Switching (ZVS) minimizes turn-on switching losses.
- **Primary Resonant Tank:** Consists of transmitter inductance $L_p$ and series resonant compensation capacitor $C_p$, establishing resonant frequency:
  $$f_0 = \frac{1}{2\pi \sqrt{L_p C_p}}$$

### 2. Resonant Inductive Air-Gap Channel
- Energy is transferred solely through magnetic near-field flux lines without galvanic connection.
- Ferrite plates behind the coils shape the field geometry, reducing stray electromagnetic interference (EMI) and boosting magnetic coupling factor $k$ ($0.2 \le k \le 0.4$ across a $20\text{ mm} - 40\text{ mm}$ air gap).

### 3. Secondary Vehicle Pick-Up Subsystem
- **Receiver Resonant Coil:** Matches resonant tank frequency:
  $$f_s = \frac{1}{2\pi \sqrt{L_s C_s}} \approx f_0$$
- **Fast-Recovery Rectification:** High-speed Schottky diode bridge converts the $100\text{ kHz}$ alternating voltage into pulsating DC.
- **LC Low-Pass Filter:** Suppresses high-frequency harmonics, generating clean DC for charging.
- **Safety Cutoff Relay:** Physically disconnects the battery from the charging circuit upon reaching the 80% (14.1V) cutoff threshold to prevent overcharge stress.

### 4. Monitoring & Telemetry Subsystem
- **Arduino Uno ATmega328P:** Samples battery voltage every $1000\text{ ms}$ using a 10-sample rolling average to filter noise.
- **I2C 16x2 LCD:** Updates vehicle operator with real-time status:
  - `System Ready / Waiting...`
  - `Charging... / Charge: XX%`
  - `Fully Charged / Resetting...`
- **UART Serial Diagnostics:** Outputs CSV data stream at $9600\text{ baud}$ for external datalogging and verification.
