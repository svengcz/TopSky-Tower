/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the sector control system
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include <algorithm>

#include <management/SectorControl.h>
#include <system/FlightRegistry.h>

using namespace topskytower;
using namespace topskytower::management;
using namespace topskytower::types;

SectorControl::SectorControl() :
        m_unicom(new Node(types::Sector("UNICOM", "", "", "FSS", "122.800"))),
        m_rootNode(nullptr),
        m_ownSector(nullptr),
        m_handoffs(),
        m_sectorsOfFlights(),
        m_handoffOfFlightsToMe() { }

SectorControl::SectorControl(const std::string& airport, const std::list<types::Sector>& sectors) :
        m_unicom(new Node(types::Sector("UNICOM", "", "", "FSS", "122.800"))),
        m_rootNode(nullptr),
        m_ownSector(nullptr),
        m_handoffs(),
        m_sectorsOfFlights() {
    std::list<types::Sector> airportSectors;

    /* find the tower sectors of the airport */
    for (const auto& sector : std::as_const(sectors)) {
        if (airport == sector.controllerInfo().prefix()) {
            switch (sector.type()) {
            case types::Sector::Type::Tower:
            case types::Sector::Type::Ground:
            case types::Sector::Type::Delivery:
                airportSectors.push_back(sector);
                break;
            default:
                break;
            }
        }
    }

    /* find all deputies of the airport that are not other airport sectors */
    std::list<std::string> deputies;
    for (const auto& tower : std::as_const(airportSectors)) {
        for (const auto& border : std::as_const(tower.borders())) {
            for (const auto& deputy : std::as_const(border.deputies())) {
                /* check if the deputy is an other airport sector */
                bool isAirportSector = false;
                for (const auto& other : std::as_const(airportSectors)) {
                    if (other.controllerInfo().identifier() == deputy) {
                        isAirportSector = true;
                        break;
                    }
                }

                /* check if the deputy is known and that it is not an other airport sector */
                if (false == isAirportSector && deputies.cend() == std::find(deputies.cbegin(), deputies.cend(), deputy))
                    deputies.push_back(deputy);
            }
        }
    }

    /* get all relevant sectors and sort them in a top-down order */
    auto sortedSectors(sectors);
    sortedSectors.sort([](const types::Sector& sector0, const types::Sector& sector1) {
        if (sector0.type() != sector1.type())
            return sector0.type() > sector1.type();
        else if (0 == sector0.borders().size() || 0 == sector1.borders().size())
            return sector0.borders().size() > sector1.borders().size();
        else
            return sector0.borders().back().upperAltitude() > sector1.borders().back().upperAltitude();
    });
    auto nodes = SectorControl::findRelevantSectors(deputies, sortedSectors);
    for (const auto& sector : std::as_const(airportSectors))
        SectorControl::insertNode(nodes, sector);

    /* sort the sectors from upper to lower airspaces */
    nodes.sort([](const std::shared_ptr<SectorControl::Node>& node0, const std::shared_ptr<SectorControl::Node>& node1) {
        /* ensure that all types are equal */
        if (node0->sector.type() != node1->sector.type())
            return node0->sector.type() > node1->sector.type();
        /* ensure that the siblings are sorted together */
        else if (node0->sector.controllerInfo().prefix() != node1->sector.controllerInfo().prefix())
            return node0->sector.controllerInfo().prefix() > node1->sector.controllerInfo().prefix();
        /* ensure that the sector without borders is lower */
        else if (0 == node0->sector.borders().size() || node1->sector.borders().size())
            return node0->sector.borders().size() > node1->sector.borders().size();
        /* ensure that the upper levels are above */
        else
            return node0->sector.borders().back().upperAltitude() > node1->sector.borders().back().upperAltitude();
    });

    /* create all relevant siblings */
    auto siblings = SectorControl::linkSiblings(nodes);

    /* create the final graph */
    SectorControl::createGraph(siblings);

    /* add the centers of the top-level to get the complete hierarchy */
    for (const auto& deputy : std::as_const(siblings.front().front()->sector.borders().front().deputies())) {
        for (const auto& sector : std::as_const(sectors)) {
            /* found the correct deputy */
            if (sector.controllerInfo().identifier() == deputy) {
                std::shared_ptr<Node> node(new Node(sector));

                if (nullptr == this->m_rootNode) {
                    node->children = siblings.front();

                    for (auto& sibling : siblings.front())
                        sibling->parents.push_back(node);
                }
                else {
                    this->m_rootNode->parents.push_back(node);
                    node->children.push_back(this->m_rootNode);
                }

                this->m_rootNode = node;

                break;
            }
        }
    }

    this->m_unicom->controllers.push_back(types::ControllerInfo());
}

