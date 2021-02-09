/*
 * @brief Defines the separation values between different aircrafts
 * @file system/Separations.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <map>

#include <types/Aircraft.h>

namespace topskytower {
    namespace system {
        class Separation {
        public:
            static std::map<std::pair<types::Aircraft::WTC, types::Aircraft::WTC>, types::Length> EuclideanDistance;
            static std::map<std::pair<types::Aircraft::WTC, types::Aircraft::WTC>, types::Time> TimeDistance;
        };
    }
}
