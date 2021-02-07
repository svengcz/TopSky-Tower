/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the stand control system
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include <algorithm>

#include <GeographicLib/Gnomonic.hpp>

#include <management/StandControl.h>
#include <system/ConfigurationRegistry.h>
#include <types/Quantity.hpp>

using namespace topskytower;
using namespace topskytower::management;
using namespace topskytower::types;

StandControl::StandControl(const std::string& airport, const types::Coordinate& center) :
        m_airportIcao(airport),
        m_standTree(),
        m_standTreeAdaptor(nullptr),
        m_aircraftStandRelation(),
        m_centerPosition(center),
        m_gatPosition() {
    system::ConfigurationRegistry::instance().registerNotificationCallback(this, &StandControl::reinitialize);

    this->reinitialize(system::ConfigurationRegistry::UpdateType::All);
}

StandControl::~StandControl() {
    system::ConfigurationRegistry::instance().deleteNotificationCallback(this);

    if (nullptr != this->m_standTreeAdaptor)
        delete this->m_standTreeAdaptor;
    this->m_standTreeAdaptor = nullptr;

    this->m_standTree.stands.clear();
    this->m_aircraftStandRelation.clear();
}

void StandControl::copyStandData(const types::Stand& config, StandData& block) {
    block.name = config.name;
    block.position = config.position;
    block.assignmentRadius = config.assignmentRadius;
    block.priority = config.priority;
    block.blockingStands = config.blockingStands;
    block.manualAssignment = config.manualAssignment;
    for (int i = 0; i < 2; ++i) {
        block.wingspan[i] = config.wingspan[i];
        block.length[i] = config.length[i];
        block.height[i] = config.height[i];
    }
    block.wtcWhitelist = config.wtcWhitelist;
    block.wtcBlacklist = config.wtcBlacklist;
    block.engineTypeWhitelist = config.engineTypeWhitelist;
    block.engineTypeBlacklist = config.engineTypeBlacklist;
}

void StandControl::reinitialize(system::ConfigurationRegistry::UpdateType type) {
    if (system::ConfigurationRegistry::UpdateType::All != type && system::ConfigurationRegistry::UpdateType::Airports != type)
        return;

    /* delete all old information */
    if (nullptr != this->m_standTreeAdaptor)
        delete this->m_standTreeAdaptor;
    this->m_standTreeAdaptor = nullptr;
    this->m_standTree.stands.clear();
    this->m_aircraftStandRelation.clear();
    this->m_standPriorities.clear();

    const auto& config = system::ConfigurationRegistry::instance().airportConfiguration(this->m_airportIcao);
    if (false == config.valid || 0 == config.aircraftStands.size())
        return;

    /* copy the stand data into the new structure and convert the coordinate to Cartesian coordinates */
    GeographicLib::Gnomonic projection(GeographicLib::Geodesic::WGS84());
    for (const auto& stand : std::as_const(config.aircraftStands)) {
        /* the GAT is a special stand */
        if ("GAT" == stand.name) {
            this->m_gatPosition = stand;
            continue;
        }

        StandData data;
        StandControl::copyStandData(stand, data);

        projection.Forward(this->m_centerPosition.latitude().convert(types::degree),
                           this->m_centerPosition.longitude().convert(types::degree),
                           data.position.latitude().convert(types::degree),
                           data.position.longitude().convert(types::degree),
                           data.cartesianPosition[0],
                           data.cartesianPosition[1]);

        this->m_standTree.stands[data.name] = data;
    }

#pragma warning(disable: 4127)
    this->m_standTreeAdaptor = new StandTreeAdaptor(2, this->m_standTree, nanoflann::KDTreeSingleIndexAdaptorParams(10));
#pragma warning(default: 4127)
    this->m_standTreeAdaptor->buildIndex();

    /* copy the airlines */
    for (const auto& airline : std::as_const(config.airlines))
        this->m_standPriorities[airline.airlineIcao] = airline;
}

