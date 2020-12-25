/*
 * @brief Defines and implements functions to get mathematical helper functions
 * @file helper/Math.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <cmath>

namespace topskytower {
    namespace helper {
        /**
         * @brief Contains helper functions to get numerical stability in the functions
         * @ingroup helper
         */
        class Math {
        public:
            Math(const Math& other) = delete;
            Math(Math&& other) = delete;
            Math& operator=(const Math& other) = delete;
            Math& operator=(Math&& other) = delete;

            /**
             * @brief Compares two floating point numbers if they are almost equal
             * @param[in] value0 The first floating point number
             * @param[in] value1 The second floating point number
             * @param[in] threshold The threshold which defines both numbers as almost equal
             * @return True if the difference between value0 and value1 is smaller than the threshold, else false
             */
            static __inline bool almostEqual(float value0, float value1, float threshold = 1e-4) {
                return std::abs(value0 - value1) <= threshold;
            }
        };
    }
}
