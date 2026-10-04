# Soil Monitoring for Smart Agriculture

Design and Implementation of a Sensor-Based Soil Monitoring System

EEG 323: Instrumentation and Measurement II
Department of Electrical and Electronics Engineering, Faculty of Engineering, University of Lagos
Group 2 Project, July 2026

## Overview

This project is a low-cost, sensor-based soil monitoring system for smart agriculture. It uses an ESP32 microcontroller to read soil moisture, soil temperature and soil pH, process the readings, and show them in real time on a small OLED screen and on a web dashboard that any phone or laptop can open.

The system is aimed at rural farmers who currently rely on guesswork or on expensive, slow laboratory testing. It is portable, battery powered, built from commonly available parts, and simple enough to operate without engineering knowledge.

## Contents

- Problem Statement
- Aim and Objectives
- Scope
- Significance
- Features
- Repository Structure
- Technical Background
- System Design
- Firmware
- Calibration
- Getting Started
- Results
- Bill of Materials
- Conclusion
- Future Improvements

## Problem Statement

- Many rural farmers are stuck with traditional soil testing because modern methods are not available to them.
- Where modern methods are available, they need a high level of skill to operate.
- Repair and maintenance of modern soil testing equipment is very expensive.

Traditional testing usually means looking at the soil and estimating its condition, or taking samples to a laboratory. Laboratory testing is accurate but costs money and takes time.

## Aim and Objectives

The aim is to design and implement a sensor-based soil monitoring system that accurately measures and monitors soil moisture, temperature and pH for smart agriculture. The system acquires, processes and displays sensor data in real time to support informed irrigation and crop management decisions.

Objectives:

- Build a portable and stable unit that can be carried to and from farmers' fields.
- Make it easy to fix and maintain, using easily available parts.
- Make it easy to operate and understand for people from all walks of life, without needing extensive engineering know-how.

## Scope

The project covers the physical design, hardware implementation and software development needed to test soil moisture, temperature and pH in real time:

- Hardware integration that can measure pH, moisture and temperature.
- Signal processing and analogue-to-digital conversion.
- Real-time data display through visual output.
- A stand-alone battery for power supply.

## Significance

For farmers: a cheap and portable way to test soil. They no longer have to take soil to a laboratory, spending money and wasting time to get accurate results.

For students: a way to put into practice their knowledge of sensors, microcontrollers and embedded systems.

## Features

- Measures soil moisture, soil temperature and soil pH.
- Local display on a 1.3-inch SH1106 OLED over I2C.
- Self-hosted Wi-Fi access point with a captive-portal web dashboard. No router, mobile data or special app is needed.
- Works with Android, iOS and Windows devices. The dashboard opens automatically after joining the network.
- Plain-language status for each reading (Dry, Good, Wet for moisture; Acidic, Neutral, Alkaline for pH).
- Last 20 readings (5 second interval) shown on the dashboard.
- 24-hour trend table built from hourly averages, plus a live row for the current hour.
- Dedicated 5 V supply for the pH front-end with decoupling capacitors to keep analog readings steady during Wi-Fi bursts.
- Battery powered (2S Li-ion pack, 7.4 V).

## Repository Structure

```
.
├── README.md
├── Presentation slides: Soil_Monitoring_for_Smart_Agriculture.pptx
├── src/
│   └── main.cpp
├── Sensor Calibration .cpp/
│   └── SENSOR_CALIBRATION.cpp
└── Arduino/
    └── Arduino IDE versions of the code
```

- src/main.cpp is the main firmware for the ESP32.
- Sensor Calibration .cpp/SENSOR_CALIBRATION.cpp is the calibration code used to obtain the reference ADC values for the moisture and pH sensors.
- Arduino contains the Arduino IDE versions of the code.
- The presentation slides are in the same folder as this README.

## Technical Background

### Precision agriculture and smart farming

Precision agriculture uses digital technologies to monitor, measure and manage variations in agricultural fields so that farming can be done more efficiently. Instead of applying water and fertilizer evenly over a whole field, it recognises that soil moisture, pH, nutrient concentration and organic matter can differ even within a small area. Smart farming extends this by adding embedded systems and wireless communication, so farmers can monitor conditions continuously and decide on irrigation and soil management with less waste and better yield.

### Soil moisture: resistive and capacitive sensors

