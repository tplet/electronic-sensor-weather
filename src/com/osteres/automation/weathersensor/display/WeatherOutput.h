//
// Created by Thibault PLET on 01/05/2017.
//

#ifndef COM_OSTERES_AUTOMATION_WEATHERSENSOR_DISPLAY_WEATHEROUTPUT_H
#define COM_OSTERES_AUTOMATION_WEATHERSENSOR_DISPLAY_WEATHEROUTPUT_H

#include <RTClib/RTClib.h>
#include <com/osteres/automation/arduino/display/Output.h>
#include <com/osteres/automation/arduino/display/device/Screen.h>
#include <com/osteres/automation/arduino/component/BatteryLevel.h>
#include <com/osteres/automation/arduino/memory/StoredProperty.h>
#include <com/osteres/automation/weathersensor/component/WeatherBuffer.h>
#include <com/osteres/util/formatter/Number.h>
#include <com/osteres/automation/arduino/component/DataBuffer.h>

using com::osteres::automation::arduino::display::Output;
using com::osteres::automation::arduino::display::device::Screen;
using com::osteres::automation::arduino::component::BatteryLevel;
using com::osteres::automation::arduino::memory::StoredProperty;
using com::osteres::automation::weathersensor::component::WeatherBuffer;
using com::osteres::automation::arduino::component::DataBuffer;
using com::osteres::util::formatter::Number;

namespace com
{
    namespace osteres
    {
        namespace automation
        {
            namespace weathersensor
            {
                namespace display
                {
                    class WeatherOutput : public Output {
                    public:
                        /**
                         * Constructor
                         *
                         * @param device Screen device associated
                         */
                        WeatherOutput(
                            Screen * device,
                            RTC_DS1307 * rtc,
                            BatteryLevel * batteryLevel,
                            StoredProperty<unsigned char> * propertyIdentifier,
                            WeatherBuffer * weatherBuffer
                        ) : Output(device)
                        {
                            this->setRTC(rtc);
                            this->setBatteryLevel(batteryLevel);
                            this->setPropertyIdentifier(propertyIdentifier);
                            this->setWeatherBuffer(weatherBuffer);

                            // Buffer point
                            this->pointScreen1Buffer = new DataBuffer(1000 * 30); // 30s
                            this->pointScreen2Buffer = new DataBuffer(1000 * 30); // 30s
                        }

                        ~WeatherOutput()
                        {
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
                        }

                        /**
                         * Setup out
                         */
                        virtual void setup()
                        {
                            // Display firsts lines
                            if (this->hasDevice() && this->getDevice()->isEnabled()) {
                                static_cast<Screen *>(this->getDevice())->detectSwitch();
                                this->print();

                                this->pointScreen1Buffer->reset();
                                this->pointScreen2Buffer->reset();
                            }

                        }

                        /**
                         * Force refresh screen
                         */
                        virtual void print()
                        {
                            this->displayScreenState1();
                            this->displayScreenState2();
                        }