SectorControl::~SectorControl() {
    SectorControl::destroyNode(this->m_rootNode);
    this->m_rootNode = nullptr;
    this->m_ownSector = nullptr;
    this->m_handoffs.clear();
}

void SectorControl::destroyNode(std::shared_ptr<Node>& node) {
    if (nullptr == node)
        return;

    for (auto& child : node->children)
        SectorControl::destroyNode(child);

    node->children.clear();
}

void SectorControl::insertNode(std::list<std::shared_ptr<Node>>& nodes, const types::Sector& sector) {
    /* check if the sector is already a node */
    bool alreadyRegistered = false;
    for (const auto& node : std::as_const(nodes)) {
        if (node->sector.controllerInfo().identifier() == sector.controllerInfo().identifier()) {
            alreadyRegistered = true;
            break;
        }
    }

    /* create the new node */
    if (false == alreadyRegistered) {
        std::shared_ptr<SectorControl::Node> node(new SectorControl::Node(sector));
        nodes.push_back(node);
    }
}

std::list<std::shared_ptr<SectorControl::Node>> SectorControl::findRelevantSectors(std::list<std::string>& deputies,
                                                                                   const std::list<types::Sector>& sectors) {
    std::list<std::shared_ptr<SectorControl::Node>> retval;

    /* filter out the centers */
    for (auto it = deputies.begin(); deputies.end() != it;) {
        bool erased = false, found = false;

        for (const auto& sector : std::as_const(sectors)) {
            if (*it == sector.controllerInfo().identifier()) {
                found = true;
                if (types::Sector::Type::Center == sector.type() || types::Sector::Type::FlightService == sector.type()) {
                    it = deputies.erase(it);
                    erased = true;
                }
                break;
            }
        }

        if (false == found)
            it = deputies.erase(it);
        else if (false == erased)
            ++it;
    }

    /* create the nodes for all deputies */
    std::string approachPrefix;
    for (const auto& deputy : std::as_const(deputies)) {
        for (const auto& sector : std::as_const(sectors)) {
            if (sector.controllerInfo().identifier() == deputy) {
                /* assume that all approaches have the same prefix */
                if (types::Sector::Type::Approach == sector.type() && 0 == approachPrefix.length())
                    approachPrefix = sector.controllerInfo().prefix();

                if (types::Sector::Type::Approach == sector.type()) {
                    if (approachPrefix == sector.controllerInfo().prefix())
                        SectorControl::insertNode(retval, sector);
                }
                else {
                    SectorControl::insertNode(retval, sector);
                }
            }
            else if (0 != sector.borders().size()) {
                auto it = std::find(sector.borders().front().deputies().cbegin(), sector.borders().front().deputies().cend(), deputy);
                if (sector.borders().front().deputies().cend() != it) {
                    if (types::Sector::Type::Approach == sector.type()) {
                        if (approachPrefix == sector.controllerInfo().prefix())
                            SectorControl::insertNode(retval, sector);
                    }
                    else {
                        SectorControl::insertNode(retval, sector);
                    }
                }
            }
        }
    }

    return retval;
}

