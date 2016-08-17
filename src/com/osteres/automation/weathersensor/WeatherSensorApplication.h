//
// Created by Thibault PLET on 04/05/2016.
//

#ifndef COM_OSTERES_AUTOMATION_WEATHERSENSOR_WEATHERSENSORAPPLICATION_H
#define COM_OSTERES_AUTOMATION_WEATHERSENSOR_WEATHERSENSORAPPLICATION_H

/* EEPROM address */
#define ADDR_COUNTER_TOTAL 0x0
#define ADDR_COUNTER_TEMP 0x4

/* Defined values */
#define LCD_WIDTH 16
#define LCD_HEIGHT 2
#define IGNORE_PACKET_SUCCESS_RESPONSE true


#include <Arduino.h>
#include <RTClib/RTClib.h>
#include <LiquidCrystal.h>
#include <DHT.h>
#include <com/osteres/util/formatter/Number.h>
#include <com/osteres/automation/Application.h>
#include <com/osteres/automation/sensor/Identity.h>
#include <com/osteres/automation/transmission/Transmitter.h>
#include <com/osteres/automation/transmission/Requester.h>
#include <com/osteres/automation/arduino/transmission/ArduinoRequester.h>
#include <com/osteres/automation/transmission/Receiver.h>
#include <com/osteres/automation/weathersensor/action/ActionManager.h>
#include <com/osteres/arduino/util/StringConverter.h>

using com::osteres::util::formatter::Number;
using com::osteres::automation::Application;
using com::osteres::automation::sensor::Identity;
using com::osteres::automation::transmission::Transmitter;
using com::osteres::automation::arduino::transmission::ArduinoRequester;
using com::osteres::automation::transmission::Receiver;
using com::osteres::automation::weathersensor::action::ActionManager;
using com::osteres::arduino::util::StringConverter;

namespace com
{
    namespace osteres
    {
        namespace automation
        {
            namespace weathersensor
            {
                class WeatherSensorApplication : public Application  {
                public:
                    /**
                     * Sensor identifier
                     */
                    static byte const SENSOR = Identity::WEATHER;

                    /**
                     * Constructor
                     */
                    WeatherSensorApplication(DHT * sensor, LiquidCrystal * screen, RTC_DS1307 * rtc, Transmitter * transmitter) {
                        this->screen = screen;
                        this->sensor = sensor;
                        this->rtc = rtc;
                        this->transmitter = transmitter;

                        // Init
                        this->intervalScreenRefresh1 = 1000 * 30; // 30s
                        this->intervalScreenRefresh2 = 1000 * 30; // 30s
                        this->timePointScreen1 = millis();
                        this->timePointScreen2 = millis();

                        // Create action manager
                        this->actionManager = new ActionManager(this->getScreen());
                    }

                    /**
                     * Destructor
                     */
                    ~WeatherSensorApplication()
                    {
                        // Remove action manager
                        if (this->actionManager != NULL) {
                            delete this->actionManager;
                            this->actionManager = NULL;
                        }
                    }

                    /**
                     * Setup application
                     */
                    virtual void setup() {

                        // Init rtc
                        this->rtc->begin();
                        //this->rtc->adjust(DateTime(2016, 6, 2, 21, 29, 00));

                        // Init DHT
                        this->sensor->begin();

                        // Init screen
                        this->screen->begin(16, 2);
                        this->screen->display();
                        this->displayScreenState1();
                        this->displayScreenState2();

                        // Transmission
                        static_cast<ArduinoRequester *>(this->transmitter->getRequester())->setRTC(this->getRtc());
                        this->transmitter->setActionManager(this->getActionManager());

                        Serial.println(F("WeatherSensorApplication: Setup executed."));
                    }

                    /**
                     * Process application
                     */
                    virtual void process() {

                        // Here, listen (action manager process packet received)
                        this->transmitter->listen();

                        // Refresh LCD every interval (line 1)
                        if (millis() - this->timePointScreen1 > this->getIntervalScreenRefresh1()) {
                            this->displayScreenState1();
                        }
                        // Refresh LCD every interval (line 2)
                        if (millis() - this->timePointScreen2 > this->getIntervalScreenRefresh2()) {
                            this->displayScreenState2();
                        }

                        // Wait 100ms
                        delay(100);
                    }

