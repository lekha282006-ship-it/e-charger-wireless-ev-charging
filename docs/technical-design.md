# Technical Design Document: Resonant Wireless EV Charging System

This document provides the mathematical principles, electrical circuit design, coil design parameters, and control algorithms for the **E-Charger: Resonant Wireless EV Charging Prototype**.

---

## 📑 Table of Contents
1. [Theoretical Principles of Magnetic Resonant Coupling](#1-theoretical-principles-of-magnetic-resonant-coupling)
2. [Dual-Source Power Management & Inverter Design](#2-dual-source-power-management--inverter-design)
3. [Resonant Coil Geometry & Magnetic Coupling](#3-resonant-coil-geometry--magnetic-coupling)
4. [Secondary Power Conditioning & BMS Stage](#4-secondary-power-conditioning--bms-stage)
5. [Microcontroller Control & Safety Protection Algorithms](#5-microcontroller-control--safety-protection-algorithms)
6. [Efficiency Analysis & Loss Budget](#6-efficiency-analysis--loss-budget)

---

## 1. Theoretical Principles of Magnetic Resonant Coupling

Traditional inductive power transfer (IPT) relies on tightly coupled magnetic cores (such as transformers) with coupling factors $k \approx 0.95 - 0.99$. In wireless electric vehicle charging, large air gaps ($20\text{ mm} - 150\text{ mm}$) reduce coupling to $k \approx 0.15 - 0.40$, resulting in high leakage inductance and reactive power draw.

To overcome this, **Magnetic Resonant Coupling (MRC)** introduces tuned reactive compensation networks on both the primary (transmitter) and secondary (receiver) sides.

```
       Primary Transmitter                       Secondary Receiver
  ┌───────────┐    Cp          Lp           Ls          Cs    ┌───────────┐
  │  HF AC    ├───┤├───┬─────█████───┐  ┌───█████─────┬───┤├───┤  Load /   │
  │  Source   │        │             │  │             │       │  Battery  │
  └───────────┘        └───/\/\/\────┘  └───/\/\/\────┘       └───────────┘
                           Rp    (Air Gap)      Rs
```

### 1.1 Resonant Frequency Condition
Both primary and secondary tanks are tuned to resonate at an identical operating frequency $f_0$:

$$\omega_0 = 2\pi f_0 = \frac{1}{\sqrt{L_p C_p}} = \frac{1}{\sqrt{L_s C_s}}$$

Where:
- $L_p, L_s$: Self-inductances of primary and secondary coils ($\mu\text{H}$)
- $C_p, C_s$: Series resonant compensation capacitors ($\text{nF}$)
- $f_0$: Resonant frequency ($100\text{ kHz}$ nominal, compliant with **SAE J2954** standard $85 - 150\text{ kHz}$)

### 1.2 Mutual Inductance & Quality Factor
Mutual inductance $M$ between coils across the spatial gap is given by:

$$M = k \sqrt{L_p L_s}$$

The unconstrained Quality Factor $Q$ represents the ratio of stored energy to dissipated energy per cycle:

$$Q_p = \frac{\omega_0 L_p}{R_p}, \quad Q_s = \frac{\omega_0 L_s}{R_s}$$

Where $R_p, R_s$ denote internal AC winding resistances. High $Q$ values ($Q > 80$) compensate for low magnetic coupling $k$, enabling high transfer efficiencies ($>85\%$).

---

## 2. Dual-Source Power Management & Inverter Design

The prototype incorporates an intelligent energy arbitration stage accepting both **Utility Grid AC Supply** and a **Renewable Solar Photovoltaic (PV) Panel**.

```
  ┌──────────────────┐
  │ Utility Grid AC  ├───►[ AC-DC Rectifier ]──┐
  └──────────────────┘                         │
                                               ▼
  ┌──────────────────┐               ┌────────────────────┐      ┌─────────────────┐
  │  Solar PV Panel  ├───►[ MPPT DC-DC]──►[ Source Selection ├──►[ HF Full-Bridge ]
  │  (12V - 18V DC)  │               │    Switching Unit  │      │    Inverter     │
  └──────────────────┘               └────────────────────┘      └────────┬────────┘
                                                                          │
                                                                   To Primary Coil
```

### 2.1 Source Prioritization Algorithm
- **Priority 1 (Solar PV Active):** If solar irradiance generates voltage $V_{solar} \ge 13.5\text{V}$, the Power Management Unit (PMU) connects solar power directly to the internal DC bus.
- **Priority 2 (Grid Fallback):** If $V_{solar} < 13.5\text{V}$ (cloud cover, nighttime), the PMU switches to the rectified utility grid power supply (12V DC regulated).

### 2.2 High-Frequency Full-Bridge Inverter
- **Topology:** Four N-channel power MOSFETs arranged in an H-bridge configuration.
- **Switching Profile:** High-frequency complementary PWM with built-in dead-time ($200\text{ ns}$) to eliminate shoot-through currents.
- **Zero-Voltage Switching (ZVS):** Inductive reactance is tuned slightly above resonance ($\omega > \omega_0$) to achieve zero-voltage turn-on, virtually eliminating capacitive turn-on switching losses.

---

## 3. Resonant Coil Geometry & Magnetic Coupling

The coils are fabricated as **planar circular spiral windings** utilizing high-frequency **Litz wire** (multi-strand insulated wire) to mitigate high-frequency **skin effect** and **proximity effect**.

### 3.1 Skin Depth Calculation
At $f_0 = 100\text{ kHz}$ in copper ($\rho = 1.68 \times 10^{-8} \ \Omega\cdot\text{m}$):

$$\delta = \sqrt{\frac{\rho}{\pi f \mu}} = \sqrt{\frac{1.68 \times 10^{-8}}{\pi \times 10^5 \times 4\pi \times 10^{-7}}} \approx 0.206\text{ mm}$$

Litz wire consisting of multiple $0.1\text{ mm}$ (AWG 38) strands ensures strand radius $r < \delta$, guaranteeing uniform current density.

### 3.2 Ferrite Core Flux Shielding
- High-permeability MnZn ferrite sheets ($\mu_r \approx 2000$) are bonded to the outer surfaces of both transmitter and receiver coils.
- **Benefits:**
  1. Increases coupling factor $k$ by approximately $25 - 35\%$.
  2. Confines magnetic flux lines to the core path, shielding the vehicle chassis from parasitic eddy current heating.
  3. Reduces electromagnetic leakage to meet ICNIRP public safety thresholds.

---

## 4. Secondary Power Conditioning & BMS Stage

The AC voltage induced in the vehicle receiver coil is converted into battery-compatible DC power through three distinct stages:

```
  [ Secondary Coil ] ──► [ Schottky Bridge Rectifier ] ──► [ LC Filter ] ──► [ Relay ] ──► [ Battery ]
```

1. **High-Speed Schottky Bridge Rectification:**
   - Employs low forward drop Schottky diodes ($V_f \approx 0.35\text{V}$) to minimize rectification losses at high frequencies ($100\text{ kHz}$).
2. **LC Low-Pass Filter:**
   - Cutoff frequency designed at $f_c \approx 5\text{ kHz}$ ($f_c \ll f_0$) using a $100\mu\text{H}$ power choke and low-ESR electrolytic capacitors to eliminate voltage ripple.
3. **Safety Isolation Relay:**
   - Songle SRD-05VDC-SL-C provides galvanic cut-off directly controlled by the Arduino microcontroller.

---

## 5. Microcontroller Control & Safety Protection Algorithms

The Arduino Uno ATmega328P monitors operational parameters and implements the safety cutoff profile.

### 5.1 Real-Time State of Charge (SoC) Calculation
Battery terminal voltage is sampled via a 5:1 precision voltage divider ($R_1 = 30\text{k}\Omega, R_2 = 7.5\text{k}\Omega$):

$$V_{battery} = V_{ADC} \times \left(\frac{R_1 + R_2}{R_2}\right) = \left(\frac{ADC\_VALUE \times 5.00}{1023}\right) \times 5.00$$

SoC percentage is calculated using dynamic piecewise linear mapping across the active charging band:

$$\text{SoC}(\%) = \left(\frac{V_{battery} - 11.0\text{V}}{14.4\text{V} - 11.0\text{V}}\right) \times 100\%$$

### 5.2 Automated Overcharge Protection & Cutoff
As validated in the physical prototype test data:
- **Relay Activation:** Engages when vehicle alignment is verified via the IR sensor and initial battery voltage is below the cutoff threshold ($V_{battery} < 14.1\text{V}$ / $\text{SoC} < 80\%$).
- **Relay Cutoff:** The moment $V_{battery} \ge 14.10\text{V}$ ($\text{SoC} = 80\%$), the microcontroller de-energizes the relay coil ($t_{response} < 50\text{ ms}$), completely isolating the battery.
- **Hysteresis Protection:** Charging will not re-arm until voltage decays below $13.80\text{V}$ ($\Delta V = 0.30\text{V}$ hysteresis) to prevent relay contact chatter.

---

## 6. Efficiency Analysis & Loss Budget

The theoretical and experimentally observed end-to-end efficiency chain is modeled below:

```
 Grid/Solar Input        Inverter Stage        Coil Air Gap          Rectifier & Filter       Battery Pack
   [ P_in: 100% ] ──► [ η_inv: 94-96% ] ──► [ η_coil: 86-90% ] ──► [ η_rect: 92-94% ] ──► [ P_out: 76-81% ]
```

| Subsystem Stage | Typical Efficiency ($\eta$) | Primary Loss Mechanism |
|-----------------|-----------------------------|------------------------|
| **Power Management & Inverter** | $94.5\%$ | MOSFET $R_{DS(on)}$ conduction & gate drive losses |
| **Coil Coupling (Air Gap 25mm)** | $88.2\%$ | Copper AC winding resistance & magnetic flux leakage |
| **Secondary Rectification & Filter** | $93.0\%$ | Diode forward voltage drop ($V_f$) & ESR capacitor losses |
| **BMS & Relay Isolation** | $99.1\%$ | Relay contact resistance ($<50\text{ m}\Omega$) |
| **System Overall End-to-End** | **$\mathbf{76.8\% - 81.2\%}$** | Net wireless DC-to-DC transfer efficiency |