std::list<std::list<std::shared_ptr<SectorControl::Node>>> SectorControl::linkSiblings(std::list<std::shared_ptr<Node>>& nodes) {
    std::list<std::list<std::shared_ptr<SectorControl::Node>>> retval;

    for (auto it = nodes.begin(); nodes.end() != it; ++it) {
        if (0 != retval.size()) {
            /* both sectors have the same type -> further analysis is needed */
            if (retval.back().front()->sector.type() == (*it)->sector.type()) {
                switch (retval.back().front()->sector.type()) {
                case types::Sector::Type::Approach:
                {
                    /*
                     * The idea is to differentiate the feeder from the pick up.
                     * It is only possible by the analyzis of the highest controlled altitute.
                     *
                     * If the altitudes are comparable (max. 10% difference), they are the same.
                     * In the end we get two or more approaches where the lowest approach is the feeder.
                     */
                    auto ratio = (retval.back().front()->sector.borders().back().upperAltitude() - (*it)->sector.borders().back().upperAltitude()) / (*it)->sector.borders().back().upperAltitude();
                    if (-0.1 <= ratio.value() && 0.1 >= ratio.value())
                        retval.back().push_back(*it);
                    else
                        retval.push_back({ *it });
                    break;
                }
                case types::Sector::Type::Departure:
                case types::Sector::Type::Tower:
                case types::Sector::Type::Ground:
                    /*
                     * Assume that the link to the airport allows only one departure airspace
                     * that can be splitted into multiple sectors.
                     *
                     * Assume that all siblings have the same prefix.
                     */
                    if (retval.back().front()->sector.controllerInfo().prefix() == (*it)->sector.controllerInfo().prefix())
                        retval.back().push_back({ *it });
                    else
                        retval.push_back({ *it });
                    break;
                case types::Sector::Type::Delivery:
                    retval.push_back({ *it });
                    break;
                default:
                    break;
                }
            }
            else if (types::Sector::Type::Departure == (*it)->sector.type()) {
                retval.back().push_back(*it);
            }
            else {
                retval.push_back({ *it });
            }
        }
        else {
            retval.push_back({ *it });
        }
    }

    return retval;
}

void SectorControl::createGraph(std::list<std::list<std::shared_ptr<Node>>>& siblings) {
    std::list<std::shared_ptr<SectorControl::Node>> parents;

    for (auto it = siblings.cbegin(); siblings.cend() != it; ++it) {
        if (types::Sector::Type::Ground == it->front()->sector.type()) {
            for (const auto& level : std::as_const(siblings)) {
                if (types::Sector::Type::Tower == level.front()->sector.type() &&
                    it->front()->sector.controllerInfo().prefix() == level.front()->sector.controllerInfo().prefix())
                {
                    for (auto& node : *it)
                        node->parents = level;
                    for (auto& node : level)
                        node->children = *it;
                }
            }
        }
        else if (types::Sector::Type::Delivery == it->front()->sector.type()) {
            for (const auto& level : std::as_const(siblings)) {
                if (types::Sector::Type::Ground == level.front()->sector.type() &&
                    it->front()->sector.controllerInfo().prefix() == level.front()->sector.controllerInfo().prefix())
                {
                    for (auto& node : *it)
                        node->parents = level;
                    for (auto& node : level)
                        node->children = *it;
                }
            }
        }
        else {
            for (auto& node : *it)
                node->parents = parents;
            for (auto& node : parents)
                node->children.insert(node->children.end(), it->begin(), it->end());

            if (types::Sector::Type::Tower < it->front()->sector.type())
                parents = *it;
        }
    }
}

std::shared_ptr<SectorControl::Node> SectorControl::findNodeBasedOnIdentifier(const std::shared_ptr<SectorControl::Node>& node,
                                                                              const std::string_view& identifier) {
    if (nullptr == node)
        return nullptr;

    /* this node is the correct one */
    if (node->sector.controllerInfo().identifier() == identifier)
        return node;

    /* check every child recursivly */
    for (const auto& child : std::as_const(node->children)) {
        auto retval = SectorControl::findNodeBasedOnIdentifier(child, identifier);
        if (nullptr != retval)
            return retval;
    }

    return nullptr;
}

