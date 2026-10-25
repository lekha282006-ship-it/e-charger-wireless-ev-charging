# Prototype Testing & Verification Suite

This directory contains test specifications, empirical telemetry datasets, and logging utilities for evaluating the **E-Charger: Resonant Wireless EV Charging Prototype**.

---

## 📁 Directory Structure

```
tests/
├── README.md               # This testing overview and guide
├── test-cases.md           # Formal HIL test matrix (TC-01 to TC-09) with verification procedures
├── sample-telemetry.csv    # Empirical 240s prototype telemetry dataset (from Table 1)
└── serial_logger.py        # Python script to stream and record live Arduino serial CSV data
```

---

## 📊 Sample Telemetry Data (`sample-telemetry.csv`)

The provided dataset represents the empirical charging cycle recorded on the physical prototype:

| Time (s) | Battery Voltage (V) | Battery Percentage (%) | Relay Switch State | Charging Status |
|:--------:|:-------------------:|:----------------------:|:------------------:|:---------------:|
| 0 | 11.8 | 45% | OFF | Not Charging |
| 30 | 12.1 | 48% | ON | Charging Started |
| 60 | 12.4 | 52% | ON | Charging |
| 90 | 12.7 | 56% | ON | Charging |
| 120 | 13.0 | 60% | ON | Charging |
| 150 | 13.3 | 65% | ON | Charging |
| 180 | 13.6 | 70% | ON | Charging |
| 210 | 13.9 | 75% | ON | Charging |
| 240 | 14.1 | 80% | OFF | Charging Completed |

---

## 🛠 Running Automated Telemetry Logging

To record live serial telemetry from the Arduino Uno during a physical charging run:

### Requirements
```bash
pip install pyserial
```

### Usage
```bash
# On Windows (adjust COM port as needed)
python tests/serial_logger.py --port COM3 --baud 9600 --output my_test_run.csv

# On Linux / macOS
python tests/serial_logger.py --port /dev/ttyUSB0 --baud 9600 --output my_test_run.csv
```

---

## 🧪 Formal Test Specifications
For complete step-by-step verification steps, pass/fail thresholds, and hardware test procedures, see:
👉 **[`test-cases.md`](file:///tests/test-cases.md)**
