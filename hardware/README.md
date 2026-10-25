# Hardware System Documentation & Assembly Guide

This folder contains the complete hardware specifications, schematics, and assembly instructions for the **E-Charger: Resonant Wireless EV Charging Prototype**.

---

## 📸 Physical Prototype Setup

The physical prototype integrates the resonant primary coil inside the road surface track and the secondary pick-up coil under the chassis of a test EV vehicle, monitored and controlled via an Arduino Uno system.

![Hardware Prototype Setup](../docs/assets/hardware-prototype-setup.png)

---

## 📑 Hardware Subdirectories & Docs
- **[`block-diagram.md`](file:///hardware/block-diagram.md):** Complete architectural block diagram, power flows, and subsystem descriptions.
- **[`components.md`](file:///hardware/components.md):** Full Bill of Materials (BOM) with electrical ratings and functional roles.

---

## 🛠 Step-by-Step Hardware Assembly Guide

### Step 1: Base Station (Primary Transmitter) Assembly
1. Mount the primary resonant coil flat onto the charging bay base plate.
2. Position the high-permeability ferrite sheet directly beneath the coil to channel the magnetic flux upward toward the vehicle.
3. Connect the primary coil to the high-frequency resonant inverter circuit (85-150 kHz).
4. Connect the power management unit (switching between the 12V Solar PV Panel and 12V Grid SMPS) to the inverter input terminals.

### Step 2: Vehicle Integration (Secondary Receiver) Assembly
1. Mount the secondary planar spiral coil on the underside of the EV vehicle chassis.
2. Maintain a target nominal air gap of **20 mm to 30 mm** above the transmitter surface.
3. Route the receiver coil leads into the high-speed Schottky diode bridge rectifier and smoothing LC filter.
4. Route the rectified DC output through the 5V relay contacts before connecting to the vehicle battery terminals.

### Step 3: Sensor & Controller Wiring
1. Mount the **Arduino Uno** onto the base station testing rig.
2. Connect the **IR alignment sensor** at the vehicle stopping threshold (facing forward/downward to detect vehicle bumper).
3. Connect the **Voltage divider sensor** in parallel across the vehicle battery terminals.
4. Connect the **16x2 LCD display** via I2C (`SDA` to `A4`, `SCL` to `A5`, `VCC` to `5V`, `GND` to `GND`).
5. Connect the **5V Relay module** signal pin to Digital Pin `7`.
6. Connect the **Charging status LED (Green)** to Pin `13` and **Standby LED (Red)** to Pin `6`.

---

## 📏 Coil Alignment & Spacing Guidelines

| Parameter | Nominal Value | Acceptable Range | Notes |
|-----------|---------------|------------------|-------|
| **Vertical Air Gap ($z$)** | 25 mm | 15 mm – 40 mm | Coupling drops sharply beyond 45 mm |
| **Lateral Misalignment ($\Delta x$)** | 0 mm | ±15 mm | High-efficiency charging zone |
| **Angular Misalignment ($\theta$)** | 0° | < 10° | Maintain parallel coil orientation |
| **Operating Frequency ($f$)** | 100 kHz | 85 kHz – 150 kHz | Tuned to SAE J2954 standard range |

---

## ⚠️ Electrical & Thermal Safety Protocols

1. **Galvanic Isolation:** The optical isolator on the relay module ensures total electrical isolation between the sensitive Arduino ATmega328P logic and the high-power charging rail.
2. **Reverse Current Protection:** Schottky blocking diodes prevent reverse discharge from the battery pack into the resonant receiver circuitry when wireless power is inactive.
3. **Overcharge Cutoff:** Hard-coded 14.1V / 80% SoC cutoff automatically disengages the relay to prevent over-voltage degradation of the battery.
4. **Thermal Dissipation:** Heat sinks must be mounted on the primary MOSFET inverter switches and secondary rectifiers during extended testing.
