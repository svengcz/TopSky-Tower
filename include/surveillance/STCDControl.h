/*
 * @brief Defines a STCD system
 * @file surveillance/STCDControl.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <map>

#include <system/ConfigurationRegistry.h>
#include <types/Flight.h>
#include <types/SectorBorder.h>

namespace topskytower {
    namespace surveillance {
        /**
         * @brief Describes a Short Term Conflict Detection
         * @ingroup surveillance
         */
        class STCDControl {
        private:
#ifndef DOXYGEN_IGNORE
            types::SectorBorder m_noTransgressionZone;

            void reinitialize(system::ConfigurationRegistry::UpdateType type);

        public:
            /**
             * @brief Creates a STCD control instance
             */
            STCDControl();
            /**
             * @brief Destroys all internal structures and registrations
             */
            ~STCDControl();
#endif
        };
    }
}