std::shared_ptr<SectorControl::Node> SectorControl::findNodeBasedOnInformation(const std::shared_ptr<SectorControl::Node>& node,
                                                                               const types::ControllerInfo& info) {
    if (nullptr == node)
        return nullptr;

    /* test the unique information */
    if (node->sector.controllerInfo().identifier() == info.identifier())
        return node;
    if (node->sector.controllerInfo().primaryFrequency() == info.primaryFrequency())
        return node;

    /* check the SectorControl's name which is likely to be unique */
    for (const auto& controller : std::as_const(node->controllers)) {
        if (controller.controllerName() == info.controllerName()) {
            /* ensure that the stations are compatible and not something different (i.e. SectorControl has TWR and ATIS) */
            if (controller.prefix() == info.prefix() && controller.suffix() == info.suffix())
                return node;
        }
    }

    for (const auto& child : std::as_const(node->children)) {
        auto candidate = SectorControl::findNodeBasedOnInformation(child, info);
        if (nullptr != candidate)
            return candidate;
    }

    return nullptr;
}

void SectorControl::cleanupHandoffList(std::shared_ptr<SectorControl::Node>& node) {
    if (0 == node->controllers.size()) {
        for (auto it = this->m_handoffs.begin(); this->m_handoffs.end() != it;) {
            if (it->second.nextSector == node)
                it = this->m_handoffs.erase(it);
            else
                ++it;
        }
    }
}

void SectorControl::controllerUpdate(const types::ControllerInfo& info) {
    auto node = SectorControl::findNodeBasedOnInformation(this->m_rootNode, info);
    if (nullptr != node) {
        /* check if the info is already registered */
        for (auto it = node->controllers.begin(); node->controllers.end() != it; ++it) {
            /* found the controller -> check if the controller changed the identifier */
            if (it->controllerName() == info.controllerName() && it->suffix() == info.suffix()) {
                if (node->sector.controllerInfo().identifier() != info.identifier()) {
                    node->controllers.erase(it);
                    this->cleanupHandoffList(node);
                }
                else {
                    *it = info;
                }
                return;
            }
        }

        node->controllers.push_back(info);
    }
}

void SectorControl::controllerOffline(const types::ControllerInfo& info) {
    auto node = SectorControl::findNodeBasedOnInformation(this->m_rootNode, info);

    if (nullptr != node) {
        /* remove the controller info */
        for (auto it = node->controllers.begin(); node->controllers.end() != it; ++it) {
            if (it->controllerName() == info.controllerName()) {
                node->controllers.erase(it);
                this->cleanupHandoffList(node);
                return;
            }
        }
    }
}

void SectorControl::setOwnSector(const types::ControllerInfo& info) {
    /* mark the same sector as the own sector */
    if (nullptr != this->m_ownSector && info.identifier() == this->m_ownSector->sector.controllerInfo().identifier())
        return;

    /* remove the old controller out of the list */
    if (nullptr != this->m_ownSector) {
        for (auto it = this->m_ownSector->controllers.begin(); this->m_ownSector->controllers.end() != it; ++it) {
            if (it->controllerName() == info.controllerName()) {
                this->m_ownSector->controllers.erase(it);
                break;
            }
        }
    }

    this->m_ownSector = SectorControl::findNodeBasedOnIdentifier(this->m_rootNode, info.identifier());
    this->m_ownSector->controllers.push_back(info);
}

