/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the Autonomous Runway Incursion Warning System
 */

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <GeographicLib/Gnomonic.hpp>
#include <GeographicLib/LocalCartesian.hpp>

#include <surveillance/STCDControl.h>

using namespace topskytower;
using namespace topskytower::surveillance;
using namespace topskytower::types;

STCDControl::STCDControl(const std::string& airport, const types::Coordinate& center, const std::list<types::Runway>& runways) :
        m_airportIcao(airport),
        m_reference(center),
        m_runways(runways),
        m_noTransgressionZones() {
    system::ConfigurationRegistry::instance().registerNotificationCallback(this, &STCDControl::reinitialize);

    this->reinitialize(system::ConfigurationRegistry::UpdateType::All);
}

STCDControl::~STCDControl() {
    system::ConfigurationRegistry::instance().deleteNotificationCallback(this);
}

void STCDControl::createNTZ(const std::pair<std::string, std::string>& runwayPair) {
    types::Runway rwy[2];

    /* find the corresponding runways */
    for (const auto& runway : std::as_const(this->m_runways)) {
        if (runway.name() == runwayPair.first)
            rwy[0] = runway;
        else if (runway.name() == runwayPair.second)
            rwy[1] = runway;
    }

    /* validate that both runways are found */
    if (0 == rwy[0].name().length() || 0 == rwy[1].name().length())
        return;

    /* calculate the heading between both runways */
    auto ntzHeading = (rwy[0].end().bearingTo(rwy[0].start()) + rwy[1].end().bearingTo(rwy[1].start())) * 0.5f;

    GeographicLib::LocalCartesian projection(this->m_reference.latitude().convert(types::degree),
                                             this->m_reference.longitude().convert(types::degree));

    /* calculate the center between both runways */
    Eigen::Vector3f gnonomicThresholds[2];
    projection.Forward(rwy[0].start().latitude().convert(types::degree), rwy[0].start().longitude().convert(types::degree), 0.0f,
                       gnonomicThresholds[0][0], gnonomicThresholds[0][1], gnonomicThresholds[0][2]);
    projection.Forward(rwy[1].start().latitude().convert(types::degree), rwy[1].start().longitude().convert(types::degree), 0.0f,
                       gnonomicThresholds[1][0], gnonomicThresholds[1][1], gnonomicThresholds[1][2]);
    Eigen::Vector3f gnonomicRunwayCenter = gnonomicThresholds[0] + 0.5f * (gnonomicThresholds[1] - gnonomicThresholds[0]);

    /* find a point on the center line in the direction of the NTZ to define the center line */
    float coordinates[3];
    projection.Reverse(gnonomicRunwayCenter[0], gnonomicRunwayCenter[1], gnonomicRunwayCenter[2],
                       coordinates[0], coordinates[1], coordinates[2]);
    types::Coordinate ntzStart(coordinates[1] * types::degree, coordinates[0] * types::degree);

    types::Coordinate ntzEnd = ntzStart.projection(ntzHeading, 10_nm);
    Eigen::Vector3f gnonomicNtzPosition;
    projection.Forward(ntzEnd.latitude().convert(types::degree), ntzEnd.longitude().convert(types::degree), 0.0f,
                       gnonomicNtzPosition[0], gnonomicNtzPosition[1], gnonomicNtzPosition[2]);

    /* define the NTZ-center line to project the points of the runway */
    Eigen::ParametrizedLine<float, 3> ntzLine(gnonomicRunwayCenter, (gnonomicNtzPosition - gnonomicRunwayCenter).normalized());

    /* find the correct starting point of the NTZ (beginning of the "earlier" runway threshold) */
    auto projectionRwy0 = ntzLine.projection(gnonomicThresholds[0]);
    auto projectionRwy1 = ntzLine.projection(gnonomicThresholds[1]);
    auto projectionDirection = (projectionRwy1 - projectionRwy0).normalized();
    if (0.1f >= (ntzLine.direction() - projectionDirection).norm()) {
        projection.Reverse(projectionRwy1[0], projectionRwy1[1], projectionRwy1[2],
                           coordinates[0], coordinates[1], coordinates[2]);
    }
    else {
        projection.Reverse(projectionRwy0[0], projectionRwy0[1], projectionRwy0[2],
                           coordinates[0], coordinates[1], coordinates[2]);
    }
    ntzStart = types::Coordinate(coordinates[1] * types::degree, coordinates[0] * types::degree);

    /* calculate the NTZ's end point */
    ntzEnd = ntzStart.projection(ntzHeading, 10_nm);

    /* calculate the edges in a fixed order (the required polygon-order will be fixed in SectorBorder, if needed) */
    std::list<types::Coordinate> edges;
    edges.push_back(ntzEnd.projection(rwy[0].heading() + 90.0_deg, 1000_ft));
    edges.push_back(ntzStart.projection(rwy[1].heading() + 90.0_deg, 1000_ft));
    edges.push_back(ntzStart.projection(rwy[1].heading() - 90.0_deg, 1000_ft));
    edges.push_back(ntzEnd.projection(rwy[0].heading() - 90.0_deg, 1000_ft));

    /* create the border */
    types::SectorBorder ntz("", {}, 0_ft, 99000_ft);
    ntz.setEdges(edges);
    this->m_noTransgressionZones.push_back(std::move(ntz));
}

