//
// Created by Thibault PLET on 02/06/2016.
//

#ifndef COM_OSTERES_AUTOMATION_WEATHERSENSOR_ACTION_ACTIONMANAGER_H
#define COM_OSTERES_AUTOMATION_WEATHERSENSOR_ACTION_ACTIONMANAGER_H

#define LCD_WIDTH 16

#include <Arduino.h>
#include <LiquidCrystal.h>
#include <com/osteres/automation/transmission/packet/Command.h>
#include <com/osteres/automation/transmission/packet/CommandString.h>
#include <com/osteres/automation/transmission/packet/Packet.h>
#include <com/osteres/automation/action/ActionManagerBase.h>

using com::osteres::automation::transmission::packet::Command;
using com::osteres::automation::transmission::packet::CommandString;
using com::osteres::automation::transmission::packet::Packet;
using com::osteres::automation::action::ActionManagerBase;

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
                    class ActionManager : public ActionManagerBase
                    {
                    public:
                        /**
                         * Constructor
                         */
                        ActionManager(LiquidCrystal * screen)
                        {
                            this->screen = screen;
                        }

                        /**
                         * Process packet
                         */
                        virtual void processPacket(Packet * packet)
                        {
                            ActionManagerBase::processPacket(packet);

                            String text = CommandString::toString(packet->getCommand());
                            switch (packet->getCommand()) {
                                case Command::DATA:
                                    text += ":" + String(packet->getDataLong1());
                                    break;
                                default:
                                    break;
                            }

                            this->displayScreenPacket(text);
                        }

                        /**
                         * Display packet on second line of screen
                         */
                        void displayScreenPacket(String text)
                        {
                            this->cleanScreenLine(1);
                            // Position to 2nd line
                            this->screen->setCursor(0, 1);
                            this->screen->write((text).c_str());
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
