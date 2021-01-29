/*
 * @brief Defines a flight plan structure
 * @file types/FlightPlan.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <string>

#include <types/Aircraft.h>
#include <types/Quantity.hpp>
#include <types/Route.h>

namespace topskytower {
    namespace types {
        /**
         * @brief Describes a flight plan structure
         * @ingroup types
         */
        class FlightPlan {
        public:
            /**
             * @brief Defines the flight plan type
             */
            enum class Type {
                Unknown = 0, /**< A flight plan type is not defined */
                VFR     = 1, /**< The flight will be a VFR flight */
                IFR     = 2  /**< The flight will be an IFR flight */
            };

            /**
             * @brief Defines the different ATC command flags
             */
            enum class AtcCommand {
                Unknown   = 0x000, /**< No or an unknown command given */
                StartUp   = 0x001, /**< The flight received the start-up clearance */
                Deicing   = 0x002, /**< The flight performs deicing */
                Pushback  = 0x004, /**< The flight performs a pushback */
                TaxiOut   = 0x008, /**< The flight is taxiing as a departure */
                LineUp    = 0x010, /**< The flight is lining up */
                Departure = 0x020, /**< The flight is departing */
                Approach  = 0x100, /**< The flight is approaching */
                Land      = 0x200, /**< The flight is landing */
                GoAround  = 0x400, /**< The flight is going around */
                TaxiIn    = 0x800, /**< The flight is taxiing as an arrival */
            };

        private:
            Type            m_type;
            types::Aircraft m_aircraft;
            std::uint16_t   m_atcCommand;
            std::string     m_origin;
            std::string     m_departureRoute;
            std::string     m_departureRunway;
            std::string     m_destination;
            std::string     m_arrivalRunway;
            types::Length   m_flightLevel;
            std::string     m_arrivalRoute;
            std::uint16_t   m_assignedSquawk;
            types::Length   m_clearanceLimit;
            bool            m_clearanceFlags;
            bool            m_rnavCapable;
            bool            m_transponderExists;
            Route           m_route;

        public:
            /**
             * @brief Creates a flight plan
             */
            FlightPlan();

            /**
             * @brief Sets the flight plan type
             * @param[in] type The new type
             */
            void setType(Type type);
            /**
             * @brief Returns the flight plan's type
             * @return The flight plan type
             */
            Type type() const;
            /**
             * @brief Sets the aircraft
             * @param[in] aircraft The new aircraft
             */
            void setAircraft(const types::Aircraft& aircraft);
            /**
             * @brief Returns the registered aircraft
             * @return The aircraft
             */
            const types::Aircraft& aircraft() const;
            /**
             * @brief Sets the origin of the flight
             * @param[in] origin The new origin
             */
            void setOrigin(const std::string& origin);
            /**
             * @brief Returns the flight's origin
             * @return The origin as ICAO
             */
            const std::string& origin() const;
            /**
             * @brief Sets the destination of the flight
             * @param[in] destination The new destination
             */
            void setDestination(const std::string& destination);
            /**
             * @brief Returns the flight's destiniation
             * @return The destination as ICAO
             */
            const std::string& destination() const;
            /**
             * @brief Sets the cruise flight level
             * @param[in] flightLevel The new cruise flight level
             */
            void setFlightLevel(const types::Length& flightLevel);
            /**
             * @brief Returns the cruise flight level
             * @return The cruise flight level
             */
            const types::Length& flightLevel() const;
            /**
             * @brief Sets the departure route (SID) of the flight
             * @param[in] route The new SID
             */
            void setDepartureRoute(const std::string& route);
            /**
             * @brief Returns the flight's departure route (SID)
             * @return The departure route
             */
            const std::string& departureRoute() const;
            /**
             * @brief Sets the arrival route (STAR/TRANS) of the flight
             * @param[in] route The new STAR/TRANS
             */
            void setArrivalRoute(const std::string& route);
            /**
             * @brief Returns the flight's arrival route (STAR/TRANS)
             * @return The arrival route
             */
            const std::string& arrivalRoute() const;
            /**
             * @brief Sets the current clearance limit
             * @param[in] altitude The new clearance limit
             */
            void setClearanceLimit(const types::Length& altitude);
            /**
             * @brief Returns the clearance limit
             * @return The clearance limit
             */
            const types::Length& clearanceLimit() const;
            /**
             * @brief Sets the clearance flag of the flight plan
             * @param[in] flag True if the flight is cleared according to the flight plan, else false
             */
            void setClearanceFlag(bool flag);
            /**
             * @brief Returns the clearance flag of the flight plan
             * @return True if the flight is cleared according to the flight plan, else false
             */
            bool clearanceFlag() const;
            /**
             * @brief Sets the assigned squawk for the flight
             * @param[in] squawk The new assigned squawk
             */
            void setAssignedSquawk(std::uint16_t squawk);
            /**
             * @brief Returns the assigned squawk
             * @return The assigned squawk or zero
             */
            std::uint16_t assignedSquawk() const;
            /**
             * @brief Sets if a flight is RNAV-capable
             * @param[in] capable True if the flight is RNAV-capable, else false
             */
            void setRnavCapable(bool capable);
            /**
             * @brief Returns if the flight is RNAV-capable
             * @return True if the flight is RNAV-capable, else false
             */
            bool rnavCapable() const;
            /**
             * @brief Sets if a flight has a transponder
             * @param[in] exists True if the flight has a transponder, else false
             */
            void setTransponderExistence(bool exists);
            /**
             * @brief Returns if the flight has a transponder
             * @return True if the flight has a transponder, else false
             */
            bool transponderExists() const;
            /**
             * @brief Sets the defined route of the flight
             * @param[in] route The new route
             */
            void setRoute(Route&& route);
            /**
             * @brief Returns the route of the flight
             * @return The route
             */
            const Route& route() const;
            /**
             * @brief Sets the new departure runway
             * @param[in] runway The departure runway
             */
            void setDepartureRunway(const std::string& runway);
            /**
             * @brief Returns the departure runway
             * @return The departure runway
             */
            const std::string& departureRunway() const;
            /**
             * @brief Sets the new arrival runway
             * @param[in] runway The arrival runway
             */
            void setArrivalRunway(const std::string& runway);
            /**
             * @brief Returns the arrival runway
             * @return The arrival runway
             */
            const std::string& arrivalRunway() const;
            /**
             * @brief Set the ATC command flag
             * @param[in] command The new command flag, but no combination between arrival and departure is allowed
             */
            void setFlag(AtcCommand command);
            /**
             * @brief Returns the departure-relevant ATC command
             * @return The departure relevant ATC command
             */
            AtcCommand departureFlag() const;
            /**
             * @brief Returns the arrival-relevant ATC command
             * @return The arrival relevant ATC command
             */
            AtcCommand arrivalFlag() const;
        };
    }
}