Resistive sensors measure the resistance between two metal probes in the soil. They are cheap and simple, but the probes sit in wet soil with current flowing through them, so electrolysis corrodes the metal over time. Accuracy drops and the sensor eventually has to be replaced.

Capacitive sensors measure changes in capacitance caused by the soil's dielectric properties. The electrodes are enclosed inside the printed circuit board and never touch the soil moisture directly, so they last much longer. Dry soil has a dielectric constant of about 3 to 5, while water is about 80, so capacitance changes strongly with water content. An onboard oscillator converts this into an analog voltage for the microcontroller. This project uses the Capacitive Soil Moisture Sensor v1.2.

### Soil temperature

Soil temperature affects seed germination, microbial activity, nutrient availability and root development. Analog sensors such as thermistors and the LM35 are simple but pick up electrical noise and suffer voltage drop over long outdoor cables.

This project uses the DS18B20 digital sensor (waterproof stainless-steel version). It uses the 1-Wire protocol, so values reach the microcontroller as digital data and are much less affected by interference and cable resistance. It works from -55 C to +125 C with an accuracy of about 0.5 C in the normal agricultural range.

### Soil pH

pH shows the concentration of hydrogen ions in the soil solution on a logarithmic scale. Most crops do best between pH 6.0 and 7.5. Outside that range, nutrient availability drops and yield can suffer.

The project uses a combination pH electrode with the PH4502C signal conditioning module. The electrode works according to the Nernst equation:

```
E = E0 - (2.303 R T / F) * pH
```

Where E is the measured electrode potential, E0 is the standard electrode potential, R is the universal gas constant, T is the absolute temperature in Kelvin, and F is the Faraday constant.

The electrode voltage is far too small for a microcontroller to measure directly. The PH4502C amplifies and conditions it into an analog output between 0 V and 5 V, with about 2.5 V corresponding to neutral pH 7.0.

### Selected sensors

| Parameter | Selected sensor or technology |
| --- | --- |
| Soil moisture | Capacitive Soil Moisture Sensor v1.2 |
| Soil temperature | DS18B20 Digital Temperature Sensor |
| Soil pH | PH4502C pH Sensor Module |

### Microcontroller choice

- Arduino Uno (ATmega328P): popular and well supported, 5 V logic, 10-bit ADC (1,024 levels). Limited memory and processing power make it a poor fit for several sensors plus real-time data handling.
- ESP8266: adds built-in Wi-Fi in a small, cheap package and runs at 3.3 V logic. It has only one analog input, so reading several analog sensors needs extra circuitry.
- ESP32: 32-bit dual-core Tensilica Xtensa LX6 up to 240 MHz, 520 KB SRAM, built-in Wi-Fi and Bluetooth, and several communication interfaces.

The ESP32 DevKit V1 was selected. It has enough analog inputs, supports digital communication with the DS18B20, talks to the display over I2C, and has enough processing power to handle all sensors at once.

### Signal conditioning and voltage matching

Many analog sensor modules run on 5 V, but the ESP32 analog pins accept at most about 3.3 V. Going over that can permanently damage the board or make it unstable. A passive resistive voltage divider scales the sensor output down while keeping the relationship between signal and physical value linear.

This project uses a 10 kOhm and 20 kOhm divider, which reduces the 5.0 V maximum of the PH4502C output to about 3.3 V before it reaches the ESP32 ADC.

## System Design

### Overview

The system is built around the ESP32 DevKit V1 and combines three sensing subsystems, a local display and a wireless dashboard in one portable unit.

### Pin connections

| Function | Device | ESP32 pin |
| --- | --- | --- |
| Soil moisture (analog) | Capacitive soil moisture sensor v1.2 | GPIO34 |
| Soil temperature (1-Wire) | DS18B20 | GPIO4 |
| Soil pH (analog, through divider) | PH4502C module | GPIO35 |
| I2C data | SH1106 OLED | GPIO21 (SDA) |
| I2C clock | SH1106 OLED | GPIO22 (SCL) |

The OLED uses I2C address 0x3C. The schematic also shows a 4.7 kOhm resistor on the DS18B20 data line.

### Block diagram

