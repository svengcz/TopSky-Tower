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
            static void checkForUpdates(RadarScreen* screen);
        };
    }
}