void STCDControl::reinitialize(system::ConfigurationRegistry::UpdateType type) {
    if (system::ConfigurationRegistry::UpdateType::All != type && system::ConfigurationRegistry::UpdateType::Runtime != type)
        return;

    const auto& configuration = system::ConfigurationRegistry::instance().runtimeConfiguration();
    this->m_noTransgressionZones.clear();

    /* no IPA active -> no NTZ-definition needed */
    if (false == configuration.ipaActive)
        return;

    std::list<std::pair<std::string, std::string>> ipaPairs, prmPairs;

    const auto& airportConfig = system::ConfigurationRegistry::instance().airportConfiguration(this->m_airportIcao);

    /* get the IPA and PRM pairs that are possible based on the static and runtime configuration */
    const auto& arrivalRunways = configuration.activeArrivalRunways.find(this->m_airportIcao)->second;
    for (auto it = arrivalRunways.cbegin(); arrivalRunways.cend() != it; ++it) {
        auto cit = it;
        std::advance(cit, 1);

        /* check if IPA is available for this runway */
        if (airportConfig.ipaRunways.cend() != airportConfig.ipaRunways.find(*it)) {
            const auto& partnerRwys = airportConfig.ipaRunways.find(*it)->second;

            for (; arrivalRunways.cend() != cit; ++cit) {
                auto partner = std::find(partnerRwys.cbegin(), partnerRwys.cend(), *cit);
                if (partnerRwys.cend() != partner)
                    ipaPairs.push_back(std::make_pair(*it, *cit));
            }
        }

        /* check if PRM is available for this runway */
        if (airportConfig.prmRunways.cend() != airportConfig.prmRunways.find(*it)) {
            const auto& partnerRwys = airportConfig.prmRunways.find(*it)->second;

            for (; arrivalRunways.cend() != cit; ++cit) {
                auto partner = std::find(partnerRwys.cbegin(), partnerRwys.cend(), *cit);
                if (partnerRwys.cend() != partner)
                    prmPairs.push_back(std::make_pair(*it, *cit));
            }
        }
    }

    /* create all required NTZs */
    for (const auto& pair : std::as_const(ipaPairs))
        this->createNTZ(pair);
    for (const auto& pair : std::as_const(prmPairs))
        this->createNTZ(pair);
}

void STCDControl::updateFlight(const types::Flight& flight) {

}

void STCDControl::removeFlight(const std::string& callsign) {

}

const std::list<types::SectorBorder>& STCDControl::noTransgressionZones() const {
    return this->m_noTransgressionZones;
}