std::shared_ptr<SectorControl::Node> SectorControl::findSectorInList(const std::list<std::shared_ptr<SectorControl::Node>>& nodes,
                                                                     const types::Position& position,
                                                                     types::Flight::Type type, bool lowerSectors) {
    types::Sector::Type bestType = types::Sector::Type::Approach;

    if (types::Flight::Type::Unknown == type) {
        for (const auto& node : std::as_const(nodes)) {
            if (true == node->sector.isInsideSector(position))
                return node;
        }
    }
    else if (types::Flight::Type::Departure == type && false == lowerSectors) {
        bestType = types::Sector::Type::Departure;
    }

    /* find the next responsible sector and priorize the correct sector type */
    std::shared_ptr<SectorControl::Node> retval = nullptr;
    for (const auto& node : std::as_const(nodes)) {
        if (true == node->sector.isInsideSector(position)) {
            if (nullptr == retval || node->sector.type() == bestType)
                retval = node;
        }
    }

    return retval;
}

const types::ControllerInfo& SectorControl::ownSector() const {
    if (nullptr == this->m_ownSector)
        return this->m_unicom->sector.controllerInfo();
    else
        return this->m_ownSector->sector.controllerInfo();
}

std::shared_ptr<SectorControl::Node> SectorControl::findOnlineResponsible(const types::Flight& flight,
                                                                          const types::Position& position,
                                                                          bool ignoreClearanceFlag) const {
    std::list<std::shared_ptr<Node>> candidates;

    for (const auto& parent : std::as_const(this->m_ownSector->parents)) {
        auto candidate = SectorControl::findLowestSector(parent, flight, position, ignoreClearanceFlag);
        if (nullptr != candidate) {
            candidates.push_back(candidate);

            /* found a sector and a departure is irrelevant */
            if (types::Sector::Type::Approach != candidate->sector.type())
                break;
        }
    }

    if (0 == candidates.size())
        return nullptr;

    std::shared_ptr<Node> node = candidates.front();

    /* check if a departure or an approach is more helpful */
    if (1 < candidates.size()) {
        for (const auto& candidate : std::as_const(candidates)) {
            if (candidate->sector.type() == types::Sector::Type::Departure && flight.type() == types::Flight::Type::Departure) {
                node = candidate;
                break;
            }
            else if (candidate->sector.type() == types::Sector::Type::Approach && flight.type() == types::Flight::Type::Arrival) {
                node = candidate;
                break;
            }
        }
    }

    /* avoid wrong calls */
    if (nullptr == node)
        return this->m_unicom;

    /* this is the next online station */
    if (0 != node->controllers.size())
        return node;

    /* check which deputy is online */
    for (const auto& deputy : std::as_const(node->sector.borders().front().deputies())) {
        auto deputyNode = SectorControl::findNodeBasedOnIdentifier(this->m_rootNode, deputy);
        if (nullptr != deputyNode && 0 != deputyNode->controllers.size())
            return deputyNode;

        /* some deliveries do not have the complete deputy-hierarchy -> check the deputies of the deputies */
        if (types::Sector::Type::Delivery == node->sector.type()) {
            for (const auto& depDeputy : std::as_const(deputyNode->sector.borders().front().deputies())) {
                auto secondStageNode = SectorControl::findNodeBasedOnIdentifier(this->m_rootNode, depDeputy);
                if (nullptr != secondStageNode && 0 != secondStageNode->controllers.size())
                    return secondStageNode;
            }
        }
    }

    /* no other real station is online */
    return this->m_unicom;
}

std::shared_ptr<SectorControl::Node> SectorControl::findLowestSector(const std::shared_ptr<SectorControl::Node>& node,
                                                                     const types::Flight& flight, const types::Position& position,
                                                                     bool ignoreClearanceFlag) {
    /* check the children */
    for (const auto& child : std::as_const(node->children)) {
        auto retval = SectorControl::findLowestSector(child, flight, position, ignoreClearanceFlag);
        if (nullptr != retval) {
            /* non-departures do not go to the delivery */
            if (types::Flight::Type::Departure != flight.type() && types::Sector::Type::Delivery == retval->sector.type())
                continue;
            /* Delivery is only allowed if the clearance flag is not set */
            else if (types::Sector::Type::Delivery != retval->sector.type() || false == flight.flightPlan().clearanceFlag())
                return retval;
            /* ingore any contextual information */
            else if (true == ignoreClearanceFlag)
                return retval;
        }
    }

    /* check the node itself */
    if (true == node->sector.isInsideSector(position))
        return node;

    return nullptr;
}