                        /**
                         * Loop
                         */
                        virtual void loop()
                        {
                            // Screen treatment
                            if (this->hasDevice()) {
                                Screen * screen = static_cast<Screen *>(this->getDevice());

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
                        }

                        /**
                         * Flag to indicate if rtc is defined
                         */
                        bool hasRTC()
                        {
                            return this->rtc != NULL;
                        }

                        /**
                         * Set rtc
                         */
                        void setRTC(RTC_DS1307 * rtc)
                        {
                            this->rtc = rtc;
                        }

                        /**
                         * Get rtc
                         */
                        RTC_DS1307 * getRTC()
                        {
                            return this->rtc;
                        }

                        /**
                         * Flag to indicate if battery level component exists
                         */
                        bool hasBatteryLevel()
                        {
                            return this->batteryLevel != NULL;
                        }

                        /**
                         * Set battery level component
                         */
                        void setBatteryLevel(BatteryLevel * batteryLevel)
                        {
                            this->batteryLevel = batteryLevel;
                        }

                        /**
                         * Get battery level component
                         */
                        BatteryLevel * getBatteryLevel()
                        {
                            return this->batteryLevel;
                        }

                        /**
                         * Flag to indicate if sendor identifier property is defined
                         */
                        bool hasPropertyIdentifier()
                        {
                            return this->propertyIdentifier != NULL;
                        }

                        /**
                         * Set sensor identifier property
                         */
                        void setPropertyIdentifier(StoredProperty<unsigned char> * property)
                        {
                            this->propertyIdentifier = property;
                        }

                        /**
                         * Get sensor identifier property
                         */
                        StoredProperty<unsigned char> * getPropertyIdentifier()
                        {
                            return this->propertyIdentifier;
                        }

                        /**
                         * Flag to indicate if weather buffer is defined
                         */
                        bool hasWeatherBuffer()
                        {
                            return this->weatherBuffer != NULL;
                        }

                        /**
                         * Set weather buffer
                         */
                        void setWeatherBuffer(WeatherBuffer * weatherBuffer)
                        {
                            this->weatherBuffer = weatherBuffer;
                        }

                        /**
                         * Get weather buffer
                         */
                        WeatherBuffer * getWeatherBuffer()
                        {
                            return this->weatherBuffer;
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
                    protected:

                        /**
                         * Display first line of screen
                         */
                        void displayScreenState1()
                        {
                            if (this->hasDevice() && this->getDevice()->isEnabled()) {
                                Screen * screen = static_cast<Screen *>(this->getDevice());

                                //
                                // First line: hour + battery level + identifier
                                //
                                String output = "";

                                // Display hour
                                if (this->hasRTC()) {
                                    DateTime now = this->getRTC()->now();
                                    output += String(Number::twoDigit(now.hour()).c_str()) +
                                              ":" + String(Number::twoDigit(now.minute()).c_str()) +
                                              ":" + String(Number::twoDigit(now.second()).c_str());
                                }

                                // Display battery level
                                if (this->hasBatteryLevel()) {
                                    float bRatio = this->getBatteryLevel()->getRatio();
                                    unsigned int percent = (unsigned int)round(bRatio * 100);
                                    output += " " + String(Number::twoDigit(percent).c_str()) + "%";
                                }

                                // Display identifier
                                if (this->hasPropertyIdentifier()) {
                                    unsigned char id = this->getPropertyIdentifier()->get();
                                    String sId = String(Number::twoDigit(id).c_str());
                                    String sSpace = "";
                                    for (unsigned char i = 0;
                                         i < screen->getWidth() - output.length() - sId.length(); i++) {
                                        sSpace += " ";
                                    }
                                    output += sSpace + sId;
                                }

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
                            if (this->hasDevice() && this->getDevice()->isEnabled()) {
                                Screen * screen = static_cast<Screen *>(this->getDevice());

                                //
                                // Second line: temp and humidity
                                //
                                String output = "";

                                if (this->hasWeatherBuffer()) {
                                    // Temp
                                    float t = this->getWeatherBuffer()->getTemperature();
                                    output +=
                                        "T:" + (isnan(t) ? String("-") : String(round(t * 10) / (double) 10, 1) + "*C");
                                    output += " ";

                                    // Humidity
                                    float h = this->getWeatherBuffer()->getHumidity();
                                    output += "H:" + (isnan(h) ? String("-") : String((double) round(h), 0) + "%");
                                }

                                // Battery voltage
                                if (this->hasBatteryLevel()) {
                                    output = "";
                                    output += "P:" +  String(round(this->getBatteryLevel()->getPinVoltage() * 100) / (double)100, 2) + "V ";
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
                            if (this->hasDevice() && this->getDevice()->isEnabled()) {
                                Screen * screen = static_cast<Screen *>(this->getDevice());

                                screen->setCursor(0, line);
                                String spaces = F("");
                                for (int i = 0; i < screen->getWidth() ; i++) {
                                    spaces += F(" ");
                                }
                                screen->write(spaces.c_str());
                            }
                        }

                        /**
                         * RTC object for DateTime
                         */
                        RTC_DS1307 * rtc = NULL;

                        /**
                         * Battery level component
                         */
                        BatteryLevel * batteryLevel = NULL;

                        /**
                         * Sensor identifier property
                         */
                        StoredProperty<unsigned char> * propertyIdentifier = NULL;

                        /**
                         * Weather buffer
                         */
                        WeatherBuffer * weatherBuffer = NULL;

                        /**
                         * Screen point buffer (line 1)
                         */
                        DataBuffer * pointScreen1Buffer = NULL;

                        /**
                         * Screen point buffer (line 2)
                         */
                        DataBuffer * pointScreen2Buffer = NULL;

                    };
                }
            }
        }
    }
}


#endif //COM_OSTERES_AUTOMATION_WEATHERSENSOR_DISPLAY_WEATHEROUTPUT_H
