//
// Created by Thibault PLET on 31/05/2016.
//

#ifndef COM_OSTERES_AUTOMATION_WEATHERSENSOR_ACTION_TRANSMITWEATHERVALUE_H
#define COM_OSTERES_AUTOMATION_WEATHERSENSOR_ACTION_TRANSMITWEATHERVALUE_H

#include <Arduino.h>
#include <com/osteres/automation/weathersensor/component/WeatherBuffer.h>
#include <com/osteres/automation/action/Action.h>
#include <com/osteres/automation/transmission/Transmitter.h>
#include <com/osteres/automation/transmission/packet/Packet.h>
#include <com/osteres/automation/transmission/packet/Command.h>

using com::osteres::automation::weathersensor::component::WeatherBuffer;
using com::osteres::automation::action::Action;
using com::osteres::automation::transmission::Transmitter;
using com::osteres::automation::transmission::packet::Packet;
using com::osteres::automation::transmission::packet::Command;

namespace com {
    namespace osteres {
        namespace automation {
            namespace weathersensor {
                namespace action {
                    class TransmitWeatherValue : public Action {
                    public:
                        /**
                         * Constructor
                         */
                        TransmitWeatherValue(unsigned char from, unsigned char to, Transmitter * transmitter, WeatherBuffer * buffer)
                        {
                            this->from = from;
                            this->to = to;
                            this->transmitter = transmitter;
                            this->buffer = buffer;
                        }

                        /**
                         * Execute action
                         */
                        bool execute()
                        {
                            Serial.println(F("Transmit to server action..."));
                            // parent
                            Action::execute();

                            Packet * packet = new Packet(this->from);

                            // Prepare data
                            packet->setDataLong1((long)round(this->buffer->getTemperature() * 100)); // Temperature
                            packet->setDataLong2((long)round(this->buffer->getHumidity())); // Hmidity
                            packet->setCommand(Command::DATA);
                            packet->setTarget(this->to);

                            // Transmit packet
                            this->transmitter->send(packet);

                            // Free memory
                            delete packet;

                            this->setSuccess();
                            return this->isSuccess();
                        }

                    protected:
                        /**
                         * Sensor identifier
                         */
                        unsigned char from;

                        /**
                         * Target of transmission
                         */
                        unsigned char to;

                        /**
                         * Transmitter gateway
                         */
                        Transmitter * transmitter = NULL;

                        /**
                         * Buffer when get value to transmit
                         */
                        WeatherBuffer * buffer = NULL;
                    };
                }
            }
        }
    }
}

#endif //COM_OSTERES_AUTOMATION_WEATHERSENSOR_ACTION_TRANSMITWEATHERVALUE_H