```
                                      SENSOR LAYER:
                                Capacitive Moisture (GPIO34)
                                DS18B20 Temperature (GPIO4)
                                PH4502C pH Module (GPIO35)
                                          |
                                    ESP32 DEVKIT V1
                Acquisition - Processing - Classification - Buffering
                        |                         |
                  SH1106 OLED                Wi-Fi Soft AP + Captive-Portal Dashboard
                  (I2C, local display)       (remote display)
              
              POWER SUPPLY 
                AMS1117 5V regulator + decoupling capacitors
```

### Power supply design

Power stability was treated as a design priority. The ESP32 Wi-Fi radio draws current in short, high bursts that can disturb sensitive analog readings. To handle this:

- An AMS1117 5V regulator provides a clean, dedicated 5 V rail for the pH front-end, separate from the logic supply.
- A 100 uF electrolytic capacitor and a 100 nF ceramic capacitor are placed across the power rails to suppress low-frequency ripple and high-frequency switching noise.
- The result is less analog signal drift while Wi-Fi is transmitting.

## Firmware

The firmware follows the standard Arduino setup() and loop() model. setup() handles one-time initialisation. loop() handles continuous acquisition, processing and display.

### Initialisation sequence

1. Start the serial interface and the I2C bus.
2. Initialise the OLED display and show a "Booting..." splash message.
3. Initialise the DS18B20 and set its conversion resolution to 10-bit.
4. Start the Wi-Fi access point.
5. Register the DNS server and the captive-portal routes.
6. Start the web server.
7. Start the one-hour trend timer.

### Main loop

1. Handle any pending DNS or web server requests.
2. Read the temperature from the DS18B20.
3. Read the moisture level from the ADC and convert it to a percentage.
4. Read the pH from 20 averaged analog samples and convert it to a pH value.
5. Update the current-reading variables used by both the OLED and the web dashboard.
6. Add the new readings to the running hourly sum.
7. If an hour has elapsed, finalise that hour into the trend table and reset the running sums.
8. If five seconds have elapsed, log a new entry into the 20-reading history buffer.
9. Redraw the OLED with the current values.
10. Delay for one second, then repeat.

### Sensor data processing

Temperature is used directly from the DS18B20, since it already outputs a calibrated digital value.

Moisture is mapped from the raw ADC reading between two calibration points taken in open air and in water:

```
Moisture (%) = (ADC_raw - ADC_air) / (ADC_water - ADC_air) * 100
```

pH uses a two-point calibration at pH 7.0 and pH 2.5, with a 20-sample average of the raw ADC reading to reduce noise:

```
pH = 7.0 - (ADC_raw - ADC_pH7) * 4.5 / (ADC_pH2.5 - ADC_pH7)
```

ADC_raw is the 20-sample averaged reading during operation. ADC_pH7 and ADC_pH2.5 are the reference ADC values recorded during calibration at pH 7.0 and pH 2.5. In the firmware, moisture is limited to 0 to 100 percent and pH to 0 to 14.

### Qualitative classification

To make readings easy for non-technical users, the firmware turns each number into a status using fixed thresholds.

| Parameter | Condition | Status |
| --- | --- | --- |
| Moisture | Moisture < 30% | Dry |
| Moisture | 30% <= Moisture < 70% | Good |
| Moisture | Moisture >= 70% | Wet |
| pH | pH < 6.5 | Acidic |
| pH | 6.5 <= pH <= 7.5 | Neutral |
| pH | pH > 7.5 | Alkaline |

### Data logging

Two circular buffers are kept in memory:

- Short-term buffer: the last 20 readings taken at 5 second intervals, shown side by side on the dashboard.
- 24-hour trend buffer: readings are accumulated over rolling 1-hour windows (3,600,000 ms) and finalised into hourly averages of temperature, moisture and pH, with an overall status. The dashboard also shows a LIVE row for the hour in progress.

Both buffers live in memory, so they are cleared when the device resets or loses power.

### Wi-Fi access point and captive portal

- The ESP32 runs as a Wi-Fi access point named SMART_SOIL_MONITOR.
- A DNS server answers every request with the ESP32 address, and several captive-portal detection routes are registered (generate_204, gen_204, hotspot-detect.html, ncsi.txt, connecttest.txt). Any other path also returns the dashboard.
- Because of this, Android, iOS and Windows devices open the dashboard automatically after joining the network.
- The dashboard page refreshes every 5 seconds and shows the current readings, the last 20 readings, and the 24-hour trend table.
- In the supplied firmware the access point is created without a password. If you want to restrict access, pass a password to WiFi.softAP().

