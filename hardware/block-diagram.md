# Hardware Block Diagram & System Architecture

This document outlines the complete electrical architecture and functional block diagram of the **E-Charger: Resonant Wireless EV Charging Prototype**.

---

## 📐 Functional Block Diagram

Below is the conceptual and functional block diagram representing the dual-source input, high-frequency resonant power conversion, magnetic coupling air gap, and the secondary rectification / battery management stage.

![System Block Diagram](../docs/assets/system-block-diagram.png)

---

## 🔄 Mermaid System Flowchart

```mermaid
graph TD
    classDef power fill:#fef3c7,stroke:#d97706,stroke-width:2px;
    classDef primary fill:#e0f2fe,stroke:#0284c7,stroke-width:2px;
    classDef coupling fill:#f0fdf4,stroke:#16a34a,stroke-width:2px,stroke-dasharray: 5 5;
    classDef secondary fill:#ede9fe,stroke:#7c3aed,stroke-width:2px;
    classDef control fill:#fef9c3,stroke:#ca8a04,stroke-width:2px;

    Grid["⚡ Utility Grid Supply\n(230V / 110V AC)"]:::power
    Solar["☀️ Solar PV Panel\n(12V-18V DC Renewable)"]:::power

    PMU["🔀 Power Management & Source Selection Unit\n(AC-DC Rectifier, DC-DC Buck-Boost)"]:::primary

    HFI["⚡ High-Frequency Inverter\n(85 kHz - 150 kHz Resonant Driver)"]:::primary

    TxCoil["🌀 Transmitter Resonant Coil\n(Primary LC Tank / Road Surface)"]:::primary

    AirGap{"🧲 Wireless Magnetic Coupling\n(Air Gap: 20mm - 40mm)"}:::coupling

    RxCoil["🌀 Receiver Resonant Coil\n(Secondary LC Tank / Vehicle Underside)"]:::secondary

    RectFilter["🔌 Rectifier & Filter Circuit\n(Fast-Recovery Diodes + LC Filter)"]:::secondary

    BMS["🛡️ Battery Management System (BMS)\n(Overvoltage, Overcurrent, Thermal Safety)"]:::secondary

    EVBattery["🔋 EV Battery / Load\n(12V Pack / Model EV Chassis)"]:::secondary

    MCU["🖥️ Monitoring & Control Unit\n(Arduino Uno ATmega328P + Sensors + 16x2 LCD + Relay)"]:::control

    %% Power Flow
    Grid --> PMU
    Solar --> PMU
    PMU --> HFI
    HFI --> TxCoil
    TxCoil ==>|Magnetic Flux| AirGap
    AirGap ==>|Induced EMF| RxCoil
    RxCoil --> RectFilter
    RectFilter --> BMS
    BMS --> EVBattery

    %% Feedback & Telemetry
    RectFilter -. Sensing .-> MCU
    BMS -. Telemetry .-> MCU
    EVBattery -. Voltage/SoC .-> MCU
    MCU -. Control Signal .-> PMU
    MCU -. Inverter PWM/Enable .-> HFI
    MCU -. Relay Cutoff (80% / 14.1V) .-> BMS
```

---

## 🔍 Subsystem Descriptions

### 1. Dual Energy Generation Stage
- **Utility Grid Supply:** Provides reliable, baseline AC power stepped down and converted via switched-mode AC-DC power conversion.
- **Solar Photovoltaic (PV) Panel:** Harvests renewable solar energy (12V-18V DC nominal). Provides priority green energy during daylight operation.

### 2. Power Management & Source Selection Unit (PMU)
- Automatically switches between Solar PV and Grid power based on irradiance availability.
- Incorporates a DC-DC buck-boost converter to maintain a regulated DC bus voltage feeding the inverter.

### 3. High-Frequency Resonant Inverter
- Transforms the DC bus into high-frequency alternating current ($85\text{ kHz} - 150\text{ kHz}$), compliant with the **SAE J2954** wireless power transfer standard.
- Drives the primary resonant tank with minimum switching losses via zero-voltage switching (ZVS).

### 4. Resonant Magnetic Coupling Stage
- **Transmitter Resonant Coil:** Embedded beneath the road surface / charging base station with series compensation capacitor ($C_p$).
- **Air Gap:** Transfers energy through oscillating near-field magnetic flux across an air gap of 20 mm – 40 mm.
- **Receiver Resonant Coil:** Mounted to the underside of the EV vehicle chassis with secondary compensation capacitor ($C_s$).

### 5. Secondary Rectification & Battery Management (BMS)
- **High-Speed Rectifier:** Fast recovery Schottky diode bridge with low forward voltage drop.
- **LC Low-Pass Filter:** Smooths high-frequency ripple into stable DC current.
- **Battery Management System (BMS):** Provides over-voltage protection (OVP), over-current protection (OCP), and manages charging profile.

### 6. Closed-Loop Monitoring & Control Unit
- **Microcontroller (Arduino Uno ATmega328P):** Central controller processing real-time sensor data, controlling the relay, and updating the I2C LCD.
- **Voltage Divider:** Scaled voltage sensing (0-25V) fed to analog pin `A0`.
- **ACS712 Current Sensor:** Measures instantaneous charging current.
- **IR Proximity Sensor:** Detects vehicle alignment over the primary coil.
- **5V Optoisolated Relay:** Isolates charging circuit when 80% SoC (14.1V) is attained.
- **16x2 Character LCD:** Displays live visual telemetry (`System Ready`, `Charging... Charge: 50%`, `Fully Charged`).
