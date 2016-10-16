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
#include <com/osteres/automation/arduino/memory/StoredProperty.h>
#include <com/osteres/automation/memory/Property.h>

using com::osteres::automation::weathersensor::component::WeatherBuffer;
using com::osteres::automation::action::Action;
using com::osteres::automation::transmission::Transmitter;
using com::osteres::automation::transmission::packet::Packet;
using com::osteres::automation::transmission::packet::Command;
using com::osteres::automation::memory::Property;
using com::osteres::automation::arduino::memory::StoredProperty;

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
                        TransmitWeatherValue(
                            Property<unsigned char> * propertyType,
                            StoredProperty<unsigned char> * propertyIdentifier,
                            unsigned char to,
                            Transmitter * transmitter,
                            WeatherBuffer * buffer
                        ) {
                            this->propertyType = propertyType;
                            this->propertyIdentifier = propertyIdentifier;
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

                            Packet * packet = new Packet(this->propertyType->get());

                            // Prepare data
                            packet->setSourceIdentifier(this->propertyIdentifier->get());
                            packet->setDataLong1(round(this->buffer->getTemperature() * 100)); // Temperature
                            packet->setDataLong2(round(this->buffer->getHumidity())); // Humidity
                            packet->setCommand(Command::DATA);
                            packet->setTarget(this->to);
                            packet->setLast(true);

                            // Transmit packet
                            this->transmitter->sendAndConfirm(packet);

                            // Free memory
                            delete packet;

                            this->setSuccess();
                            return this->isSuccess();
                        }

                    protected:
                        /**
                         * Sensor type identifier property
                         */
                        Property<unsigned char> * propertyType = NULL;

                        /**
                         * Sensor identifier property
                         */
                        StoredProperty<unsigned char> * propertyIdentifier = NULL;

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
