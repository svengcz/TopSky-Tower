/*
 * @brief Defines a flight plan control system
 * @file surveillance/FlightPlanControl.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <list>
#include <map>
#include <string>

#include <types/Flight.h>

namespace topskytower {
    namespace surveillance {
        /**
         * @brief Describes a flight plan control system
         * @ingroup surveillance
         */
        class FlightPlanControl {
        public:
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
                types::Length           requestedFlightLevel;
            };

            std::map<std::string, FlightPlanStatus> m_flightChecks;

            FlightPlanControl();

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
        };
    }
}
