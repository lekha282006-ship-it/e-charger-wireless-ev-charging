"""
serial_logger.py
Telemetry Logger for E-Charger Wireless EV Charging Prototype

Reads real-time CSV telemetry from Arduino Uno over serial and saves it to a timestamped CSV file.
Usage:
    python serial_logger.py --port COM3 --baud 9600 --output telemetry_run.csv
"""

import argparse
import sys
import time

try:
    import serial
except ImportError:
    print("[WARNING] pyserial not installed. Install via: pip install pyserial")
    serial = None


def run_logger(port: str, baud: int, output_file: str):
    if serial is None:
        print("[ERROR] Cannot run without pyserial. Please install pyserial.")
        sys.exit(1)

    print(f"[*] Opening serial port {port} at {baud} baud...")
    try:
        ser = serial.Serial(port, baud, timeout=2.0)
        time.sleep(2)  # Allow Arduino auto-reset to settle
        print(f"[*] Connected! Logging data to '{output_file}'...")
        print("[*] Press Ctrl+C to stop logging.\n")

        with open(output_file, "w", encoding="utf-8") as f:
            header_written = False
            while True:
                line = ser.readline().decode("utf-8", errors="ignore").strip()
                if not line:
                    continue

                # Filter debug banner lines
                if line.startswith("=") or "E-CHARGER" in line or "Author" in line:
                    print(f"  [BOOT] {line}")
                    continue

                # Echo to console
                print(f"  [DATA] {line}")

                # Write CSV line
                f.write(line + "\n")
                f.flush()

    except KeyboardInterrupt:
        print("\n[*] Logging stopped by user.")
    except Exception as e:
        print(f"[ERROR] Serial error: {e}")
    finally:
        if "ser" in locals() and ser.is_open:
            ser.close()
            print("[*] Serial port closed.")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="E-Charger Prototype Serial Telemetry Logger")
    parser.add_argument("--port", type=str, default="COM3", help="Serial port (e.g., COM3, /dev/ttyUSB0)")
    parser.add_argument("--baud", type=int, default=9600, help="Baud rate (default: 9600)")
    parser.add_argument("--output", type=str, default="telemetry_log.csv", help="Output CSV filepath")
    args = parser.parse_args()

    run_logger(args.port, args.baud, args.output)
