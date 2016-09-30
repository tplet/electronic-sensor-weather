#include <Arduino.h>
#include <StandardCplusplus.h>
#include <SPI.h>
#include "RF24/nRF24L01.h"
#include <RF24/RF24.h>
#include <RTClib/RTClib.h>
#include <LiquidCrystal.h>
#include <DHT.h>
#include <com/osteres/automation/weathersensor/WeatherSensorApplication.h>
#include <com/osteres/automation/transmission/Transmitter.h>
#include <com/osteres/automation/arduino/transmission/ArduinoRequester.h>


using com::osteres::automation::weathersensor::WeatherSensorApplication;
using com::osteres::automation::transmission::Transmitter;
using com::osteres::automation::arduino::transmission::ArduinoRequester;
using com::osteres::automation::transmission::packet::Packet;
using com::osteres::automation::transmission::packet::Command;

/* Pins CE, CSN for ARDUINO */
#define RF_CE    9
#define RF_CSN   10
/* Pin for DHT sensor and DHT type */
#define DHT_PIN  2
#define DHT_TYPE DHT22

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
// Screen
LiquidCrystal lcd(3, 4, 5, 6, 7, 8);
// Sensor
DHT sensor(DHT_PIN, DHT_TYPE);

/*
 * Prepare object manager
 */
// Transmission (master mode)
Transmitter transmitter(&radio, false);
// Application
WeatherSensorApplication application(&sensor, &lcd, &rtc, &transmitter);

/**
 * Initialize
 */
void setup() {
    Serial.begin(9600);

    // Set requester manually
    transmitter.setRequester(new ArduinoRequester(transmitter.getRadio(), transmitter.getWritingChannel()));

    // Setup transmitter
    transmitter.setup();

    // Setup (configuration)
    application.setup();
    application.setIntervalScreenRefresh1(100); // 0.1s
    application.setIntervalScreenRefresh2(5000); // 5s
    application.getWeatherBuffer()->setBufferDelay(30000); // 30s
}

//Packet * packet;
//unsigned int i = 0;


/**
 * Loop
 */
void loop()
{
    // Process
    application.process();
}