                    /**
                     * Display first line of screen
                     */
                    void displayScreenState1()
                    {
                        // Save instant point
                        this->timePointScreen1 = millis();

                        //
                        // First line: hour
                        //
                        this->cleanScreenLine(0);
                        DateTime now = this->getRtc()->now();
                        string s = "";
                        s += Number::twoDigit(now.hour()) +
                                ":" + Number::twoDigit(now.minute()) +
                                ":" + Number::twoDigit(now.second());
                        this->screen->setCursor(0, 0);
                        this->screen->write(s.c_str());
                    }

                    /**
                     * Display second line of screen
                     *
                     * Operation can take few seconds!
                     */
                    void displayScreenState2()
                    {
                        // Save instant point
                        this->timePointScreen2 = millis();

                        //
                        // Second line: temp and humidity
                        //
                        this->cleanScreenLine(1);
                        float h = this->getHumidity();
                        float t = this->getTemperature();
                        String s = "";

                        // Temp
                        s += "T:" + (isnan(t) ? String("-") : String(round(t * 10) / 10) + "*C");

                        s += " ";

                        // Humidity
                        s += "H:" + (isnan(h) ? String("-") : String(round(h)) + "%");

                        this->screen->setCursor(0, 1);
                        this->screen->write(s.c_str());
                    }

                    /**
                     * Clean line on screen
                     */
                    void cleanScreenLine(uint8_t line)
                    {
                        this->screen->setCursor(0, line);
                        String spaces = F("");
                        for (int i = 0; i < LCD_WIDTH ; i++) {
                            spaces += F(" ");
                        }
                        this->screen->write(spaces.c_str());
                    }

                    /**
                     * Get humidity value
                     */
                    float getHumidity(bool force = false)
                    {
                        return this->sensor->readHumidity(force);
                    }

                    /**
                     * Get temperature value
                     * If fahrenheit param set to true, return value in Fahrenheit unit
                     */
                    float getTemperature(bool fahrenheit = false, bool force = false)
                    {
                        return this->sensor->readTemperature(fahrenheit, force);
                    }

                    /**
                     * Get RTC
                     */
                    RTC_DS1307 * getRtc() {
                        return this->rtc;
                    }

                    /**
                     * Get action manager
                     */
                    ActionManager * getActionManager()
                    {
                        return this->actionManager;
                    }

                    /**
                     * Get screen
                     */
                    LiquidCrystal * getScreen() {
                        return this->screen;
                    }

                    /**
                     * Get sensor
                     */
                    DHT * getSensor() {
                        return this->sensor;
                    }

                    /**
                     * Set interval to refresh screen (line 1)
                     */
                    void setIntervalScreenRefresh1(unsigned int interval) {
                        this->intervalScreenRefresh1 = interval;
                    }

                    /**
                     * Get interval of screen refresh (line 1)
                     */
                    unsigned int getIntervalScreenRefresh1() {
                        return this->intervalScreenRefresh1;
                    }

                    /**
                     * Set interval to refresh screen (line 1)
                     */
                    void setIntervalScreenRefresh2(unsigned int interval) {
                        this->intervalScreenRefresh2 = interval;
                    }

                    /**
                     * Get interval of screen refresh (line 1)
                     */
                    unsigned int getIntervalScreenRefresh2() {
                        return this->intervalScreenRefresh2;
                    }

                protected:

                    /**
                     * RTC object for DateTime
                     */
                    RTC_DS1307 * rtc = NULL;

                    /**
                     * Screen
                     */
                    LiquidCrystal * screen = NULL;

                    /**
                     * Transmitter
                     */
                    Transmitter * transmitter = NULL;

                    /**
                     * Weather sensor
                     */
                    DHT * sensor = NULL;

                    /**
                     * Action manager
                     */
                    ActionManager * actionManager = NULL;

                    /**
                     * Time point for lcd display (line 1)
                     */
                    unsigned long timePointScreen1;

                    /**
                     * Time point for lcd display (line 2)
                     */
                    unsigned long timePointScreen2;

                    /**
                     * Interval for refresh screen (for line 1)
                     */
                    unsigned int intervalScreenRefresh1;

                    /**
                     * Interval for refresh screen (for line 2)
                     */
                    unsigned int intervalScreenRefresh2;

                };
            }
        }
    }
}


#endif //COM_OSTERES_AUTOMATION_WEATHERSENSOR_WEATHERSENSORAPPLICATION_H
