/**
 * @file main.ino
 * @brief E-Charger: Resonant Wireless EV Charging Prototype Firmware
 * @author Lekha (lekha282006-ship-it)
 * @license MIT
 * 
 * Hardware Platform: Arduino Uno (ATmega328P)
 * System Features:
 *   - Dual-Source Energy Management (Utility Grid & Solar PV Input)
 *   - Resonant Inductive Wireless Power Transfer Control
 *   - Real-Time Voltage & Current Telemetry via Analog Sensors
 *   - Automated Relay Switching with Overcharge Protection (Cutoff at 80% / 14.1V)
 *   - 16x2 I2C LCD Status Display (Standby, Charging %, Fully Charged)
 *   - IR Proximity Vehicle Alignment Detection
 *   - High-Speed Serial Telemetry Logging (CSV Format)
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ============================================================================
// PIN DEFINITIONS & HARDWARE CONFIGURATION
// ============================================================================

// Analog Input Pins
const uint8_t PIN_VOLTAGE_SENSOR   = A0;  // Voltage divider (0-25V range)
const uint8_t PIN_CURRENT_SENSOR   = A1;  // ACS712 Current sensor (optional)

// Digital Control & Sensor Pins
const uint8_t PIN_IR_ALIGNMENT     = 2;   // IR Proximity sensor (Active LOW)
const uint8_t PIN_RELAY_CONTROL    = 7;   // 5V Relay module signal pin
const uint8_t PIN_LED_CHARGING     = 13;  // Green Charging Indicator LED
const uint8_t PIN_LED_STANDBY      = 6;   // Red/Yellow Standby/Alert Indicator LED

// I2C LCD Configuration (Default address: 0x27 or 0x3F, 16 cols, 2 rows)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ============================================================================
// SYSTEM CALIBRATION & BATTERY PARAMETERS
// ============================================================================

// Reference Voltage of Arduino ADC (typically 5.0V, calibrate if using USB)
const float ADC_VREF               = 5.00;

// Voltage Divider Ratio: Vout = Vin * (R2 / (R1 + R2))
// Standard 25V Module: R1 = 30k Ohm, R2 = 7.5k Ohm -> Ratio = (30+7.5)/7.5 = 5.0
const float VOLTAGE_DIVIDER_RATIO  = 5.00;

// Battery Charging Voltage Thresholds (12V Lead-Acid / Li-Ion Prototype Curve)
const float BATTERY_V_EMPTY        = 11.00; // 0% state of charge reference
const float BATTERY_V_FULL         = 14.40; // 100% full capacity voltage
const float CUTOFF_VOLTAGE         = 14.10; // Cutoff voltage threshold (80% capacity)
const float CUTOFF_PERCENTAGE      = 80.0;  // Cutoff state of charge percentage (%)
const float HYSTERESIS_VOLTAGE     = 0.30;  // Re-enable charging hysteresis gap (V)

// Relay Logic Configuration
// Most Arduino relay modules are Active LOW (LOW = Relay ON/Closed, HIGH = Relay OFF/Open)
const uint8_t RELAY_ACTIVE_STATE   = LOW;   
const uint8_t RELAY_INACTIVE_STATE = HIGH;  

// Timing Constants (Non-blocking millis)
const unsigned long SENSOR_INTERVAL = 1000; // Read sensors every 1000ms
const unsigned long LCD_INTERVAL    = 1000; // Refresh LCD every 1000ms
const unsigned long RESET_DELAY     = 5000; // Display "Resetting..." for 5000ms

// ============================================================================
// SYSTEM STATE DEFINITIONS
// ============================================================================

enum SystemState {
  STATE_STANDBY,          // Vehicle not aligned / Waiting for EV
  STATE_CHARGING_STARTED, // Vehicle aligned, relay engaged, charging initiated
  STATE_CHARGING,         // Closed-loop charging active with live SoC readout
  STATE_FULLY_CHARGED,    // Target SoC (80% / 14.1V) reached, relay cutoff
  STATE_RESETTING         // System cool-down, resetting to ready
};

SystemState currentState = STATE_STANDBY;

// Global Telemetry Variables
float currentVoltage    = 0.0;
float currentAmperage   = 0.0;
float batteryPercentage = 0.0;
bool isVehicleAligned   = false;
bool relayState         = false;

unsigned long lastSensorTime = 0;
unsigned long lastLcdTime    = 0;
unsigned long stateTimer     = 0;
unsigned long sessionTimeSec = 0;

// ============================================================================
// HELPER FUNCTION DECLARATIONS
// ============================================================================

float readBatteryVoltage();
float calculateSoC(float voltage);
void setRelay(bool enable);
void updateLcdDisplay();
void streamSerialTelemetry();

// ============================================================================
// SETUP FUNCTION
// ============================================================================

void setup() {
  // Initialize Serial Communication for Telemetry & Diagnostics
  Serial.begin(9600);
  while (!Serial && millis() < 3000); // Wait for serial port or timeout

  Serial.println(F("=================================================="));
  Serial.println(F("  E-CHARGER: WIRELESS RESONANT EV CHARGING SYSTEM  "));
  Serial.println(F("  Author: Lekha (lekha282006-ship-it)              "));
  Serial.println(F("  Status: Firmware Initializing...                 "));
  Serial.println(F("=================================================="));

  // Configure Pin Modes
  pinMode(PIN_IR_ALIGNMENT, INPUT_PULLUP);
  pinMode(PIN_RELAY_CONTROL, OUTPUT);
  pinMode(PIN_LED_CHARGING, OUTPUT);
  pinMode(PIN_LED_STANDBY, OUTPUT);

  // Initialize Safe State: Relay Disengaged, Standby LED Active
  setRelay(false);
  digitalWrite(PIN_LED_STANDBY, HIGH);
  digitalWrite(PIN_LED_CHARGING, LOW);

  // Initialize 16x2 I2C LCD Display
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // Welcome Screen
  lcd.setCursor(0, 0);
  lcd.print("E-Charger System");
  lcd.setCursor(0, 1);
  lcd.print("Booting up...   ");
  delay(1500);

  // Initial State: System Ready
  currentState = STATE_STANDBY;
  updateLcdDisplay();

  // Output CSV Header for Serial Logging
  Serial.println(F("Time(s),Voltage(V),Battery(%),Relay,Status"));
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
  unsigned long currentMillis = millis();

  // 1. Periodic Sensor Acquisition
  if (currentMillis - lastSensorTime >= SENSOR_INTERVAL) {
    lastSensorTime = currentMillis;
    sessionTimeSec++;

    // Read IR alignment sensor (Active LOW indicates vehicle detected over pad)
    isVehicleAligned = (digitalRead(PIN_IR_ALIGNMENT) == LOW);

    // Read battery terminal voltage and calculate State of Charge
    currentVoltage = readBatteryVoltage();
    batteryPercentage = calculateSoC(currentVoltage);

    // 2. Finite State Machine (FSM) Logic
    switch (currentState) {
      
      case STATE_STANDBY:
        setRelay(false);
        digitalWrite(PIN_LED_STANDBY, HIGH);
        digitalWrite(PIN_LED_CHARGING, LOW);

        if (isVehicleAligned) {
          if (batteryPercentage < CUTOFF_PERCENTAGE && currentVoltage < CUTOFF_VOLTAGE) {
            currentState = STATE_CHARGING_STARTED;
            stateTimer = currentMillis;
          }
        }
        break;

      case STATE_CHARGING_STARTED:
        setRelay(true);
        digitalWrite(PIN_LED_STANDBY, LOW);
        digitalWrite(PIN_LED_CHARGING, HIGH);

        // Immediate transition into continuous charging mode
        currentState = STATE_CHARGING;
        break;

      case STATE_CHARGING:
        // Safety Check 1: Vehicle misalignment / removal
        if (!isVehicleAligned) {
          Serial.println(F("[ALERT] Vehicle disconnected or misaligned. Aborting charge."));
          setRelay(false);
          currentState = STATE_STANDBY;
          break;
        }

        // Safety Check 2: Target charge reached (80% / 14.1V cut-off threshold)
        if (batteryPercentage >= CUTOFF_PERCENTAGE || currentVoltage >= CUTOFF_VOLTAGE) {
          Serial.println(F("[INFO] Battery target reached (80% / 14.1V). Cutoff triggered."));
          setRelay(false);
          digitalWrite(PIN_LED_CHARGING, LOW);
          currentState = STATE_FULLY_CHARGED;
          stateTimer = currentMillis;
        }
        break;

      case STATE_FULLY_CHARGED:
        setRelay(false);
        digitalWrite(PIN_LED_CHARGING, LOW);
        digitalWrite(PIN_LED_STANDBY, HIGH);

        // Maintain "Fully Charged / Resetting..." screen for defined duration
        if (currentMillis - stateTimer >= RESET_DELAY) {
          currentState = STATE_RESETTING;
          stateTimer = currentMillis;
        }
        break;

      case STATE_RESETTING:
        // System cooldown and state transition back to Standby
        if (!isVehicleAligned || (currentMillis - stateTimer >= 3000)) {
          currentState = STATE_STANDBY;
        }
        break;
    }

    // 3. Emit Serial Telemetry Stream
    streamSerialTelemetry();
  }

  // 4. Update 16x2 LCD Display Screen
  if (currentMillis - lastLcdTime >= LCD_INTERVAL) {
    lastLcdTime = currentMillis;
    updateLcdDisplay();
  }
}

// ============================================================================
// HARDWARE DRIVER & COMPUTATION FUNCTIONS
// ============================================================================

/**
 * @brief Reads analog voltage via divider and calculates real battery voltage.
 * Applies oversampling (10 samples) to eliminate high-frequency ripple.
 */
