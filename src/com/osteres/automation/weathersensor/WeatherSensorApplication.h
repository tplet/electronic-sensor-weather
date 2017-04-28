//
// Created by Thibault PLET on 04/05/2016.
//

#ifndef COM_OSTERES_AUTOMATION_WEATHERSENSOR_WEATHERSENSORAPPLICATION_H
#define COM_OSTERES_AUTOMATION_WEATHERSENSOR_WEATHERSENSORAPPLICATION_H

/* Defined values */
#define IGNORE_PACKET_SUCCESS_RESPONSE true
#define DATETIME_UPDATE 86400000 // 1 day
#define BATTERY_SEND 600000 // 10 minutes


#include <Arduino.h>
#include <StandardCplusplus.h>
#include <LiquidCrystal.h>
#include <DHT.h>
#include <string>
#include <com/osteres/util/formatter/Number.h>
#include <com/osteres/automation/arduino/ArduinoApplication.h>
#include <com/osteres/automation/sensor/Identity.h>
#include <com/osteres/automation/transmission/Requester.h>
#include <com/osteres/automation/transmission/Receiver.h>
#include <com/osteres/automation/weathersensor/action/ActionManager.h>
#include <com/osteres/arduino/util/StringConverter.h>
#include <com/osteres/automation/weathersensor/component/WeatherBuffer.h>
#include <com/osteres/automation/arduino/component/DataBuffer.h>
#include <com/osteres/automation/weathersensor/action/TransmitWeatherValue.h>
#include <com/osteres/automation/arduino/component/Screen.h>

using com::osteres::util::formatter::Number;
using com::osteres::automation::arduino::ArduinoApplication;
using com::osteres::automation::sensor::Identity;
using com::osteres::automation::transmission::Receiver;
using com::osteres::automation::weathersensor::action::ActionManager;
using com::osteres::arduino::util::StringConverter;
using com::osteres::automation::weathersensor::component::WeatherBuffer;
using com::osteres::automation::arduino::component::DataBuffer;
using com::osteres::automation::weathersensor::action::TransmitWeatherValue;
using com::osteres::automation::arduino::component::Screen;
using std::string;

namespace com
{
    namespace osteres
    {
        namespace automation
        {
            namespace weathersensor
            {
                class WeatherSensorApplication : public ArduinoApplication  {
                public:
                    /**
                     * Sensor identifier
                     */
                    static byte const SENSOR = Identity::WEATHER;

                    /**
                     * Constructor
                     */
                    WeatherSensorApplication(
                        Transmitter * transmitter,
                        RTC_DS1307 * rtc,
                        DHT * sensor,
                        LiquidCrystal * screenDevice
                    ) : ArduinoApplication(WeatherSensorApplication::SENSOR, transmitter, rtc)
                    {
                        this->construct(sensor, screenDevice);
                    }

                    /**
                     * Constructor
                     */
                    WeatherSensorApplication(
                        Transmitter * transmitter,
                        RTC_DS1307 * rtc,
                        DHT * sensor
                    ) : ArduinoApplication(WeatherSensorApplication::SENSOR, transmitter, rtc)
                    {
                        this->construct(sensor);
                    }

                    /**
                     * Destructor
                     */
                    ~WeatherSensorApplication()
                    {
                        // Remove weather buffer
                        if (this->weatherBuffer != NULL) {
                            delete this->weatherBuffer;
                            this->weatherBuffer = NULL;
                        }
                        if (this->pointBatteryBuffer != NULL) {
                            delete this->pointBatteryBuffer;
                            this->pointBatteryBuffer = NULL;
                        }
                        // Remove datetime point buffer
                        if (this->pointDateTimeBuffer != NULL) {
                            delete this->pointDateTimeBuffer;
                            this->pointDateTimeBuffer = NULL;
                        }
                        // Remove screen 1 point buffer
                        if (this->pointScreen1Buffer != NULL) {
                            delete this->pointScreen1Buffer;
                            this->pointScreen1Buffer = NULL;
                        }
                        // Remove screen 2 point buffer
                        if (this->pointScreen2Buffer != NULL) {
                            delete this->pointScreen2Buffer;
                            this->pointScreen2Buffer = NULL;
                        }
                        // Remove weather action
                        if (this->actionWeather != NULL) {
                            delete this->actionWeather;
                            this->actionWeather = NULL;
                        }
                        // Remove screen
                        if (this->screen != NULL) {
                            delete this->screen;
                            this->screen = NULL;
                        }
                    }

