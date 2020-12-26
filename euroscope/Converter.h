/*
 * @brief Defines the EuroScope to TopSky-Tower converter
 * @file euroscope/Converter.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#ifndef __AFXWIN_H__
#error "include 'stdafx.h' before including this file for PCH"
#endif

#include <string>

#pragma warning(push, 0)
#include <EuroScopePlugIn.h>
#pragma warning(pop)

#include <types/Flight.h>

namespace topskytower {
    namespace euroscope {
        /**
         * @brief Defines the different ES converters
         * @ingroup euroscope
         */
        class Converter {
        public:
            static types::Coordinate convert(const EuroScopePlugIn::CPosition& position);
            static types::Flight convert(const EuroScopePlugIn::CRadarTarget& target, const std::string& airport);
        };
    }
}