### OLED display

The 1.3-inch SH1106 display shows the title, temperature, moisture with its status, and pH with its status, redrawn every loop.

## Calibration

The firmware relies on four reference ADC values, set as constants at the top of the code:

| Constant | Value | Meaning |
| --- | --- | --- |
| AIR_VALUE | 2469 | Moisture sensor ADC reading in open air (0 percent) |
| WATER_VALUE | 752 | Moisture sensor ADC reading in water (100 percent) |
| ADC_PH7 | 2975 | pH module ADC reading at pH 7.0 |
| ADC_PH25 | 3790 | pH module ADC reading at pH 2.5 |

These values were obtained with the sensors used in this project. Every sensor and probe is slightly different, so recalibrate when you change any of them. The code in the Sensor Calibration .cpp folder is for this purpose: read the raw ADC values in each reference condition and put them into the constants in the main firmware.

ADC resolution is set to 12 bits (0 to 4095).

## Getting Started

### Requirements

Hardware: see the Bill of Materials below.

Board: ESP32 DevKit V1.

Libraries:

- Adafruit GFX Library
- Adafruit SH110X
- OneWire
- DallasTemperature
- WiFi, WebServer, DNSServer and Wire come with the ESP32 board package.

### Upload the firmware

1. Wire the components as listed under Pin connections.
2. Calibrate the sensors and update the four constants (see Calibration).
3. Open the Arduino IDE version in the Arduino folder, or build src/main.cpp with your own C++ toolchain.
4. Select the ESP32 DevKit V1 board and the correct port, then upload.

### Use the monitor

1. Power the unit from the battery pack.
2. Place the moisture sensor and the DS18B20 probe in the soil, and the pH probe as required for measurement.
3. On a phone or laptop, join the Wi-Fi network SMART_SOIL_MONITOR.
4. The dashboard should open automatically. If it does not, open a browser and go to the ESP32 access point address (usually 192.168.4.1).
5. Read the current values, the status labels, the last 20 readings and the 24-hour trend. The OLED shows the live values locally.

## Results

### Functional validation

Bench testing evaluated the integrated system under operating conditions. Signal integrity, power management stability, sensor conversion performance, and both outputs (OLED screen and Wi-Fi dashboard) were checked, with the probes placed in a container of sand.

### OLED display issue and fix

The SH1106 display initialised correctly over I2C at address 0x3C but showed no text. The cause was an uninitialised text colour in the Adafruit_GFX library, which defaulted to the same colour as the background, so the text was invisible. Calling the following fixed it:

```
display.setTextColor(SH110X_WHITE);
```

Sample OLED output after the fix:

```
SMART SOIL MONITOR
Temp: 27.7 C
Moist: 41% Good
pH: 9.29 Alkaline
```

### Soil moisture sensor

The capacitive sensor gave stable analog output on GPIO34 without signal degradation. Using the air value of 2469 and the water value of 752 in the moisture equation, readings were normalised across 0 to 100 percent. During tests the system recorded a stable moisture level of 41 percent and classified it as Good.

### Temperature sensor

The DS18B20 on GPIO4 gave continuous, high-precision readings. Reducing the conversion resolution from 12-bit to 10-bit cut the blocking conversion delay from about 750 ms to about 187 ms per acquisition loop. The measured temperature stayed at 27.7 C with a resolution of 0.25 C, matching the expected ambient laboratory conditions.

### pH sensor

The PH4502C output was scaled down to safe levels for the ESP32 ADC pin (GPIO35) using the 10 kOhm and 20 kOhm voltage divider. Two-point calibration gave ADC(pH 7) = 2975 and ADC(pH 2.5) = 3790. Oversampling the analog pin 20 times per loop filtered out transient power fluctuations and gave a steady raw ADC output. The test sample gave a pH of 9.29, which the firmware correctly labelled Alkaline (pH above 7.5).

### Wireless dashboard

The SMART_SOIL_MONITOR access point and DNS server gave seamless connectivity. Android, iOS and Windows devices were automatically served the dashboard through captive-portal detection when they joined the network.