void StandControl::markStandAsOccupied(std::map<std::string, StandData>::iterator& iter, const types::Flight& flight, types::Flight::Type type) {
    /* mark the direct stand as occupied */
    iter->second.occupancyFlights.push_back(std::make_pair(flight, type));
    this->m_aircraftStandRelation[flight.callsign()] = iter->first;

    /* mark the blocking stands as occupied */
    for (const auto& neighbor : std::as_const(iter->second.blockingStands)) {
        auto neighborIt = this->m_standTree.stands.find(neighbor);
        if (this->m_standTree.stands.end() != neighborIt)
            neighborIt->second.occupancyFlights.push_back(std::make_pair(flight, type));
    }
}

static __inline bool __outOfRange(const types::Length borders[2], const types::Length& dimension) {
    if (borders[0] > dimension || borders[1] < dimension)
        return true;
    else
        return false;
}

std::list<std::string> StandControl::findAvailableAndUsableStands(const types::Flight& flight, bool ignoreManualFlag) const {
    std::list<std::string> retval;

    /* test all stands */
    for (const auto& stand : std::as_const(this->m_standTree.stands)) {
        /* ingore the stand for the automatic assignment */
        if (false == ignoreManualFlag && true == stand.second.manualAssignment)
            continue;

        /* stand is occupied */
        if (0 != stand.second.occupancyFlights.size())
            continue;

        /* validate the length dimension */
        if (true == __outOfRange(stand.second.length, flight.flightPlan().aircraft().length()))
            continue;
        /* validate the wingspan dimension */
        if (true == __outOfRange(stand.second.wingspan, flight.flightPlan().aircraft().wingspan()))
            continue;
        /* validate the height dimension */
        if (true == __outOfRange(stand.second.height, flight.flightPlan().aircraft().height()))
            continue;

        /* check the WTC blacklist */
        auto wtcIt = std::find(stand.second.wtcBlacklist.cbegin(), stand.second.wtcBlacklist.cend(),
                               flight.flightPlan().aircraft().wtc());
        if (stand.second.wtcBlacklist.cend() != wtcIt)
            continue;

        /* check the engine type blacklist */
        auto engineIt = std::find(stand.second.engineTypeBlacklist.cbegin(), stand.second.engineTypeBlacklist.cend(),
                                  flight.flightPlan().aircraft().engineType());
        if (stand.second.engineTypeBlacklist.cend() != engineIt)
            continue;

        /* found a theoretically canidate */
        retval.push_back(stand.first);
    }

    return retval;
}

bool StandControl::findOptimalStand(const types::Flight& flight, types::Flight::Type type, const std::list<std::string>& availableStands) {
    types::Length minDistance = 1000.0_nm;
    types::Aircraft::WTC bestWtc = types::Aircraft::WTC::Super;
    std::string bestStand;

    for (const auto& stand : std::as_const(availableStands)) {
        auto it = this->m_standTree.stands.find(stand);
        auto distance = it->second.position.distanceTo(flight.currentPosition().coordinate());

        /* most important is the optimal WTC category */
        if (0 != it->second.wtcWhitelist.size()) {
            if (bestWtc < it->second.wtcWhitelist.front()) {
                minDistance = distance;
                bestWtc = it->second.wtcWhitelist.front();
                bestStand = stand;
                continue;
            }
        }

        /* second priority is the distance */
        if (minDistance > distance) {
            minDistance = distance;
            bestStand = stand;
        }
    }

    /* found a stand -> assign it */
    if (0 != bestStand.length()) {
        auto it = this->m_standTree.stands.find(bestStand);
        this->markStandAsOccupied(it, flight, type);
        return true;
    }

    return false;
}

