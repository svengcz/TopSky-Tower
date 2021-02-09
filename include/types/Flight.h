/*
 * @brief Defines a flight structure
 * @file types/Flight.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <types/FlightPlan.h>
#include <types/Position.h>

namespace topskytower {
    namespace types {
        /**
         * @brief Describes a flight with all kinematic information
         * @ingroup types
         */
        class Flight {
        public:
            /**
             * @brief Defines if the flight is a departing, arriving or unknown
             */
            enum class Type {
                Unknown   = 0, /**< Unable to determine if it is departing or arriving */
                Departure = 1, /**< It is a departing flight */
                Arrival   = 2  /**< It is an arriving flight */
            };

        private:
            FlightPlan      m_flightPlan;
            std::string     m_callsign;
            bool            m_airborne;
            Position        m_currentPosition;
            Velocity        m_groundSpeed;
            Velocity        m_verticalSpeed;
            bool            m_markedByController;
            bool            m_onMissedApproach;
            bool            m_irregularFlight;
            bool            m_establishedOnILS;
            bool            m_departureReady;
            bool            m_isTrackedByController;
            bool            m_isTrackedByOtherController;
            std::string     m_handoffReceivedBy;

        public:
            /**
             * @brief Creates a flight with zero values
             */
            Flight();
            /**
             * @brief Creates a flight with a defined callsign
             * @param[in] callsign The flight's callsign
             */
            explicit Flight(const std::string& callsign);

            /**
             * @brief Returns the flight's callsign
             * @return The callsign
             */
            const std::string& callsign() const;
            /**
             * @brief Returns if the flight was airborne in the past or not
             * @return True if the flight was airborne, else false
             */
            bool airborne() const;
            /**
             * @brief Returns the current position
             * @return The current position
             */
            const Position& currentPosition() const;
            /**
             * @brief Returns the ground speed
             * @return The ground speed
             */
            const Velocity& groundSpeed() const;
            /**
             * @brief Returns the vertical speed
             * @return The vertical speed
             */
            const Velocity& verticalSpeed() const;
            /**
             * @brief Returns if the flight is marked by a controller
             * @return True if it is marked, else false
             */
            bool markedByController() const;
            /**
             * @brief Returns if the flight is on a missed approach procedure
             * @return True if it is on a missed approach procedure, else false
             */
            bool onMissedApproach() const;
            /**
             * @brief Returns if the flight is a non-standard handoff
             * @return True if it is a non-standard handoff, else false
             */
            bool irregularHandoff() const;
            /**
             * @brief Returns if the flight is ready for departure
             * @return True if it is ready, else false
             */
            bool readyForDeparture() const;
            /**
             * @brief Returns if the flight is established on the ILS
             * @return True if it is established on the ILS
             */
            bool establishedOnILS() const;
            /**
             * @brief Returns the flight plan
             * @return The flight plan
             */
            const FlightPlan& flightPlan() const;
            /**
             * @brief Returns a changable flight plan
             * @return The changable flight plan
             */
            FlightPlan& flightPlan();
            /**
             * @brief Sets the airborne state
             * @param[in] airborne The airborne state
             */
            void setAirborne(bool airborne);
            /**
             * @brief Sets the current position
             * @param[in] position The current position
             */
            void setCurrentPosition(const Position& position);
            /**
             * @brief Sets the ground speed
             * @param[in] groundSpeed The ground speed
             */
            void setGroundSpeed(const Velocity& groundSpeed);
            /**
             * @brief Sets the vertical speed
             * @param[in] verticalSpeed The vertical speed
             */
            void setVerticalSpeed(const Velocity& verticalSpeed);
            /**
             * @brief Sets if the flight is marked by a controller or not
             * @param[in] value True if it is marked, else false
             */
            void setMarkedByController(bool value);
            /**
             * @brief Sets if the flight is on a missed approach procedure
             * @param[in] value True if it is on a missed approach procedure, else false
             */
            void setOnMissedApproach(bool value);
            /**
             * @brief Sets if the flight is a non-standard handoff
             * @param[in] value True if it is a non-standard handoff, else false
             */
            void setIrregularHandoff(bool value);
            /**
             * @brief Sets if the flight is established on the ILS
             * @param[in] value True if it is established on the ILS, else false
             */
            void setEstablishedOnILS(bool value);
            /**
             * @brief Sets if the flight is ready for departure or not
             * @param[in] value True if the flight is ready, else false
             */
            void setReadyForDeparture(bool value);
            /**
             * @brief Sets the flight plan
             * @param[in] plan The flight plan
             */
            void setFlightPlan(const FlightPlan& plan);
            /**
             * @brief Returns if the aircraft is tracked
             * @return True if the aircraft is tracked, else false
             */
            bool isTracked() const;
            /**
             * @brief Sets the tracking state
             * @param[in] state The new tracking state
             */
            void setTrackedState(bool state);
            /**
             * @brief Returns if the flight is tracked by an other controller
             * @return True if the flight is tracked by an other controller, else false
             */
            bool isTrackedByOther() const;
            /**
             * @brief Sets the tracking state for other controllers
             * @param[in] state The new tracking by other state
             */
            void setTrackedByOtherState(bool state);
            /**
             * @brief Sets the ID of the controller who initiated a handoff to this instance
             * @param[in] id The controllers ID
             */
            void setHandoffInitiatedId(const std::string& id);
            /**
             * @brief Returns the ID of the controller who initiated a handoff to this instance
             * @return The controller's ID
             */
            const std::string& handoffInitiatedId() const;
            /**
             * @brief Predicts the next position based on the kinematics and a constant-velocity model
             * @param[in] duration The time step of the prediction
             * @param[in] minGroundSpeed The minimum assumed ground speed if the actual one is smaller
             * @return The predicted position
             */
            Position predict(const Time& duration, const Velocity& minGroundSpeed) const;
        };
    }
}
