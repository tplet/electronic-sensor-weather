/*
 * Debug
 */
// Use screen (use ~12.5 ko additional flash space / limited to 30 ko)
#define SCREEN true

#include <Arduino.h>
#include <StandardCplusplus.h>
#include <SPI.h>
#include "RF24/nRF24L01.h"
#include <RF24/RF24.h>
#include <RTClib/RTClib.h>
#if SCREEN
#include <LiquidCrystal.h>
#endif
#include <DHT.h>
#include <com/osteres/automation/weathersensor/WeatherSensorApplication.h>
#include <com/osteres/automation/weathersensor/display/WeatherOutput.h>
#include <com/osteres/automation/transmission/Transmitter.h>
#include <com/osteres/automation/arduino/transmission/ArduinoRequester.h>
#include <com/osteres/automation/arduino/component/BatteryLevel.h>
#include <com/osteres/automation/arduino/display/device/Screen.h>


using com::osteres::automation::weathersensor::WeatherSensorApplication;
using com::osteres::automation::weathersensor::display::WeatherOutput;
using com::osteres::automation::transmission::Transmitter;
using com::osteres::automation::arduino::transmission::ArduinoRequester;
using com::osteres::automation::transmission::packet::Packet;
using com::osteres::automation::transmission::packet::Command;
using com::osteres::automation::arduino::component::BatteryLevel;
using com::osteres::automation::arduino::display::device::Screen;

/*
 * Pin
 */
// Pins CE, CSN for ARDUINO
#define RF_CE    9
#define RF_CSN   10
// Pin for DHT sensor and DHT type
#define DHT_PIN  2
#define DHT_TYPE DHT22
// Pin for enable/disable screen (analog ping)
#define PIN_STATE_SCREEN_ANALOG 1
// Pin for battery level
#define PIN_BATTERY_LEVEL_ANALOG 0

/**
 * Vars
 */

/**
 * Configuration
 */
// Nothing

/*
 * Prepare electronic component
 */
// Radio transmitter
RF24 radio(RF_CE, RF_CSN);
// DateTime
RTC_DS1307 rtc;
#if SCREEN
// Screen
LiquidCrystal lcd(3, 4, 5, 6, 7, 8);
#endif
// Sensor
DHT sensor(DHT_PIN, DHT_TYPE);

/*
 * Prepare object manager
 */
// Transmission (master mode)
Transmitter transmitter(&radio, false);
// Application
WeatherSensorApplication application(&transmitter, &rtc, &sensor);
// BatteryLevel
BatteryLevel batteryLevel(PIN_BATTERY_LEVEL_ANALOG);
#if SCREEN
// Output
WeatherOutput ouput(new Screen(&lcd), &rtc, &batteryLevel, application.getPropertyIdentifier(), application.getWeatherBuffer());
#endif

/**
 * Initialize
 */
void setup() {
    Serial.begin(9600);

    // Read current vcc voltage
    float vcc = readVcc() / 1000.0;
    batteryLevel.setVcc(vcc);
    // Adjust real values of resistors
    batteryLevel.setResistanceGnd(10.02e3);
    batteryLevel.setResistanceVcc(3.28e3);

    // Set requester manually
    transmitter.setRequester(new ArduinoRequester(transmitter.getRadio(), transmitter.getWritingChannel()));

    // Setup transmitter
    transmitter.setup();

    #if SCREEN
    // Setup output
    ouput.getPointScreen1Buffer()->setBufferDelay(100); // 0.1s
    ouput.getPointScreen2Buffer()->setBufferDelay(5000); // 5s
    static_cast<Screen *>(ouput.getDevice())->enableSwitchDetection(PIN_STATE_SCREEN_ANALOG, false);
    application.setOutput(&ouput);
    #endif

    // Setup (configuration)
    application.setBatteryLevel(&batteryLevel);
    application.getWeatherBuffer()->setBufferDelay(30000); // 30s
    application.setup();
}

/**
 * Loop
 */
void loop()
{
    // Process
    application.process();
}

long readVcc() {
    // Read 1.1V reference against AVcc
    // set the reference to Vcc and the measurement to the internal 1.1V reference
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
#else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#endif

    delay(2); // Wait for Vref to settle
    ADCSRA |= _BV(ADSC); // Start conversion
    while (bit_is_set(ADCSRA,ADSC)); // measuring

    uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
    uint8_t high = ADCH; // unlocks both

    long result = (high<<8) | low;

    result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
    return result; // Vcc in millivolts
}