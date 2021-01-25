/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the IPA toolbar button
 */

#include "stdafx.h"

#include <system/ConfigurationRegistry.h>

#include "../RadarScreen.h"
#include "IpaToolbarButton.h"

using namespace topskytower;
using namespace topskytower::euroscope;

IpaToolbarButton::IpaToolbarButton(RadarScreen* parent) :
        ToolbarButton(parent, "IPA", Gdiplus::RectF(0.0f, 0.0f, 40.0f, 10.0f)) { }

void IpaToolbarButton::clicked() {
    bool isActive = this->active();

    auto configuration = system::ConfigurationRegistry::instance().runtimeConfiguration();
    const auto& airportConfig = system::ConfigurationRegistry::instance().airportConfiguration(this->m_parent->airportIcao());
    configuration.ipaActive = false;

    /* no active arrival runways found configuration found */
    if (configuration.activeArrivalRunways.cend() == configuration.activeArrivalRunways.find(this->m_parent->airportIcao()))
        return;

    const auto& arrivalRunways = configuration.activeArrivalRunways.find(this->m_parent->airportIcao())->second;
    bool ipaAvailable = false, prmAvailable = false;
    for (auto it = arrivalRunways.cbegin(); arrivalRunways.cend() != it; ++it) {
        auto cit = it;
        std::advance(cit, 1);

        if (airportConfig.ipaRunways.cend() != airportConfig.ipaRunways.find(*it)) {
            const auto& partnerRwys = airportConfig.ipaRunways.find(*it)->second;

            for (; arrivalRunways.cend() != cit; ++cit) {
                auto partner = std::find(partnerRwys.cbegin(), partnerRwys.cend(), *cit);
                if (partnerRwys.cend() != partner) {
                    ipaAvailable = true;
                    break;
                }
            }
        }
        else if (airportConfig.prmRunways.cend() != airportConfig.prmRunways.find(*it)) {
            const auto& partnerRwys = airportConfig.prmRunways.find(*it)->second;

            for (; arrivalRunways.cend() != cit; ++cit) {
                auto partner = std::find(partnerRwys.cbegin(), partnerRwys.cend(), *cit);
                if (partnerRwys.cend() != partner) {
                    prmAvailable = true;
                    break;
                }
            }
        }

        if (true == ipaAvailable || true == prmAvailable) {
            configuration.ipaActive = !isActive;
            break;
        }
    }

    system::ConfigurationRegistry::instance().setRuntimeConfiguration(configuration);
}

bool IpaToolbarButton::active() {
    return system::ConfigurationRegistry::instance().runtimeConfiguration().ipaActive;
}

