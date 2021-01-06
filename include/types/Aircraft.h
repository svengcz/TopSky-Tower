/*
 * @brief Defines an aircraft structure
 * @file types/Aircraft.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <string>

#include <types/Quantity.hpp>

namespace topskytower {
    namespace types {
        /**
         * @brief Describes an aircraft with all information
         * @ingroup types
         */
        class Aircraft {
        public:
            /**
             * @brief Defines the different wake turbulence categories
             */
            enum class WTC {
                Unknown = 0, /**< Unknown category */
                Light   = 1, /**< Light aircraft category (i.e. Cessna C172) */
                Medium  = 2, /**< Medium aircraft category (i.e. Airbus A320) */
                Heavy   = 3, /**< Heavy aircraft category (i.e. Boeing B747) */
                Super   = 4  /**< Super aircraft category (i.e. Airbus A380) */
            };
            /**
             * @brief Defines the engine type
             */
            enum class EngineType {
                Unknown   = 0, /**< Unknown engine type */
                Turboprop = 1, /**< Turboprobs in use */
                Jet       = 2, /**< Jets in use */
                Electric  = 3  /**< Electric engines in use */
            };

        private:
            std::string   m_code;
            EngineType    m_engineType;
            std::uint8_t  m_engineCount;
            WTC           m_wtc;
            types::Length m_length;
            types::Length m_wingspan;
            types::Length m_height;
            types::Mass   m_maxTakeoffWeight;

        public:
            /**
             * @brief Creates an uninitialized aircraft
             */
            Aircraft();

            /**
             * @brief Sets the ICAO code
             * @param[in] code The ICAO code
             */
            void setIcaoCode(const std::string& code);
            /**
             * @brief Returns the ICAO code
             * @return The ICAO code
             */
            const std::string& icaoCode() const;
            /**
             * @brief Sets the engine type
             * @param[in] type The engine type
             */
            void setEngineType(EngineType type);
            /**
             * @brief Returns the engine type
             * @return The engine type
             */
            EngineType engineType() const;
            /**
             * @brief Sets the engine count
             * @param[in] count The engine count
             */
            void setEngineCount(std::uint8_t count);
            /**
             * @brief Returns the engine count
             * @return The engine count
             */
            std::uint8_t engineCount() const;
            /**
             * @brief Sets the WTC
             * @param[in] wtc The wake turbulence category
             */
            void setWTC(WTC wtc);
            /**
             * @brief Returns the aircraft's WTC
             * @return The wake turbulence category
             */
            WTC wtc() const;
            /**
             * @brief Sets the length
             * @param[in] length The aircraft's length
             */
            void setLength(const types::Length& length);
            /**
             * @brief Returns the aircraft's length
             * @return The aircraft's length
             */
            const types::Length& length() const;
            /**
             * @brief Sets the wingpsan
             * @param[in] wingspan The aircraft's wingspan
             */
            void setWingspan(const types::Length& wingspan);
            /**
             * @brief Returns the aircraft's wingspan
             * @return The aircraft's wingspan
             */
            const types::Length& wingspan() const;
            /**
             * @brief Sets the height
             * @param[in] height The aircraft's height
             */
            void setHeight(const types::Length& height);
            /**
             * @brief Returns the aircraft's height
             * @return The aircraft's height
             */
            const types::Length& height() const;
            /**
             * @brief Sets the MTOW
             * @param[in] mtow The maximum take-off weight
             */
            void setMTOW(const types::Mass& mtow);
            /**
             * @brief Returns the aircraft's MTOW
             * @return The aircraft's maximum take-off weight
             */
            const types::Mass& mtow() const;
        };
    }
}
