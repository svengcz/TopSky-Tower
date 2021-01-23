/*
 * @brief Defines the version checker
 * @file euroscope/VersionChecker.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <string>

#include "RadarScreen.h"

namespace topskytower {
    namespace euroscope {
        /**
         * @brief Defines the version checker to validates the plugin's version
         * @ingroup euroscope
         */
        class VersionChecker {
        private:
            static std::uint32_t versionHash(std::uint8_t major, std::uint8_t minor, std::uint8_t patch);

        public:
            /**
             * @brief Checks if an update is available and shows it on the given RADAR screen
             * @param[in] screen The screen that will show the messages
             */
            static void checkForUpdates(RadarScreen* screen);
        };
    }
}
