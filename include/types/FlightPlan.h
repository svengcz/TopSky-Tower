/*
 * @brief Defines a flight plan structure
 * @file types/FlightPlan.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <string>

#include <types/Quantity.hpp>

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

        private:
            Type          m_type;
            std::string   m_origin;
            std::string   m_departureRoute;
            std::string   m_destination;
            std::string   m_arrivalRoute;
            std::uint16_t m_assignedSquawk;
            types::Length m_clearanceLimit;
            bool          m_cleared;

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
            void setClearedFlag(bool flag);
            /**
             * @brief Returns the clearance flag of the flight plan
             * @return True if the flight is cleared according to the flight plan, else false
             */
            bool clearedFlag() const;
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
        };
    }
}
