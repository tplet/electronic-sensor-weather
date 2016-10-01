//
// Created by Thibault PLET on 02/06/2016.
//

#ifndef COM_OSTERES_AUTOMATION_WEATHERSENSOR_ACTION_ACTIONMANAGER_H
#define COM_OSTERES_AUTOMATION_WEATHERSENSOR_ACTION_ACTIONMANAGER_H

#define LCD_WIDTH 16

#include <Arduino.h>
#include <LiquidCrystal.h>
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
                        ActionManager(LiquidCrystal * screen) : ArduinoActionManager() {
                            this->screen = screen;
                        }

                        /**
                         * Process packet
                         */
                        virtual void processPacket(Packet * packet)
                        {
                            // Parent
                            ArduinoActionManager::processPacket(packet);


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
                         * Get screen
                         */
                        LiquidCrystal * getScreen()
                        {
                            return this->screen;
                        }

                    protected:

                        /**
                         * Screen
                         */
                        LiquidCrystal * screen = NULL;
                    };
                }
            }
        }
    }
}

#endif //COM_OSTERES_AUTOMATION_WEATHERSENSOR_ACTION_ACTIONMANAGER_H