| Metric | Measured or logged output | Note |
| --- | --- | --- |
| Temperature | 27.7 C | Nominal room or soil temperature |
| Moisture level | 41.8% (live average) | Good status range |
| Soil pH | 9.33 (live average) | Alkaline status range |
| History array | 20 samples at 5 second interval | Rendered horizontally across columns |
| 24-hour trend | Rolling hourly averages plus LIVE row | Fully populated in memory buffer |

The 24-hour trend log accumulated sample sums over the 1-hour window and produced a live row with an average temperature of 27.7 C, average moisture of 41.8%, average pH of 9.33 and an overall status of Good.

### Discussion

The results confirm that the system meets the objectives set out for the project.

- Signal isolation and power stability: the dedicated 5 V supply for the pH front-end through the LM2596 buck converter, together with decoupling capacitors, prevented analog signal drift during Wi-Fi transmission bursts.
- User accessibility: a stand-alone captive portal removes the need for external routers, cellular coverage or special client software, which makes the device suitable for non-technical users in rural field environments.

## Bill of Materials

| S/N | Component | Description | Qty | Estimated cost (NGN) |
| --- | --- | --- | --- | --- |
| 1 | ESP32 DevKit V1 | Microcontroller | 1 | 14,000 |
| 2 | 1.3-inch I2C OLED Display | | 1 | 8,000 |
| 3 | Capacitive Soil Moisture Sensor v1.2 | | 1 | 1,800 |
| 4 | DS18B20 Waterproof Digital Temperature Sensor | | 1 | 820 |
| 5 | Analog pH Sensor Kit (PH4502C + Glass Probe) | | 1 | 25,000 |
| 6 | Full-Sized Solderless Breadboard | | 1 | 2,200 |
| 7 | Assorted Jumper Wires (M-M, M-F, F-F) | | 1 | 2,600 |
| 8 | Resistors, Potentiometers, and Voltage Dividers | | 1 | 480 |
| 9 | Lithium-Ion Battery Pack (2S, 7.4V) | | 1 | 2,000 |
| 10 | Decoupling Capacitors (100 uF + 0.1 uF) | | 1 | 100 |
| 11 | AMS1117 5V regulator | | 1 | 1,100 |
| | Total | | | 58,100 |

## Conclusion

The project designed, built and tested an ESP32-based smart soil monitoring system for precision agriculture. It monitors soil moisture, temperature and pH in real time, and it meets the main goal of being low cost, practical, and easy to use and maintain in agricultural settings.

The main design decisions worked well:

- The 10 kOhm and 20 kOhm voltage divider let the 5 V output of the PH4502C connect safely to the 3.3 V analog input of the ESP32, while keeping the signal stable enough for consistent readings.
- The dedicated AMS1117 5V regulator and the 100 uF and 100 nF capacitors reduced noise and voltage fluctuation, especially when the ESP32 Wi-Fi was transmitting.
- Reducing the DS18B20 resolution to 10-bit brought the conversion time down to about 187 ms while keeping enough accuracy for soil monitoring.
- The captive portal and dual circular buffers let users see the current values and the changes over the previous 24 hours on a simple web dashboard.

Overall the hardware and software worked together reliably during testing, and the system gave information that can help farmers make better irrigation and soil management decisions. There is still room for improvement, but the project shows that an affordable smart soil monitoring system can be built with readily available components.

## Future Improvements

- Automated irrigation control: add a relay-controlled water pump or solenoid valve that responds to moisture thresholds, so the system acts on what it senses.
- Solar-powered operation: add a solar panel and rechargeable battery circuit for long, unattended use in open fields.
- Multi-node wireless coverage: one unit only reflects one point in a field. Several ESP32 nodes could talk to a central gateway over ESP-NOW or LoRa to cover larger or multiple plots.
- Long-term data storage: add an SD card module or cloud logging, on top of the in-memory buffers, to keep data beyond 24 hours and allow trend analysis over full growing seasons.
- Additional soil parameters: add nitrogen, phosphorus and potassium (NPK) sensors for a fuller picture of soil health.
- Mobile application: a dedicated app, alongside the captive-portal dashboard, for better usability and push notifications when a reading goes out of its acceptable range.

## Credits

Developed by Group 2 as part of EEG 323: Instrumentation and Measurement II, Department of Electrical and Electronics Engineering, University of Lagos.
