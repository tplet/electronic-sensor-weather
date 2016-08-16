#include <Arduino.h>
#include <SPI.h>
#include "RF24/nRF24L01.h"
#include <RF24/RF24.h>
#include <RTClib/RTClib.h>
#include <LiquidCrystal.h>
#include <com/osteres/util/Arduino.h>
#include <com/osteres/automation/weathersensor/WeatherSensorApplication.h>
#include <com/osteres/automation/transmission/Transmitter.h>


using com::osteres::automation::weathersensor::WeatherSensorApplication;
using com::osteres::automation::transmission::Transmitter;
using com::osteres::automation::transmission::packet::Packet;
using com::osteres::automation::transmission::packet::Command;

/* Pins CE, CSN for ARDUINO */
#define RF_CE    9
#define RF_CSN   10

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

/*
 * Prepare object manager
 */
// Transmission (master mode)
Transmitter transmitter(&radio, WeatherSensorApplication::SENSOR, true);
// Application
WeatherSensorApplication application(&lcd, &rtc, &transmitter);

/**
 * Initialize
 */
void setup() {
    Serial.begin(9600);

    // Setup transmitter
    transmitter.setup();

    // Setup
    application.setup();
    application.setIntervalScreenRefresh(100); // 0.1s
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