bool SectorControl::isInOwnSectors(const types::Flight& flight, const types::Position& position, bool ignoreClearanceFlag) const {
    auto node = this->findOnlineResponsible(flight, position, ignoreClearanceFlag);
    if (nullptr == node)
        return false;

    return this->m_ownSector == node && nullptr != this->m_ownSector;
}

void SectorControl::updateFlight(const types::Flight& flight) {
    if (nullptr == this->m_rootNode || nullptr == this->m_ownSector)
        return;

    /* check if the handoff is initiated or manually changed */
    bool manuallyChanged = false, handoffDone = false;
    auto it = this->m_handoffs.find(flight.callsign());
    if (this->m_handoffs.end() != it && (true == it->second.manuallyChanged || true == it->second.handoffPerformed)) {
        manuallyChanged = it->second.manuallyChanged;
        handoffDone = it->second.handoffPerformed;
    }

    /* get current sector of the flight */
    this->m_sectorsOfFlights[flight.callsign()] = this->findLowestSector(this->m_rootNode, flight, flight.currentPosition(), false);
    if (nullptr == this->m_sectorsOfFlights[flight.callsign()])
        this->m_sectorsOfFlights.erase(flight.callsign());

    bool ignoreClearanceFlag = types::Sector::Type::Delivery == this->m_ownSector->sector.type();
    bool insideOwnBorder = this->isInOwnSectors(flight, flight.currentPosition(), ignoreClearanceFlag);
    if (false == manuallyChanged && false == handoffDone && (true == insideOwnBorder || true == flight.isTracked()))
    {
        types::Position predicted;

        /* use the exactly same position, but change the evaluation of the clearance flag */
        if (types::Sector::Type::Delivery == this->m_ownSector->sector.type())
            predicted = flight.currentPosition();
        /* the flight is moving on the ground -> predict only 10 seconds */
        else if (40_kn > flight.groundSpeed())
            predicted = flight.predict(10_s, 20_kn);
        /* the flight is assumed to be in the air */
        else
            predicted = flight.predict(20_s, flight.groundSpeed());

        /* get the handoff initiator to avoid rehandoffs if we received an early handoff */
        auto handoffIt = this->m_handoffOfFlightsToMe.find(flight.callsign());
        if (false == insideOwnBorder) {
            if (this->m_handoffOfFlightsToMe.end() == handoffIt) {
                this->m_handoffOfFlightsToMe[flight.callsign()] = flight.handoffInitiatedId();
                handoffIt = this->m_handoffOfFlightsToMe.find(flight.callsign());
            }
        }
        /* delete the handoff tracker if the flight is in our sector */
        else if (this->m_handoffOfFlightsToMe.end() != handoffIt) {
            this->m_handoffOfFlightsToMe.erase(handoffIt);
            handoffIt = this->m_handoffOfFlightsToMe.end();
        }

        /* the aircraft remains in own sector */
        if (true == this->isInOwnSectors(flight, predicted, false)) {
            /* check if an old handoff exists */
            it = this->m_handoffs.find(flight.callsign());
            if (this->m_handoffs.end() != it)
                this->m_handoffs.erase(it);
            return;
        }

        /* get the next responsible node and the next online station */
        auto nextNode = this->findOnlineResponsible(flight, predicted, false);

        /* found a possible handoff candidate */
        if (nullptr != nextNode && nextNode != this->m_ownSector) {
            /* give the flight to an other sector */
            if (this->m_handoffOfFlightsToMe.end() == handoffIt || handoffIt->second != nextNode->sector.controllerInfo().identifier()) {
                this->m_handoffs[flight.callsign()] = { false, false, flight, nextNode };
            }
            /* delete the old entry, because of a new controller */
            else if (this->m_handoffOfFlightsToMe.end() != handoffIt && handoffIt->second == nextNode->sector.controllerInfo().identifier()) {
                auto hIt = this->m_handoffs.find(flight.callsign());
                if (this->m_handoffs.end() != hIt)
                    this->m_handoffs.erase(hIt);
            }
        }
    }
    /* check if we have to remove the handoff information */
    else if (this->m_handoffs.end() != it && true == it->second.handoffPerformed) {
        /* find the current online controller but ignore the clearance flag to avoid too early deletions, if Delivery is online */
        auto currentNode = this->findOnlineResponsible(flight, flight.currentPosition(), ignoreClearanceFlag);

        /* check if an other controller is responsible */
        if (currentNode != this->m_ownSector && false == flight.isTracked())
            this->m_handoffs.erase(it);
    }
}