bool StandControl::assignStand(const types::Flight& flight, types::Flight::Type type, const std::list<types::StandPriorities>& priorities,
                               const std::list<std::string>& availableStands) {
    std::list<std::string> finalCandidates;

    /* test all priority levels */
    for (const auto& priority : std::as_const(priorities)) {
        /* check if the stand is in the available list */
        for (const auto& stand : std::as_const(priority.stands)) {
            auto it = std::find(availableStands.cbegin(), availableStands.cend(), stand);
            /* found a candidate on the level */
            if (availableStands.cend() != it)
                finalCandidates.push_back(*it);
        }

        /* find the best stand */
        if (0 != finalCandidates.size()) {
            if (true == this->findOptimalStand(flight, type, finalCandidates))
                return true;
        }
    }

    return false;
}

void StandControl::updateFlight(const types::Flight& flight, types::Flight::Type type) {
    if (nullptr == this->m_standTreeAdaptor)
        return;

    const auto& maxDist = system::ConfigurationRegistry::instance().systemConfiguration().standAssociationDistance;
    if (maxDist < flight.currentPosition().coordinate().distanceTo(this->m_centerPosition))
        return;

    /* unknown and departures have the priority */
    if (types::Flight::Type::Arrival != type) {
        /* check if we have to delete the old assignment due to push-back or taxi */
        auto relIt = this->m_aircraftStandRelation.find(flight.callsign());
        if (this->m_aircraftStandRelation.end() != relIt) {
            const auto& stand = this->m_standTree.stands[relIt->second];
            auto distance = stand.position.distanceTo(flight.currentPosition().coordinate());

            /* flight is too far away */
            if (distance > stand.assignmentRadius)
                this->removeFlight(flight.callsign());

            return;
        }

        GeographicLib::Gnomonic projection(GeographicLib::Geodesic::WGS84());
        float queryPt[2];

        /* calculate the Cartesian coordinate */
        projection.Forward(this->m_centerPosition.latitude().convert(types::degree),
                           this->m_centerPosition.longitude().convert(types::degree),
                           flight.currentPosition().coordinate().latitude().convert(types::degree),
                           flight.currentPosition().coordinate().longitude().convert(types::degree),
                           queryPt[0], queryPt[1]);

        /* search the nearest stand */
        size_t index;
        float sqDistance;
        if (1 == this->m_standTreeAdaptor->knnSearch(queryPt, 1, &index, &sqDistance)) {
            auto it = this->m_standTree.stands.begin();
            std::advance(it, index);

            /* check that the distance matches */
            if (it->second.assignmentRadius < it->second.position.distanceTo(flight.currentPosition().coordinate()))
                return;

            /* remove arrival flights out of the stand */
            std::list<std::string> arrivalCallsigns;
            for (auto oit = it->second.occupancyFlights.cbegin(); it->second.occupancyFlights.cend() != oit; ++oit) {
                if (types::Flight::Type::Arrival == oit->second)
                    arrivalCallsigns.push_back(oit->first.callsign());
            }
            for (const auto& cs : std::as_const(arrivalCallsigns))
                this->removeFlight(cs);

            /* reserve the stand */
            this->markStandAsOccupied(it, flight, type);
        }
    }
    /* handle only IFR arrival flights */
    else if (types::FlightPlan::Type::IFR == flight.flightPlan().type() && types::Flight::Type::Arrival == type) {
        /* check if we need to assign a new stand */
        auto it = this->m_aircraftStandRelation.find(flight.callsign());
        if (this->m_aircraftStandRelation.cend() != it)
            return;

        auto availableStands = this->findAvailableAndUsableStands(flight, false);

        std::string airlineIcao;
        if (3 < flight.callsign().length())
            airlineIcao = flight.callsign().substr(0, 3);

        /* check if an airline-definition exists */
        auto airlineIt = this->m_standPriorities.find(airlineIcao);
        if (this->m_standPriorities.cend() != airlineIt) {
            /* try to assign a stand based on the priorities */
            if (true == this->assignStand(flight, type, airlineIt->second.standPriorities, availableStands))
                return;
        }

        /* assing based on list in a generic way */
        this->findOptimalStand(flight, type, availableStands);
    }
    else if (types::FlightPlan::Type::VFR == flight.flightPlan().type() && types::Flight::Type::Arrival == type) {
        if (0 != this->m_gatPosition.name.length())
            this->m_aircraftStandRelation[flight.callsign()] = this->m_gatPosition.name;
    }
}

