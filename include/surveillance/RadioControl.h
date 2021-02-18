/*
 * @brief Defines a radio control system
 * @file surveillance/RadioControl.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <list>
#include <mutex>

#include <types/Flight.h>

namespace topskytower {
    namespace surveillance {
        /**
         * @brief Describes a Radio control system
         * @ingroup surveillance
         *
         * This module controls the different radio signals and tracks which
         * aircrafts are transmitting.
         */
        class RadioControl {
        private:
#ifndef DOXYGEN_IGNORE
            std::mutex             m_transmissionsLock;
            std::list<std::string> m_activeTransmissions;

            RadioControl();

        public:
            RadioControl(const RadioControl&) = delete;
            RadioControl(RadioControl&&) = delete;
            RadioControl& operator=(const RadioControl&) = delete;
            RadioControl& operator=(RadioControl&&) = delete;

            /**
             * @brief Updates the currently transmitting stations
             * @param[in] callsigns The transmitting stations
             */
            void transmissions(const std::vector<std::string>& callsigns);
            /**
             * @brief Checks if a specific flight is transmitting
             * @param[in] flight The requested flight
             * @return True if it is transmitting, else false
             */
            bool isTransmitting(const types::Flight& flight);
            /**
             * @brief Returns the callsigns of all transmitting flights
             * @return The list of all transmitting callsigns
             */
            std::list<std::string> transmittingFlights();
            /**
             * @brief Returns the current instance of the radio control
             * @return The radio control
             */
            static RadioControl& instance();
#endif
        };
    }
}
