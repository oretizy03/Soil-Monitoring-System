/* SENSOR_CALIBRATION.ino
   Standalone calibration utility for the capacitive moisture sensor and the PH4502C pH probe. This is a SEPARATE sketch from SMART_SOIL_MONITOR.ino - flash this one temporarily, note the numbers it gives you, then reflash the main sketch with those numbers plugged into AIR_VALUE / WATER_VALUEand ADC_PH7 / ADC_PH25.

   HOW TO USE 
   1. Flash this sketch, open Serial Monitor at 115200 baud.
   2. MOISTURE: hold the sensor in open air (nothing touching the sensing plates). Watch the "Moisture Status" column - wait for it to say STABLE, then note the "Moisture Raw" value. That's your new AIR_VALUE. Then submerge the sensor to your normal working depth in water, wait for STABLE again, note that value - that's your new WATER_VALUE.
   3. pH: rinse the probe, place it in a known pH buffer solution (e.g. a pH 7.0 buffer). Wait for "pH Status" to say STABLE, note the "pH Raw" value and which buffer it was in. Rinse, place it in a second known buffer (e.g. pH 4.0 or whatever the main sketch's second reference point is), repeat. Those two (known pH, raw value) pairs are what ADC_PH7 / ADC_PH25 in the main sketch are built from.

   A reading only turns STABLE once the last 10 samples (5 seconds) are all within a small span of each other - this stops you from copying down a number the sensor hasn't actually settled on yet, which is the single most common source of a bad calibration. */

#include <Arduino.h>

#define MOISTURE_PIN 34
#define PH_PIN 35

const int WINDOW = 10;                 
const int STABLE_MOISTURE_SPAN = 15;   
const int STABLE_PH_SPAN = 10;

int moistureWindow[WINDOW];
int phWindow[WINDOW];
int windowIndex = 0;
int windowCount = 0;

unsigned long lastSample = 0;

int readPHRaw() {
  long total = 0;
  const int SAMPLES = 10;
  for (int i = 0; i < SAMPLES; i++) {
    total += analogRead(PH_PIN);
    delay(5);
  }
  return total / SAMPLES;
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  delay(500);

  Serial.println();
  Serial.println("=== Soil Sensor Calibration Utility ===");
  Serial.println("Moisture: hold in air, then water - note the raw value once STABLE.");
  Serial.println("pH: place probe in a known buffer solution - note the raw value once STABLE.");
  Serial.println();
  Serial.println("Moisture Raw\tMoisture Status\tpH Raw (avg x10)\tpH Status");
}

void loop() {
  if (millis() - lastSample < 500) return;   
  lastSample = millis();

  int moistureRaw = analogRead(MOISTURE_PIN);
  int phRaw = readPHRaw();

  moistureWindow[windowIndex] = moistureRaw;
  phWindow[windowIndex] = phRaw;
  windowIndex = (windowIndex + 1) % WINDOW;
  if (windowCount < WINDOW) windowCount++;

  String moistureStatus = "settling...";
  String phStatus = "settling...";

  if (windowCount == WINDOW) {
    int mMin = moistureWindow[0], mMax = moistureWindow[0];
    int pMin = phWindow[0], pMax = phWindow[0];

    for (int i = 1; i < WINDOW; i++) {
      mMin = min(mMin, moistureWindow[i]);
      mMax = max(mMax, moistureWindow[i]);
      pMin = min(pMin, phWindow[i]);
      pMax = max(pMax, phWindow[i]);
    }

    if (mMax - mMin <= STABLE_MOISTURE_SPAN) moistureStatus = "STABLE";
    if (pMax - pMin <= STABLE_PH_SPAN) phStatus = "STABLE";
  }

  Serial.print(moistureRaw);
  Serial.print("\t\t");
  Serial.print(moistureStatus);
  Serial.print("\t\t");
  Serial.print(phRaw);
  Serial.print("\t\t\t");
  Serial.println(phStatus);
}
