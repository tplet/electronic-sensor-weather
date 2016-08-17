//
// Created by Thibault PLET on 31/05/2016.
//

#ifndef COM_OSTERES_AUTOMATION_WEATHERSENSOR_COMPONENT_WEATHERBUFFER_H
#define COM_OSTERES_AUTOMATION_WEATHERSENSOR_COMPONENT_WEATHERBUFFER_H

#include <DHT.h>
#include <com/osteres/automation/arduino/component/DataBuffer.h>

using com::osteres::automation::arduino::component::DataBuffer;

namespace com {
    namespace osteres {
        namespace automation {
            namespace weathersensor {
                namespace component {
                    class WeatherBuffer : public DataBuffer {
                    public:
                        /**
                         * Constructor
                         */
                        WeatherBuffer(DHT * sensor) : DataBuffer()
                        {
                            this->sensor = sensor;
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
                         * Get sensor
                         */
                        DHT * getSensor()
                        {
                            return this->sensor;
                        }

                    protected:

                        /**
                         * Sensor to read temperature and humidity
                         */
                        DHT * sensor;
                    };
                }
            }
        }
    }
}


#endif //COM_OSTERES_AUTOMATION_WEATHERSENSOR_COMPONENT_WEATHERBUFFER_H
