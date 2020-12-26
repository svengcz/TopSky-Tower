/*
 * @brief Defines a controller information
 * @file types/Position.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <types/Position.h>

namespace topskytower {
    namespace types {
        /**
         * @brief Describes a controller information
         * @ingroup types
         */
        class ControllerInfo {
        private:
            std::string m_identifier;
            std::string m_prefix;
            std::string m_midfix;
            std::string m_suffix;
            std::string m_primaryFrequency;
            std::string m_controllerName;

        public:
            /**
             * @brief Creates a controller information
             */
            ControllerInfo();
            /**
             * @brief Creates a controller information
             * @param[in] identifier The controller's identifier
             * @param[in] prefix The controller's callsign prefix
             * @param[in] midfix The controller's callsign midfix
             * @param[in] suffix The controller's callsign suffix
             * @param[in] primaryFrequency The controller's primary frequency
             * @param[in] fullName The controller's full name
             */
            ControllerInfo(const std::string& identifer, const std::string& prefix, const std::string& midfix,
                           const std::string& suffix, const std::string& primaryFrequency, const std::string& fullName);

            /**
             * @brief Returns the identifier
             * @return The identifier
             */
            const std::string& identifier() const;
            /**
             * @brief Returns the callsign's prefix
             * @return The callsign's prefix
             */
            const std::string& prefix() const;
            /**
             * @brief Returns the callsign's midfix
             * @return The callsign's midfix
             */
            const std::string& midfix() const;
            /**
             * @brief Returns the callsign's suffix
             * @return The callsign's suffix
             */
            const std::string& suffix() const;
            /**
             * @brief Returns the callsign's primary frequency
             * @return The callsign's primary frequency
             */
            const std::string& primaryFrequency() const;
            /**
             * @brief Returns the controller's full name
             * @return The controller's full name
             */
            const std::string& controllerName() const;
        };
    }
}
