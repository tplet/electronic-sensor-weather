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
#include <DHT.h>
#include <com/osteres/automation/arduino/ArduinoApplication.h>
#include <com/osteres/automation/sensor/Identity.h>
#include <com/osteres/automation/weathersensor/action/ActionManager.h>
#include <com/osteres/automation/weathersensor/component/WeatherBuffer.h>
#include <com/osteres/automation/arduino/component/DataBuffer.h>
#include <com/osteres/automation/weathersensor/action/TransmitWeatherValue.h>
#include <com/osteres/automation/arduino/display/Output.h>

using com::osteres::automation::arduino::ArduinoApplication;
using com::osteres::automation::sensor::Identity;
using com::osteres::automation::weathersensor::action::ActionManager;
using com::osteres::automation::weathersensor::component::WeatherBuffer;
using com::osteres::automation::arduino::component::DataBuffer;
using com::osteres::automation::weathersensor::action::TransmitWeatherValue;
using com::osteres::automation::arduino::display::Output;

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
                        Output * output
                    ) : ArduinoApplication(WeatherSensorApplication::SENSOR, transmitter, rtc)
                    {
                        this->construct(sensor, output);
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
                        // Remove weather action
                        if (this->actionWeather != NULL) {
                            delete this->actionWeather;
                            this->actionWeather = NULL;
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

                        // Output
                        if (this->hasOutput()) {
                            this->getOutput()->setup();
                        }

                        // Transmission
                        this->transmitter->setActionManager(this->getActionManager());
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

                        // Output treatment
                        if (this->hasOutput()) {
                            this->getOutput()->loop();
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
                     * Get battery buffer point
                     */
                    DataBuffer * getPointBatteryBuffer()
                    {
                        return this->pointBatteryBuffer;
                    }

                    /**
                     * Flag to indicate if an ouput is defined
                     */
                    bool hasOutput()
                    {
                        return this->output != NULL;
                    }

                    /**
                     * Set output to display
                     */
                    void setOutput(Output * output)
                    {
                        this->output = output;
                    }

                    /**
                     * Get output display
                     */
                    Output * getOutput()
                    {
                        return this->output;
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
                    }

                    /**
                     * Common part constructor
                     */
                    void construct(DHT * sensor, Output * output)
                    {
                        this->construct(sensor);
                        this->setOutput(output);
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
                     * Action to transmit weather
                     */
                    TransmitWeatherValue * actionWeather = NULL;

                    /**
                     * Output display
                     */
                    Output * output = NULL;

                };
            }
        }
    }
}


#endif //COM_OSTERES_AUTOMATION_WEATHERSENSOR_WEATHERSENSORAPPLICATION_H