void StandControl::removeFlight(const std::string& callsign) {
    /* check if the flight is registered at a stand */
    auto it = this->m_aircraftStandRelation.find(callsign);
    if (this->m_aircraftStandRelation.end() != it) {
        /* find the coorect stand */
        auto standIt = this->m_standTree.stands.find(it->second);
        if (this->m_standTree.stands.end() != standIt) {
            /* mark the stand as free */
            for (auto csIt = standIt->second.occupancyFlights.begin(); csIt != standIt->second.occupancyFlights.end(); ++csIt) {
                if (csIt->first.callsign() == callsign) {
                    standIt->second.occupancyFlights.erase(csIt);
                    break;
                }
            }

            /* mark all blocked neighbors free as well */
            for (const auto& neighbor : std::as_const(standIt->second.blockingStands)) {
                auto neighborIt = this->m_standTree.stands.find(neighbor);
                if (this->m_standTree.stands.end() != neighborIt) {
                    for (auto csIt = neighborIt->second.occupancyFlights.begin(); csIt != neighborIt->second.occupancyFlights.end(); ++csIt) {
                        if (csIt->first.callsign() == callsign) {
                            neighborIt->second.occupancyFlights.erase(csIt);
                            break;
                        }
                    }
                }
            }
        }

        /* erase the relation */
        this->m_aircraftStandRelation.erase(it);
    }
}

void StandControl::assignManually(const types::Flight& flight, types::Flight::Type type, const std::string& stand) {
    this->removeFlight(flight.callsign());

    if (0 != this->m_gatPosition.name.length() && this->m_gatPosition.name == stand) {
        this->m_aircraftStandRelation[flight.callsign()] = stand;
    }
    else {
        auto it = this->m_standTree.stands.find(stand);
        if (this->m_standTree.stands.end() != it)
            this->markStandAsOccupied(it, flight, type);
    }
}

bool StandControl::standExists(const std::string& name) const {
    if (0 != this->m_gatPosition.name.length() && this->m_gatPosition.name == name) {
        return true;
    }
    else {
        auto it = this->m_standTree.stands.find(name);
        return this->m_standTree.stands.end() != it;
    }
}

std::string StandControl::stand(const types::Flight& flight) const {
    auto it = this->m_aircraftStandRelation.find(flight.callsign());
    if (this->m_aircraftStandRelation.cend() != it)
        return it->second;
    else
        return "";
}

const types::Stand& StandControl::stand(const std::string& name) const {
    static types::Stand __fallback;

    if ("GAT" == name && 0 != this->m_gatPosition.name.length()) {
        return this->m_gatPosition;
    }
    else {
        for (const auto& stand : std::as_const(this->m_standTree.stands)) {
            if (name == stand.second.name)
                return stand.second;
        }
    }

    return __fallback;
}

std::list<std::string> StandControl::allPossibleAndAvailableStands(const types::Flight& flight) const {
    auto retval = this->findAvailableAndUsableStands(flight, true);
    if (0 != this->m_gatPosition.name.length())
        retval.push_front(this->m_gatPosition.name);
    return retval;
}

std::list<std::pair<std::string, bool>> StandControl::allStands() const {
    std::list<std::pair<std::string, bool>> retval;

    if (0 != this->m_gatPosition.name.length())
        retval.push_back(std::make_pair(this->m_gatPosition.name, false));

    for (const auto& stand : std::as_const(this->m_standTree.stands))
        retval.push_back(std::make_pair(stand.first, 0 != stand.second.occupancyFlights.size()));

    return std::move(retval);
}

bool StandControl::standIsBlocked(const std::string& stand) const {
    auto it = this->m_standTree.stands.find(stand);
    if (this->m_standTree.stands.cend() != it)
        return 1 < it->second.occupancyFlights.size();
    else
        return false;
}
