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
                    static byte const SENSOR = Identity::MASTER;

                    /**
                     * Constructor
                     */
                    WeatherSensorApplication(LiquidCrystal * screen, RTC_DS1307 * rtc, Transmitter * transmitter) {
                        this->screen = screen;
                        this->rtc = rtc;
                        this->transmitter = transmitter;

                        // Init
                        this->intervalScreenRefresh = 1000 * 30; // 30s
                        this->timePointScreen = millis();

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

                        // Init screen
                        this->screen->begin(16, 2);
                        this->screen->display();
                        this->displayScreenState();

                        // Transmission
                        static_cast<ArduinoRequester *>(this->transmitter->getRequester())->setRTC(this->getRtc());
                        //this->transmitter->setActionManager(this->getActionManager());

                        Serial.println(F("PacketDisplayApplication: Setup executed."));
                    }

                    /**
                     * Process application
                     */
                    virtual void process() {

                        // Here, listen (action manager process packet received)
                        this->transmitter->listen();

                        // Refresh LCD every interval
                        if (millis() - this->timePointScreen > this->getIntervalScreenRefresh()) {
                            this->displayScreenState();
                        }

                        // Wait 100ms
                        delay(100);
                    }

                    /**
                     * Display first line of screen
                     */
                    void displayScreenState()
                    {
                        // Save instant point
                        this->timePointScreen = millis();

                        // Clean first line by using ' ' (space)
                        this->cleanScreenLine(0);

                        // Get datetime
                        DateTime now = this->getRtc()->now();

                        // Content to display
                        string s = "";
                        s += Number::twoDigit(now.hour()) +
                                ":" + Number::twoDigit(now.minute()) +
                                ":" + Number::twoDigit(now.second());

                        // Display to screen
                        this->screen->setCursor(0, 0);
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
                     * Set interval to refresh screen
                     */
                    void setIntervalScreenRefresh(unsigned int interval) {
                        this->intervalScreenRefresh = interval;
                    }

                    /**
                     * Get interval of screen refresh
                     */
                    unsigned int getIntervalScreenRefresh() {
                        return this->intervalScreenRefresh;
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
                     * Action manager
                     */
                    ActionManager * actionManager = NULL;

                    /**
                     * Time point for lcd display
                     */
                    unsigned long timePointScreen;

                    /**
                     * Interval for refresh screen
                     */
                    unsigned int intervalScreenRefresh;

                };
            }
        }
    }
}


#endif //COM_OSTERES_AUTOMATION_WEATHERSENSOR_WEATHERSENSORAPPLICATION_H