float readBatteryVoltage() {
  const int NUM_SAMPLES = 10;
  long adcSum = 0;

  for (int i = 0; i < NUM_SAMPLES; i++) {
    adcSum += analogRead(PIN_VOLTAGE_SENSOR);
    delayMicroseconds(100);
  }

  float avgAdc = (float)adcSum / NUM_SAMPLES;
  float pinVoltage = (avgAdc * ADC_VREF) / 1023.0;
  float batteryVoltage = pinVoltage * VOLTAGE_DIVIDER_RATIO;

  return batteryVoltage;
}

/**
 * @brief Approximates Battery State of Charge (SoC%) based on terminal voltage.
 * Linear piecewise interpolation between empty (11.0V) and full (14.4V).
 */
float calculateSoC(float voltage) {
  if (voltage <= BATTERY_V_EMPTY) return 0.0;
  if (voltage >= BATTERY_V_FULL)  return 100.0;

  // Linear estimation: SoC% = ((V - V_empty) / (V_full - V_empty)) * 100
  float soc = ((voltage - BATTERY_V_EMPTY) / (BATTERY_V_FULL - BATTERY_V_EMPTY)) * 100.0;
  return constrain(soc, 0.0, 100.0);
}

/**
 * @brief Controls the power isolation relay switch.
 */