void SectorControl::removeFlight(const std::string& callsign) {
    auto it = this->m_handoffs.find(callsign);
    if (this->m_handoffs.end() != it)
        this->m_handoffs.erase(it);

    auto sit = this->m_sectorsOfFlights.find(callsign);
    if (this->m_sectorsOfFlights.end() != sit)
        this->m_sectorsOfFlights.erase(sit);

    auto hit = this->m_handoffOfFlightsToMe.find(callsign);
    if (this->m_handoffOfFlightsToMe.end() != hit)
        this->m_handoffOfFlightsToMe.erase(hit);
}

bool SectorControl::isInOwnSector(const types::Flight& flight) {
    auto it = this->m_sectorsOfFlights.find(flight.callsign());

    if (this->m_sectorsOfFlights.cend() != it) {
        if (it->second == this->m_ownSector)
            return true;

        return this->isInOwnSectors(flight, flight.currentPosition(), false);
    }

    return false;
}

bool SectorControl::handoffRequired(const types::Flight& flight) const {
    auto it = this->m_handoffs.find(flight.callsign());
    if (this->m_handoffs.cend() != it)
        return false == it->second.handoffPerformed;
    return false;
}

bool SectorControl::handoffRequired(const std::string& callsign) const {
    auto it = this->m_handoffs.find(callsign);
    if (this->m_handoffs.cend() != it)
        return false == it->second.handoffPerformed;
    return false;
}

bool SectorControl::handoffPossible(const types::Flight& flight) const {
    if (nullptr == this->m_rootNode || nullptr == this->m_ownSector)
        return false;

    bool inOwnSector = this->isInOwnSectors(flight, flight.currentPosition(), false);
    return true == inOwnSector || true == flight.isTracked();
}

void SectorControl::handoffPerformed(const types::Flight& flight) {
    auto it = this->m_handoffs.find(flight.callsign());
    if (this->m_handoffs.end() != it)
        it->second.handoffPerformed = true;
}

const types::ControllerInfo& SectorControl::handoffSector(const types::Flight& flight) const {
    auto it = this->m_handoffs.find(flight.callsign());
    if (this->m_handoffs.cend() == it)
        return this->m_unicom->sector.controllerInfo();

    return it->second.nextSector->sector.controllerInfo();
}

const types::ControllerInfo& SectorControl::handoffSector(const std::string& callsign) const {
    auto it = this->m_handoffs.find(callsign);
    if (this->m_handoffs.cend() == it)
        return this->m_unicom->sector.controllerInfo();

    return it->second.nextSector->sector.controllerInfo();
}

