/*
 * @brief Defines a NOTAM control system
 * @file management/NotamControl.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <functional>
#include <thread>
#include <list>
#include <map>
#include <mutex>

#include <types/SystemConfiguration.h>

namespace topskytower {
    namespace management {
        /**
         * @brief The NOTAM control handles all available and relevant NOTAMs of the airports
         * @ingroup management
         */
        class NotamControl {
        public:
#ifndef DOXYGEN_IGNORE
        private:
            NotamControl();

            void run();

        public:
            /**
             * @brief Destroys all internal structures
             */
            ~NotamControl();

            NotamControl(const NotamControl& other) = delete;
            NotamControl(NotamControl&& other) = delete;

            NotamControl& operator=(const NotamControl& other) = delete;
            NotamControl& operator=(NotamControl&& other) = delete;

            /**
             * @brief Returns the NOTAM control singleton
             * @return The NOTAM control system
             */
            static NotamControl& instance();
#endif
        };
    }
}
