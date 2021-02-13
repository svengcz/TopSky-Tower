/*
 * @brief Defines a flight plan control system
 * @file surveillance/FlightPlanControl.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <list>
#include <map>
#include <string>

#include <system/ConfigurationRegistry.h>
#include <types/Flight.h>

namespace topskytower {
    namespace surveillance {
        /**
         * @brief Describes a flight plan control system
         * @ingroup surveillance
         *
         * This module checks the flight plan based on flight levels, the even-odd-rule, navigation capabilities and engine types.
         * Additionally is the definition of destination constraints possible.
         * This allows to deactivate some generic checks, like flight levels or even-odd-changes.
         *
         * ![Flight-plan check](doc/imgs/FPCheck.png)
         *
         * The flight plan checker shows abbreviation:
         *  - VFR - Defined as VFR flight
         *  - RTE - Invalid route with no SID exit point
         *  - SID - Unknown SID
         *  - FL - Does not match flight level constraints
         *  - E/O - Even-odd-rule does not fit  
         *  - ENG - Wrong engine type
         *  - NAV - Wrong navigation capabilities
         *  - XDR - Wrong transponder capabilities
         *  - UNK - Something else went wrong
         *  - ERR - Multiple flight plan errors detected
         *
         * The checker provides a simple menu that activates an flight plan error-log reader and an overwrite-function.
         * The overwrite functions marks the flight plan as valid independent from error log.
         *
         * ![Flight-plan menu](doc/imgs/FPCheckMenu.png)
         *
         * ![Flight-plan log](doc/imgs/FPCheckErrorLog.png)
         *
         * If a flight plan is marked as valid or is overwritten by the controller is the initial clearance limit set
         * and the flight plan changed in that way that all waypoints before the SIDs exit point are deleted and the
         * SID with the departure runway is added to the flight plan.
         * The flight plan checker provides a marker if the SID contains climb constraints and the phraseology requires
         * the "climb via SID" call.
         */
        class FlightPlanControl {
        public:
#ifndef DOXYGEN_IGNORE
            /**
             * @brief Define sthe error codes after a flight plan validation
             */
            enum class ErrorCode {
                Unknown        = 0x00, /**< The flight plan is not analyzed */
                VFR            = 0x01, /**< It is an VFR flight */
                NoError        = 0x02, /**< No error found */
                Route          = 0x03, /**< No valid route found */
                DepartureRoute = 0x04, /**< The SID is unknown */
                EngineType     = 0x05, /**< The engine type does not match */
                Navigation     = 0x06, /**< The navigation type (RNAV) does not match */
                Transponder    = 0x07, /**< The transponder does not match */
                FlightLevel    = 0x08, /**< The minimum or maximum flight level does not match */
                EvenOddLevel   = 0x09, /**< The flight level does not match to the even/odd constraint */
            };

        private:
            struct FlightPlanStatus {
                std::list<ErrorCode>    errorCodes;
                bool                    overwritten;
                std::string             destination;
                std::string             departureRoute;
                types::FlightPlan::Type type;
                bool                    rnavCapable;
                bool                    transponderAvailable;
                types::Length           requestedFlightLevel;

                FlightPlanStatus() :
                        errorCodes(),
                        overwritten(false),
                        destination(),
                        departureRoute(),
                        type(types::FlightPlan::Type::Unknown),
                        rnavCapable(false),
                        transponderAvailable(false),
                        requestedFlightLevel() { }
            };

            std::map<std::string, FlightPlanStatus> m_flightChecks;

            FlightPlanControl();

            void reinitialize(system::ConfigurationRegistry::UpdateType type);

        public:
            /**
             * Destroys all internal structures
             */
            ~FlightPlanControl();

            /**
             * @brief The flight plan of a flight will be validated
             * @param[in] flight The converted flight structure
             * @return True if a complete validation run was performed, else false (i.e. no FP change detected)
             */
            bool validate(const types::Flight& flight);
            /**
             * @brief Removes a flight out of the list
             * @param[in] callsign The flight's callsign
             */
            void removeFlight(const std::string& callsign);
            /**
             * @brief Marks an invalid flight plan as overwritten by the controller
             * @param[in] callsign The flight's callsign
             */
            void overwrite(const std::string& callsign);
            /**
             * @brief Returns the error codes of a flight plan
             * @param[in] callsign The flight's callsign
             * @return The error codes of the corresponding flight plan
             */
            const std::list<ErrorCode>& errorCodes(const std::string& callsign) const;
            /**
             * @brief Returns if a check is overwritten or not
             * @param[in] callsign The flight's callsign
             * @return True if the check is overwritten, else false
             */
            bool overwritten(const std::string& callsign) const;
            /**
             * @brief Returns the static instance of the flight plan control
             * @return The flight plan control
             */
            static FlightPlanControl& instance();
#endif
        };
    }
}