                    /**
                     * Setup application
                     */
                    virtual void setup()
                    {
                        // Parent
                        ArduinoApplication::setup();

                        // Init DHT
                        this->weatherBuffer->getSensor()->begin();

                        // Display firsts lines
                        if (this->hasScreen() && this->getScreen()->isEnabled()) {
                            this->getScreen()->detectSwitch();
                            this->displayScreenState1();
                            this->displayScreenState2();

                            this->pointScreen1Buffer->reset();
                            this->pointScreen2Buffer->reset();
                        }

                        // Transmission
                        this->transmitter->setActionManager(this->getActionManager());

                        Serial.println(F("WeatherSensorApplication: Setup executed."));
                    }

                    /**
                     * Process application
                     */
                    virtual void process()
                    {
                        // Request an identifier if needed. Note: Not mandatory anymore
                        if (this->isNeedIdentifier()) {
                            this->requestForAnIdentifier();

                            // Send and listen
                            this->transmitter->srs(3000); // 3s

                        } // Process
                        else {

                            // Test data buffer (only if identifier has been allocated)
                            if (!this->isNeedIdentifier() && this->weatherBuffer->isOutdated()) {
                                this->requestForSendData();
                            }

                            // Refresh DateTime from server (every day)
                            if (this->pointDateTimeBuffer->isOutdated()) {
                                this->requestForDateTime();
                                this->pointDateTimeBuffer->reset();
                            }

                            // Send battery level
                            if (this->pointBatteryBuffer->isOutdated()) {
                                this->requestForBatteryLevel();
                                this->pointBatteryBuffer->reset();
                            }

                            // Send and listen
                            this->transmitter->srs();
                        }

                        // Screen treatment
                        if (this->hasScreen()) {
                            Screen * screen = this->getScreen();

                            // Switch detection
                            screen->detectSwitch();

                            // Display if enabled
                            if (screen->isEnabled()) {
                                // Refresh LCD every interval (line 1)
                                if (this->pointScreen1Buffer->isOutdated()) {
                                    this->pointScreen1Buffer->reset();
                                    this->displayScreenState1();
                                }
                                // Refresh LCD every interval (line 2)
                                if (this->pointScreen2Buffer->isOutdated()) {
                                    this->pointScreen2Buffer->reset();
                                    this->displayScreenState2();
                                }
                            }
                        }

                        // Wait 100ms
                        delay(100);
                    }

                    /**
                     * Send data to server
                     */
                    void requestForSendData()
                    {
                        // Process
                        this->getActionWeather()->execute();
                        this->weatherBuffer->reset();
                    }

                    /**
                     * Display first line of screen
                     */
                    void displayScreenState1()
                    {
                        if (this->hasScreen() && this->getScreen()->isEnabled()) {
                            Screen * screen = this->getScreen();

                            //
                            // First line: hour + battery level + identifier
                            //
                            string output;
                            // Display hour
                            DateTime now = this->getRTC()->now();
                            string sHour = "";
                            sHour += Number::twoDigit(now.hour()) +
                                     ":" + Number::twoDigit(now.minute()) +
                                     ":" + Number::twoDigit(now.second());
                            output += sHour;

                            // Display battery level
                            if (this->hasBatteryLevel()) {
                                float bRatio = this->getBatteryLevel()->getRatio();
                                unsigned int percent = (unsigned int)round(bRatio * 100);
                                output += " " + Number::twoDigit(percent) + "%";
                            }

                            // Display identifier
                            unsigned char id = this->getPropertyIdentifier()->get();
                            string sId = Number::twoDigit(id);
                            string sSpace = "";
                            for (unsigned char i = 0; i < screen->getWidth() - output.length() - sId.length(); i++) {
                                sSpace += " ";
                            }
                            output += sSpace + sId;

                            // Write to screen
                            this->cleanScreenLine(0);
                            screen->setCursor(0, 0);
                            screen->write(output.c_str());
                        }
                    }