std::list<std::string> SectorControl::handoffStations(const types::Flight& flight) const {
    auto it = this->m_handoffs.find(flight.callsign());
    std::list<std::string> retval;
    if (this->m_handoffs.cend() == it)
        return retval;

    for (const auto& controller : std::as_const(it->second.nextSector->controllers)) {
        if (0 != controller.prefix().size()) {
            std::string controllerCallsign(controller.prefix() + "_");
            if (0 != controller.midfix().length())
                controllerCallsign += controller.midfix() + "_";
            controllerCallsign += controller.suffix();

            retval.push_back(std::move(controllerCallsign));
        }
        else {
            retval.push_back("");
        }
    }

    return retval;
}

std::list<std::shared_ptr<SectorControl::Node>> SectorControl::findSectorCandidates(const std::shared_ptr<SectorControl::Node>& node) const {
    std::list<std::shared_ptr<SectorControl::Node>> retval;

    if (this->m_ownSector != node && 0 != node->controllers.size())
        retval.push_back(node);

    for (const auto& child : std::as_const(node->children))
        retval.splice(retval.end(), this->findSectorCandidates(child));

    return retval;
}

std::list<types::ControllerInfo> SectorControl::handoffSectors() const {
    std::list<types::ControllerInfo> retval;

    auto nodes = this->findSectorCandidates(this->m_rootNode);

    /* erase duplicates */
    nodes.sort([](const std::shared_ptr<SectorControl::Node>& node0, const std::shared_ptr<SectorControl::Node>& node1) {
        return node0 < node1;
    });
    auto last = std::unique(nodes.begin(), nodes.end());
    nodes.erase(last, nodes.end());

    /* sort based on type */
    nodes.sort([](const std::shared_ptr<SectorControl::Node>& node0, const std::shared_ptr<SectorControl::Node>& node1) {
        return node0->sector.type() > node1->sector.type();
    });

    for (const auto& node : std::as_const(nodes))
        retval.push_back(node->sector.controllerInfo());

    return retval;
}

void SectorControl::handoffSectorSelect(const types::Flight& flight, const std::string& identifier) {
    auto it = this->m_handoffs.find(flight.callsign());
    if (this->m_handoffs.end() == it) {
        this->m_handoffs[flight.callsign()] = { false, false, flight, nullptr };
        it = this->m_handoffs.find(flight.callsign());
    }

    auto node = SectorControl::findNodeBasedOnIdentifier(this->m_rootNode, identifier);
    if (nullptr != node) {
        it->second.manuallyChanged = true;
        it->second.handoffPerformed = false;
        it->second.nextSector = node;
    }
}

bool SectorControl::sectorHandoverPossible() const {
    if (nullptr != this->m_rootNode && nullptr != this->m_ownSector)
        return 1 < this->m_ownSector->controllers.size();
    else
        return false;
}

std::list<types::ControllerInfo> SectorControl::sectorHandoverCandidates() const {
    std::list<types::ControllerInfo> retval;

    if (nullptr != this->m_rootNode && nullptr != this->m_ownSector) {
        for (const auto& controller : std::as_const(this->m_ownSector->controllers))
            retval.push_back(controller);
    }

    return retval;
}

bool SectorControl::isInSector(const types::Flight& flight) const {
    auto it = this->m_sectorsOfFlights.find(flight.callsign());
    if (this->m_sectorsOfFlights.cend() != it && nullptr != it->second)
        return true;
    else
        return false;
}

std::list<types::ControllerInfo> SectorControl::findOnlineControllers(const std::shared_ptr<Node>& node) const {
    std::list<types::ControllerInfo> retval;

    for (const auto& child : std::as_const(node->children)) {
        auto children = this->findOnlineControllers(child);

        for (const auto& elem : std::as_const(children)) {
            auto it = std::find(retval.cbegin(), retval.cend(), elem);
            if (retval.cend() == it)
                retval.push_back(elem);
        }
    }

    for (const auto& controller : std::as_const(node->controllers)) {
        auto it = std::find(retval.cbegin(), retval.cend(), controller);
        if (retval.cend() == it)
            retval.push_back(controller);
    }

    return retval;
}
