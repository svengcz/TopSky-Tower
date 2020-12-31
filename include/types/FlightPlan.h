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
            enum class Type {
                Unknown = 0,
                VFR     = 1,
                IFR     = 2
            };

        private:
            Type          m_type;
            std::string   m_origin;
            std::string   m_departureRoute;
            std::string   m_destination;
            std::string   m_arrivalRoute;
            types::Length m_clearanceLimit;
            bool          m_cleared;

        public:
            /**
             * @brief Creates a flight plan
             */
            FlightPlan();

            void setType(Type type);
            Type type() const;
            void setOrigin(const std::string& origin);
            const std::string& origin() const;
            void setDestination(const std::string& destination);
            const std::string& destination() const;
            void setDepartureRoute(const std::string& route);
            const std::string& departureRoute() const;
            void setArrivalRoute(const std::string& route);
            const std::string& arrivalRoute() const;
            void setClearanceLimit(const types::Length& altitude);
            const types::Length& clearanceLimit() const;
            void setClearedFlag(bool flag);
            bool clearedFlag() const;
        };
    }
}