                    /**
                     * Display second line of screen
                     *
                     * Operation can take few seconds!
                     */
                    void displayScreenState2()
                    {
                        if (this->hasScreen() && this->getScreen()->isEnabled()) {
                            Screen * screen = this->getScreen();

                            //
                            // Second line: temp and humidity
                            //
                            String output = "";

                            // Temp
                            float t = this->weatherBuffer->getTemperature();
                            output += "T:" + (isnan(t) ? String("-") : String(round(t * 10) / (double) 10, 1) + "*C");
                            output += " ";

                            // Humidity
                            float h = this->weatherBuffer->getHumidity();
                            output += "H:" + (isnan(h) ? String("-") : String((double) round(h), 0) + "%");

                            // Battery voltage
                            if (this->hasBatteryLevel()) {
                                output = "";
                                output += "B:" + String(round(this->getBatteryLevel()->getVoltage() * 100) / (double)100, 2) + "V";
                            }

                            this->cleanScreenLine(1);
                            screen->setCursor(0, 1);
                            screen->write(output.c_str());
                        }
                    }

                    /**
                     * Clean line on screen
                     */
                    void cleanScreenLine(uint8_t line)
                    {
                        if (this->hasScreen() && this->getScreen()->isEnabled()) {
                            Screen * screen = this->getScreen();

                            screen->setCursor(0, line);
                            String spaces = F("");
                            for (int i = 0; i < screen->getWidth() ; i++) {
                                spaces += F(" ");
                            }
                            screen->write(spaces.c_str());
                        }
                    }

                    /**
                     * Get weather buffer
                     */
                    WeatherBuffer * getWeatherBuffer()
                    {
                        return this->weatherBuffer;
                    }

                    /**
                     * Get action weather
                     */
                    TransmitWeatherValue * getActionWeather()
                    {
                        if (this->actionWeather == NULL) {
                            this->actionWeather = new TransmitWeatherValue(
                                this->getPropertyType(),
                                this->getPropertyIdentifier(),
                                Identity::MASTER,
                                this->transmitter,
                                this->weatherBuffer
                            );
                        }

                        return this->actionWeather;
                    }

                    /**
                     * Get screen component
                     */
                    Screen * getScreen()
                    {
                        return this->screen;
                    }

                    /**
                     * Flag to indicate if screen exists
                     */
                    bool hasScreen()
                    {
                        return this->screen != NULL;
                    }

                    /**
                     * Get Screen point buffer (line 1)
                     */
                    DataBuffer * getPointScreen1Buffer()
                    {
                        return this->pointScreen1Buffer;
                    }

                    /**
                     * Get Screen point buffer (line 2)
                     */
                    DataBuffer * getPointScreen2Buffer()
                    {
                        return this->pointScreen2Buffer;
                    }

                    /**
                     * Get battery buffer point
                     */
                    DataBuffer * getPointBatteryBuffer()
                    {
                        return this->pointBatteryBuffer;
                    }

                protected:

                    /**
                     * Common part constructor
                     */
                    void construct(DHT * sensor)
                    {
                        // Create action manager
                        ActionManager * actionManager = new ActionManager();
                        this->setActionManager(actionManager);

                        // Weather buffer
                        this->weatherBuffer = new WeatherBuffer(sensor);

                        // Battery buffer
                        this->pointBatteryBuffer = new DataBuffer(BATTERY_SEND, 5000); // 5s after boot

                        // Buffer points
                        this->pointDateTimeBuffer = new DataBuffer(DATETIME_UPDATE, 10000); // 10s after boot
                        this->pointScreen1Buffer = new DataBuffer(1000 * 30); // 30s
                        this->pointScreen2Buffer = new DataBuffer(1000 * 30); // 30s
                    }

                    /**
                     * Common part constructor
                     */
                    void construct(DHT * sensor, LiquidCrystal * screenDevice)
                    {
                        this->construct(sensor);

                        // Create screen component
                        this->screen = new Screen(screenDevice);
                    }

                    /**
                     * Weather buffer
                     */
                    WeatherBuffer * weatherBuffer = NULL;

                    /**
                     * Battery buffer point
                     */
                    DataBuffer * pointBatteryBuffer = NULL;

                    /**
                     * DateTime point buffer
                     */
                    DataBuffer * pointDateTimeBuffer = NULL;

                    /**
                     * Screen point buffer (line 1)
                     */
                    DataBuffer * pointScreen1Buffer = NULL;

                    /**
                     * Screen point buffer (line 2)
                     */
                    DataBuffer * pointScreen2Buffer = NULL;

                    /**
                     * Action to transmit weather
                     */
                    TransmitWeatherValue * actionWeather = NULL;

                    /**
                     * Screen component
                     */
                    Screen * screen = NULL;

                };
            }
        }
    }
}


#endif //COM_OSTERES_AUTOMATION_WEATHERSENSOR_WEATHERSENSORAPPLICATION_H
