/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the sector control system
 */

#include <surveillance/FlightRegistry.h>
#include <surveillance/SectorControl.h>

using namespace topskytower;
using namespace topskytower::surveillance;
using namespace topskytower::types;

SectorControl::SectorControl() :
        m_unicom(types::Sector("UNICOM", "", "", "FSS", "122.800")),
        m_rootNode(nullptr),
        m_ownSector(nullptr),
        m_handoffs(),
        m_sectorsOfFlights() { }

SectorControl::SectorControl(const std::string& airport, const std::list<types::Sector>& sectors) :
        m_unicom(types::Sector("UNICOM", "", "", "FSS", "122.800")),
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
    nodes.sort([](SectorControl::Node* node0, SectorControl::Node* node1) {
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
    SectorControl::linkSiblings(nodes);

    /* create the final graph */
    this->m_rootNode = SectorControl::createGraph(nodes);

    /* add the centers of the top-level to get the complete hierarchy */
    this->finalizeGraph(sectors);

    this->m_unicom.controllers.push_back(types::ControllerInfo());
}

SectorControl::~SectorControl() {
    SectorControl::destroyNode(this->m_rootNode);
    this->m_ownSector = nullptr;
    this->m_handoffs.clear();
}

void SectorControl::destroyNode(Node* node) {
    if (nullptr == node)
        return;

    for (auto& sibling : node->siblings)
        delete sibling;
    node->siblings.clear();

    for (auto& child : node->children)
        SectorControl::destroyNode(child);

    delete node;
}

void SectorControl::insertNode(std::list<Node*>& nodes, const types::Sector& sector) {
    /* ignore the delivery */
    if (types::Sector::Type::Delivery == sector.type())
        return;

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
        SectorControl::Node* node = new SectorControl::Node(sector);
        nodes.push_back(node);
    }
}

