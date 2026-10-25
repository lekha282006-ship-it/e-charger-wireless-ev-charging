# Hardware Components & Bill of Materials (BOM)

This document provides a comprehensive component breakdown and specifications for the **E-Charger: Resonant Wireless EV Charging Prototype**.

---

## 📋 Complete Bill of Materials (BOM)

| Item # | Component Name | Model / Part No. | Specification / Rating | Qty | Subsystem |
|--------|----------------|------------------|------------------------|-----|-----------|
| **1** | Microcontroller Board | Arduino Uno R3 | ATmega328P, 16 MHz, 5V Logic | 1 | Monitoring & Control |
| **2** | 16x2 Character LCD | 1602 LCD + PCF8574 | I2C Interface, 5V, Address 0x27 | 1 | Telemetry & Display |
| **3** | Relay Module | SRD-05VDC-SL-C | 1-Channel, 5V Coil, 10A 250VAC/30VDC | 1 | Power Cutoff / BMS |
| **4** | Wireless Power Tx Module | Custom / XKT-412 | 85-150 kHz, 12V Input, 1-2A | 1 | Primary Transmitter |
| **5** | Wireless Power Rx Module | Planar Spiral Coil | Resonant tuned, 12V Output, 1A | 1 | Secondary Pick-up |
| **6** | Voltage Sensor Module | Voltage Divider | Range: 0-25V DC (R1=30kΩ, R2=7.5kΩ) | 1 | Monitoring & Control |
| **7** | Current Sensor Module | ACS712-05B | Hall Effect, ±5A Range, 185 mV/A | 1 | Monitoring & Control |
| **8** | IR Proximity Sensor | LM393 / TCRT5000 | Active LOW, 3.3V-5V, 2-10 cm range | 1 | Vehicle Alignment |
| **9** | Solar Photovoltaic Panel | Monocrystalline PV | 12V-18V Voc, 10W - 20W Output | 1 | Renewable Generation |
| **10** | AC-DC Grid Adapter | SMPS Adapter | Input: 100-240V AC, Output: 12V 2A DC | 1 | Utility Grid Supply |
| **11** | Rechargeable EV Battery | 12V Lead-Acid / Li-Ion | 12V Nominal, 1.2Ah - 7Ah | 1 | EV Battery / Load |
| **12** | Model EV Chassis | 1:18 Scale Truck | Yellow chassis, coil mounting bay | 1 | Mechanical Test Bed |
| **13** | Status Indicator LEDs | 5mm Diffused LED | 1x Green (Charging), 1x Red (Standby) | 2 | Visual Telemetry |
| **14** | Current-Limiting Resistors | Metal Film | 220Ω, 0.25W | 2 | LED Protection |
| **15** | Fast-Recovery Rectifier | MBR10100 / 1N5822 | Schottky Barrier, 100V 10A, Low Vf | 1 | Secondary Rectifier |
| **16** | Filter Capacitors | Electrolytic + Ceramic | 1000µF 25V + 100nF Ceramic | 2 | DC Smoothing Filter |
| **17** | Prototyping Breadboard | 400-Tie Points | Half-size solderless breadboard | 1 | Circuit Assembly |
| **18** | Jumper Wires | Male-to-Male / Male-Female | 24 AWG, Multicolored 20cm | 1 set | Interconnects |

---

## 🔬 Component Details & Functional Roles

### 1. Arduino Uno R3 (ATmega328P)
- **Role:** Core brain of the prototype.
- **Key Specs:**
  - Clock Frequency: 16 MHz
  - Operating Voltage: 5V
  - Flash Memory: 32 KB
  - Analog Inputs: 6 (10-bit ADC, 0-1023 resolution)
  - Digital I/O Pins: 14 (with PWM on 6 pins)

### 2. 16x2 I2C Character LCD (PCF8574)
- **Role:** Displays real-time operational state and charging metrics.
- **Key Specs:**
  - Backlight: High-contrast green/yellow LED
  - Communication: I2C (SDA on `A4`, SCL on `A5`), saving 6 digital I/O lines
  - Contrast Adjustment: Built-in trimmer potentiometer

### 3. Songle SRD-05VDC-SL-C Relay Module
- **Role:** Physical isolation switch for the battery charging loop.
- **Safety Features:** Built-in flyback diode, optocoupler isolation, and LED trigger indicator.
- **Contact Ratings:** 10A at 250V AC / 10A at 30V DC.

### 4. Resonant Magnetic Inductive Coils (Tx & Rx)
- **Role:** Wireless power transfer across the air gap.
- **Coil Geometry:** Planar spiral multi-strand Litz wire to mitigate skin effect and proximity effect at 100 kHz.
- **Ferrite Shielding:** High-permeability MnZn/NiZn ferrite backing plates beneath the coils to direct magnetic flux lines and shield surrounding electronics.

### 5. Voltage Divider Sensor (0-25V)
- **Role:** Accurately steps down the 11.0V - 14.4V battery voltage into the 0-5V ADC range of the Arduino.
- **Transfer Equation:**
  $$V_{pin} = V_{battery} \times \frac{R_2}{R_1 + R_2} = V_{battery} \times \frac{7.5\text{ k}\Omega}{30\text{ k}\Omega + 7.5\text{ k}\Omega} = \frac{V_{battery}}{5}$$
  $$V_{battery} = V_{pin} \times 5.0$$

### 6. IR Proximity Alignment Sensor
- **Role:** Detects the presence and spatial alignment of the model EV over the transmitter pad before energizing the high-power coil, preventing wasted energy and foreign object heating.
- **Operation:** Emits modulated infrared light; returns `LOW` logic level when reflection threshold is reached.
