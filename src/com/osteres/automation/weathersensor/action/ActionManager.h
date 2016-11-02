//
// Created by Thibault PLET on 02/06/2016.
//

#ifndef COM_OSTERES_AUTOMATION_WEATHERSENSOR_ACTION_ACTIONMANAGER_H
#define COM_OSTERES_AUTOMATION_WEATHERSENSOR_ACTION_ACTIONMANAGER_H

#define LCD_WIDTH 16

#include <Arduino.h>
#include <StandardCplusplus.h>
#include <string>
#include <com/osteres/automation/transmission/packet/Command.h>
#include <com/osteres/automation/transmission/packet/CommandString.h>
#include <com/osteres/automation/transmission/packet/Packet.h>
#include <com/osteres/automation/arduino/action/ArduinoActionManager.h>
#include <com/osteres/automation/arduino/action/SensorIdentifierAction.h>

using com::osteres::automation::transmission::packet::Command;
using com::osteres::automation::transmission::packet::CommandString;
using com::osteres::automation::transmission::packet::Packet;
using com::osteres::automation::arduino::action::ArduinoActionManager;
using com::osteres::automation::arduino::action::SensorIdentifierAction;
using std::string;

namespace com
{
    namespace osteres
    {
        namespace automation
        {
            namespace weathersensor
            {
                namespace action
                {
                    class ActionManager : public ArduinoActionManager
                    {
                    public:
                        /**
                         * Constructor
                         */
                        ActionManager() : ArduinoActionManager() {
                        }

                        /**
                         * Process packet
                         */
                        virtual void processPacket(Packet * packet)
                        {
                            // Parent
                            ArduinoActionManager::processPacket(packet);


                        }

                    protected:

                    };
                }
            }
        }
    }
}

#endif //COM_OSTERES_AUTOMATION_WEATHERSENSOR_ACTION_ACTIONMANAGER_H