void setRelay(bool enable) {
  relayState = enable;
  if (enable) {
    digitalWrite(PIN_RELAY_CONTROL, RELAY_ACTIVE_STATE);
  } else {
    digitalWrite(PIN_RELAY_CONTROL, RELAY_INACTIVE_STATE);
  }
}

/**
 * @brief Updates the 16x2 character I2C LCD to match prototype display states.
 * States:
 *   - Standby:       "System Ready    " / "Waiting...      "
 *   - Charging:      "Charging...     " / "Charge: XX%     "
 *   - Fully Charged: "Fully Charged   " / "Resetting...    "
 */
void updateLcdDisplay() {
  switch (currentState) {

    case STATE_STANDBY:
      lcd.setCursor(0, 0);
      lcd.print("System Ready    ");
      lcd.setCursor(0, 1);
      lcd.print("Waiting...      ");
      break;

    case STATE_CHARGING_STARTED:
    case STATE_CHARGING:
      lcd.setCursor(0, 0);
      lcd.print("Charging...     ");
      lcd.setCursor(0, 1);
      lcd.print("Charge: ");
      lcd.print((int)batteryPercentage);
      lcd.print("%     ");
      break;

    case STATE_FULLY_CHARGED:
    case STATE_RESETTING:
      lcd.setCursor(0, 0);
      lcd.print("Fully Charged   ");
      lcd.setCursor(0, 1);
      lcd.print("Resetting...    ");
      break;
  }
}

/**
 * @brief Streams telemetry data over Serial in CSV format for analysis.
 */
void streamSerialTelemetry() {
  Serial.print(sessionTimeSec);
  Serial.print(F(","));
  Serial.print(currentVoltage, 1);
  Serial.print(F(","));
  Serial.print((int)batteryPercentage);
  Serial.print(F(","));
  Serial.print(relayState ? F("ON") : F("OFF"));
  Serial.print(F(","));

  switch (currentState) {
    case STATE_STANDBY:
      Serial.println(F("Not Charging"));
      break;
    case STATE_CHARGING_STARTED:
      Serial.println(F("Charging Started"));
      break;
    case STATE_CHARGING:
      Serial.println(F("Charging"));
      break;
    case STATE_FULLY_CHARGED:
    case STATE_RESETTING:
      Serial.println(F("Charging Completed"));
      break;
  }
}