std::list<SectorControl::Node*> SectorControl::findRelevantSectors(std::list<std::string>& deputies,
                                                                   const std::list<types::Sector>& sectors) {
    std::list<SectorControl::Node*> retval;

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

void SectorControl::linkSiblings(std::list<SectorControl::Node*>& nodes) {
    SectorControl::Node* lastNode = nullptr;
    for (auto it = nodes.begin(); nodes.end() != it;) {
        bool erased = false;

        if (nullptr != lastNode) {
            /* both sectors have the same type -> further analysis is needed */
            if (lastNode->sector.type() == (*it)->sector.type()) {
                switch (lastNode->sector.type()) {
                case types::Sector::Type::Approach:
                {
                    /*
                     * The idea is to differentiate the feeder from the pick up.
                     * It is only possible by the analyzis of the highest controlled altitute.
                     *
                     * If the altitudes are comparable (max. 10% difference), they are the same.
                     * In the end we get two or more approaches where the lowest approach is the feeder.
                     */
                    auto ratio = (lastNode->sector.borders().back().upperAltitude() - (*it)->sector.borders().back().upperAltitude()) / (*it)->sector.borders().back().upperAltitude();
                    if (-0.1 <= ratio.value() && 0.1 >= ratio.value()) {
                        lastNode->siblings.push_back(*it);
                        it = nodes.erase(it);
                        erased = true;
                    }
                    else {
                        lastNode = *it;
                    }
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
                    if (lastNode->sector.controllerInfo().prefix() == (*it)->sector.controllerInfo().prefix()) {
                        lastNode->siblings.push_back(*it);
                        it = nodes.erase(it);
                        erased = true;
                    }
                    else {
                        lastNode = *it;
                    }
                    break;
                default:
                    break;
                }
            }
            else {
                lastNode = *it;
            }
        }
        else {
            lastNode = *it;
        }

        if (false == erased)
            ++it;
    }
}

SectorControl::Node* SectorControl::createGraph(const std::list<SectorControl::Node*>& nodes) {
    std::list<SectorControl::Node*> parents;
    SectorControl::Node* root = nullptr;

    for (const auto& node : std::as_const(nodes)) {
        if (0 != parents.size()) {
            /* define the parent-child relation for the non-ground or departure stations */
            if (types::Sector::Type::Ground != node->sector.type() && types::Sector::Type::Departure != node->sector.type()) {
                for (auto& parent : parents) {
                    parent->children.push_back(node);
                    node->parents.push_back(parent);
                }
            }

            /* found the departure -> the actual parent is the feeder -> both are the next parents */
            if (types::Sector::Type::Departure == node->sector.type()) {
                /* remove the old elements and keep only the feeder */
                if (1 != parents.size())
                    parents = { parents.back() };
                parents.front()->parents.front()->children.push_back(node);
                node->parents.push_back(parents.front()->parents.front());
                parents.push_back(node);
            }
            /* last parent is not the feeder -> overwrite it */
            else if (types::Sector::Type::Approach == node->sector.type()) {
                *parents.begin() = node;
            }
            /* ask the first parent for the children and find the correct tower */
            else if (types::Sector::Type::Ground == node->sector.type()) {
                auto& children = parents.front()->children;
                for (auto& child : children) {
                    if (types::Sector::Type::Tower == child->sector.type() &&
                        child->sector.controllerInfo().prefix() == node->sector.controllerInfo().prefix())
                    {
                        child->children.push_back(node);
                        node->parents.push_back(child);
                    }
                }
            }
        }
        else {
            parents.push_back(node);
            root = node;
        }
    }

    return root;
}

void SectorControl::finalizeGraph(const std::list<types::Sector>& sectors) {
    /* unable to find other sectors */
    if (nullptr == this->m_rootNode || 0 == this->m_rootNode->sector.borders().size())
        return;

    for (const auto& deputy : std::as_const(this->m_rootNode->sector.borders().front().deputies())) {
        for (const auto& sector : std::as_const(sectors)) {
            if (sector.controllerInfo().identifier() == deputy && types::Sector::Type::Center <= sector.type()) {
                SectorControl::Node* newNode = new SectorControl::Node(sector);

                newNode->children.push_back(this->m_rootNode);
                this->m_rootNode->parents.push_back(newNode);
                this->m_rootNode = newNode;
            }
        }
    }
}

SectorControl::Node* SectorControl::findNodeBasedOnIdentifier(SectorControl::Node* node, const std::string_view& identifier) {
    if (nullptr == node)
        return nullptr;

    /* this node is the correct one */
    if (node->sector.controllerInfo().identifier() == identifier)
        return node;

    /* check the siblings */
    for (const auto& sibling : std::as_const(node->siblings)) {
        if (sibling->sector.controllerInfo().identifier() == identifier)
            return sibling;
    }

    /* check every child recursivly */
    for (const auto& child : std::as_const(node->children)) {
        auto retval = SectorControl::findNodeBasedOnIdentifier(child, identifier);
        if (nullptr != retval)
            return retval;
    }

    return nullptr;
}

SectorControl::Node* SectorControl::findNodeBasedOnInformation(SectorControl::Node* node, const types::ControllerInfo& info) {
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

    for (const auto& sibling : std::as_const(node->siblings)) {
        auto candidate = SectorControl::findNodeBasedOnInformation(sibling, info);
        if (nullptr != candidate)
            return candidate;
    }

    for (const auto& child : std::as_const(node->children)) {
        auto candidate = SectorControl::findNodeBasedOnInformation(child, info);
        if (nullptr != candidate)
            return candidate;
    }

    return nullptr;
}

void SectorControl::cleanupHandoffList(SectorControl::Node* node) {
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

void SectorControl::setOwnSector(const std::string_view& identifier) {
    /* mark the same sector as the own sector */
    if (nullptr != this->m_ownSector && identifier == this->m_ownSector->sector.controllerInfo().identifier())
        return;

    this->m_ownSector = SectorControl::findNodeBasedOnIdentifier(this->m_rootNode, identifier);
}

SectorControl::Node* SectorControl::findSectorInList(const std::list<SectorControl::Node*>& nodes, const types::Position& position,
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
    SectorControl::Node* retval = nullptr;
    for (const auto& node : std::as_const(nodes)) {
        if (true == node->sector.isInsideSector(position)) {
            if (nullptr == retval || node->sector.type() == bestType)
                retval = node;
        }

        /* check the siblings which are not reachable by parent, child tests */
        for (const auto& sibling : std::as_const(node->siblings)) {
            if (true == sibling->sector.isInsideSector(position)) {
                if (nullptr == retval || sibling->sector.type() == bestType)
                    retval = sibling;
            }
        }
    }

    return retval;
}

const types::ControllerInfo& SectorControl::ownSector() const {
    if (nullptr == this->m_ownSector)
        return this->m_unicom.sector.controllerInfo();
    else
        return this->m_ownSector->sector.controllerInfo();
}

SectorControl::Node* SectorControl::findResponsible(const types::Position& position, types::Flight::Type type) const {
    SectorControl::Node* next = nullptr;

    /* test the children */
    next = SectorControl::findSectorInList(this->m_ownSector->children, position, type, true);

    /* test the node itself */
    if (nullptr == next && true == this->m_ownSector->sector.isInsideSector(position))
        return this->m_ownSector;

    /* test the siblings */
    if (nullptr == next)
        next = SectorControl::findSectorInList(this->m_ownSector->siblings, position, type, false);

    /* test the parents */
    if (nullptr == next)
        next = SectorControl::findSectorInList(this->m_ownSector->parents, position, type, false);

    return next;
}

SectorControl::Node* SectorControl::findOnlineResponsible(SectorControl::Node* node) {
    /* avoid wrong calls */
    if (nullptr == node)
        return &this->m_unicom;

    /* this is the next online station */
    if (0 != node->controllers.size())
        return node;

    /* check which deputy is online */
    for (const auto& deputy : std::as_const(node->sector.borders().front().deputies())) {
        auto deputyNode = SectorControl::findNodeBasedOnIdentifier(this->m_rootNode, deputy);
        if (nullptr != deputyNode && 0 != deputyNode->controllers.size())
            return deputyNode;
    }

    /* no other real station is online */
    return &this->m_unicom;
}

SectorControl::Node* SectorControl::findLowestSector(SectorControl::Node* node, const types::Position& position) {
    /* check the children */
    for (const auto& child : std::as_const(node->children)) {
        auto retval = SectorControl::findLowestSector(child, position);
        if (nullptr != retval)
            return retval;
    }

    /* check the node itself */
    if (true == node->sector.isInsideSector(position))
        return node;

    /* check the siblings */
    for (const auto& sibling : std::as_const(node->siblings)) {
        auto retval = SectorControl::findLowestSector(sibling, position);
        if (nullptr != retval)
            return retval;
    }

    return nullptr;
}

bool SectorControl::isInOwnSectors(const types::Position& position) const {
    auto node = SectorControl::findLowestSector(this->m_ownSector, position);
    if (nullptr != node) {
        /* controlled in own sector */
        if (node == this->m_ownSector)
            return true;
        /* controlled by lower sector but it can be offline */
        else
            return 0 == node->controllers.size();
    }
    else {
        return false;
    }
}

void SectorControl::update(const types::Flight& flight) {
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
    this->m_sectorsOfFlights[flight.callsign()] = this->findLowestSector(this->m_rootNode, flight.currentPosition());
    if (nullptr == this->m_sectorsOfFlights[flight.callsign()])
        this->m_sectorsOfFlights.erase(flight.callsign());

    if (false == manuallyChanged && false == handoffDone && true == this->isInOwnSectors(flight.currentPosition())) {
        auto predicted = flight.predict(20_s, 20_kn);

        /* the aircraft remains in own sector */
        if (true == this->isInOwnSectors(predicted)) {
            /* check if an old handoff exists */
            it = this->m_handoffs.find(flight.callsign());
            if (this->m_handoffs.end() != it)
                this->m_handoffs.erase(it);
            return;
        }

        /* get the next responsible node and the next online station */
        auto nextNode = this->findResponsible(predicted, flight.type());
        auto nextOnline = this->findOnlineResponsible(nextNode);

        /* no handoff to the controlled sector needed */
        if (nextOnline != this->m_ownSector)
            this->m_handoffs[flight.callsign()] = { false, false, flight, nextOnline };
    }
    /* check if we have to remove the handoff information */
    else if (this->m_handoffs.end() != it && true == it->second.handoffPerformed) {
        /* find the current online controller */
        auto currentNode = this->findResponsible(flight.currentPosition(), flight.type());
        auto currentOnline = this->findOnlineResponsible(currentNode);

        /* check if an other controller is responsible */
        if (currentOnline != this->m_ownSector)
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
}

bool SectorControl::handoffRequired(const std::string& callsign) const {
    auto it = this->m_handoffs.find(callsign);
    if (this->m_handoffs.cend() != it)
        return false == it->second.handoffPerformed;
    return false;
}

bool SectorControl::handoffPossible(const std::string& callsign) const {
    if (nullptr == this->m_rootNode || nullptr == this->m_ownSector)
        return false;

    auto& flight = surveillance::FlightRegistry::instance().flight(callsign);
    return this->isInOwnSectors(flight.currentPosition());
}

void SectorControl::handoffPerformed(const std::string& callsign) {
    auto it = this->m_handoffs.find(callsign);
    if (this->m_handoffs.end() != it)
        it->second.handoffPerformed = true;
}

const types::ControllerInfo& SectorControl::handoffSector(const std::string& callsign) const {
    auto it = this->m_handoffs.find(callsign);
    if (this->m_handoffs.cend() == it)
        return this->m_unicom.sector.controllerInfo();

    return it->second.nextSector->sector.controllerInfo();
}

std::list<std::string> SectorControl::handoffStations(const std::string& callsign) const {
    auto it = this->m_handoffs.find(callsign);
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

std::list<SectorControl::Node*> SectorControl::findSectorCandidates(SectorControl::Node* node) const {
    std::list<SectorControl::Node*> retval;

    if (this->m_ownSector != node && 0 != node->controllers.size())
        retval.push_back(node);

    for (const auto& sibling : std::as_const(node->siblings))
        retval.splice(retval.end(), this->findSectorCandidates(sibling));

    for (const auto& child : std::as_const(node->children))
        retval.splice(retval.end(), this->findSectorCandidates(child));

    return retval;
}

std::list<types::ControllerInfo> SectorControl::handoffSectors() const {
    std::list<types::ControllerInfo> retval;

    auto nodes = this->findSectorCandidates(this->m_rootNode);

    /* erase duplicates */
    nodes.sort([](Node* node0, Node* node1) { return node0 < node1; });
    auto last = std::unique(nodes.begin(), nodes.end());
    nodes.erase(last, nodes.end());

    /* sort based on type */
    nodes.sort([](Node* node0, Node* node1) { return node0->sector.type() > node1->sector.type(); });

    for (const auto& node : std::as_const(nodes))
        retval.push_back(node->sector.controllerInfo());

    return retval;
}

void SectorControl::handoffSectorSelect(const std::string& callsign, const std::string& identifier) {
    auto it = this->m_handoffs.find(callsign);
    if (this->m_handoffs.end() == it) {
        this->m_handoffs[callsign] = { false, false, surveillance::FlightRegistry::instance().flight(callsign), nullptr };
        it = this->m_handoffs.find(callsign);
    }

    auto node = SectorControl::findNodeBasedOnIdentifier(this->m_rootNode, identifier);
    if (nullptr != node) {
        it->second.manuallyChanged = true;
        it->second.nextSector = node;
    }
}

bool SectorControl::sectorHandoverPossible() const {
    if (nullptr != this->m_rootNode && nullptr != this->m_ownSector)
        return 0 != this->m_ownSector->controllers.size();
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
