/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the controller manager
 */

#include <surveillance/Controller.h>

using namespace topskytower;
using namespace topskytower::surveillance;
using namespace topskytower::types;

Controller::Controller(const std::string& airport, const std::list<types::Sector>& sectors) :
        m_unicom(types::Sector("", "", "", "FSS", "122.800")),
        m_rootNode(nullptr),
        m_ownSector(nullptr),
        m_handoffs() {
    std::list<types::Sector> airportSectors;

    /* find the tower sectors of the airport */
    for (const auto& sector : std::as_const(sectors)) {
        if (airport == sector.prefix()) {
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
                    if (other.identifier() == deputy) {
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
    auto nodes = Controller::findRelevantSectors(deputies, sectors);
    for (const auto& sector : std::as_const(airportSectors))
        Controller::insertNode(nodes, sector);

    /* sort the sectors from upper to lower airspaces */
    nodes.sort([](Controller::Node* node0, Controller::Node* node1) {
        /* ensure that all types are equal */
        if (node0->sector.type() != node1->sector.type())
            return node0->sector.type() > node1->sector.type();
        /* ensure that the siblings are sorted together */
        else if (node0->sector.prefix() != node1->sector.prefix())
            return node0->sector.prefix() > node1->sector.prefix();
        /* ensure that the sector without borders is lower */
        else if (0 == node0->sector.borders().size() || node1->sector.borders().size())
            return node0->sector.borders().size() > node1->sector.borders().size();
        /* ensure that the upper levels are above */
        else
            return node0->sector.borders().back().upperAltitude() > node1->sector.borders().back().upperAltitude();
    });

    /* create all relevant siblings */
    Controller::linkSiblings(nodes);

    /* create the final graph */
    this->m_rootNode = Controller::createGraph(nodes);

    /* add the centers of the top-level to get the complete hierarchy */
    this->finalizeGraph(sectors);

    this->m_unicom.isOnline = true;
}

Controller::~Controller() {
    Controller::destroyNode(this->m_rootNode);
    this->m_ownSector = nullptr;
    this->m_handoffs.clear();
}

void Controller::destroyNode(Node* node) {
    if (nullptr == node)
        return;

    for (auto& sibling : node->siblings)
        delete sibling;
    node->siblings.clear();

    for (auto& child : node->children)
        Controller::destroyNode(child);

    delete node;
}

void Controller::insertNode(std::list<Node*>& nodes, const types::Sector& sector) {
    /* ignore the delivery */
    if (types::Sector::Type::Delivery == sector.type())
        return;

    /* check if the sector is already a node */
    bool alreadyRegistered = false;
    for (const auto& node : std::as_const(nodes)) {
        if (node->sector.identifier() == sector.identifier()) {
            alreadyRegistered = true;
            break;
        }
    }

    /* create the new node */
    if (false == alreadyRegistered) {
        Controller::Node* node = new Controller::Node(sector);
        nodes.push_back(node);
    }
}

std::list<Controller::Node*> Controller::findRelevantSectors(std::list<std::string>& deputies, const std::list<types::Sector>& sectors) {
    std::list<Controller::Node*> retval;

    /* filter out the centers */
    for (auto it = deputies.begin(); deputies.end() != it;) {
        bool erased = false, found = false;

        for (const auto& sector : std::as_const(sectors)) {
            if (*it == sector.identifier()) {
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
            if (sector.identifier() == deputy) {
                /* assume that all approaches have the same prefix */
                if (types::Sector::Type::Approach == sector.type() && 0 == approachPrefix.length())
                    approachPrefix = sector.prefix();

                if (types::Sector::Type::Approach == sector.type()) {
                    if (approachPrefix == sector.prefix())
                        Controller::insertNode(retval, sector);
                }
                else {
                    Controller::insertNode(retval, sector);
                }
            }
            else if (0 != sector.borders().size()) {
                auto it = std::find(sector.borders().front().deputies().cbegin(), sector.borders().front().deputies().cend(), deputy);
                if (sector.borders().front().deputies().cend() != it) {
                    if (types::Sector::Type::Approach == sector.type()) {
                        if (approachPrefix == sector.prefix())
                            Controller::insertNode(retval, sector);
                    }
                    else {
                        Controller::insertNode(retval, sector);
                    }
                }
            }
        }
    }

    return retval;
}

void Controller::linkSiblings(std::list<Controller::Node*>& nodes) {
    Controller::Node* lastNode = nullptr;
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
                    if (lastNode->sector.prefix() == (*it)->sector.prefix()) {
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

Controller::Node* Controller::createGraph(const std::list<Controller::Node*>& nodes) {
    std::list<Controller::Node*> parents;
    Controller::Node* root = nullptr;

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
                    if (types::Sector::Type::Tower == child->sector.type() && child->sector.prefix() == node->sector.prefix()) {
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

void Controller::finalizeGraph(const std::list<types::Sector>& sectors) {
    /* unable to find other sectors */
    if (nullptr == this->m_rootNode || 0 == this->m_rootNode->sector.borders().size())
        return;

    for (const auto& deputy : std::as_const(this->m_rootNode->sector.borders().front().deputies())) {
        for (const auto& sector : std::as_const(sectors)) {
            if (sector.identifier() == deputy && types::Sector::Type::Center <= sector.type()) {
                Controller::Node* newNode = new Controller::Node(sector);

                newNode->children.push_back(this->m_rootNode);
                this->m_rootNode->parents.push_back(newNode);
                this->m_rootNode = newNode;
            }
        }
    }
}

Controller::Node* Controller::findNode(Controller::Node* node, const std::string_view& identifier) {
    if (nullptr == node)
        return nullptr;

    /* this node is the correct one */
    if (node->sector.identifier() == identifier)
        return node;

    /* check the siblings */
    for (const auto& sibling : std::as_const(node->siblings)) {
        if (sibling->sector.identifier() == identifier)
            return sibling;
    }

    /* check every child recursivly */
    for (const auto& child : std::as_const(node->children)) {
        auto retval = Controller::findNode(child, identifier);
        if (nullptr != retval)
            return retval;
    }

    return nullptr;
}

void Controller::controllerOnline(const std::string_view& identifier) {
    auto node = Controller::findNode(this->m_rootNode, identifier);
    if (nullptr != node)
        node->isOnline = true;
}

void Controller::controllerOffline(const std::string_view& identifier) {
    auto node = Controller::findNode(this->m_rootNode, identifier);
    if (nullptr != node)
        node->isOnline = false;
}

void Controller::setOwnSector(const std::string_view& identifier) {
    /* mark the same sector as the own sector */
    if (nullptr != this->m_ownSector && identifier == this->m_ownSector->sector.identifier())
        return;

    this->m_ownSector = Controller::findNode(this->m_rootNode, identifier);
}

Controller::Node* Controller::findSectorInList(const std::list<Controller::Node*>& nodes, const types::Position& position) {
    for (const auto& node : std::as_const(nodes)) {
        if (true == node->sector.isInsideSector(position))
            return node;
    }

    return nullptr;
}

Controller::Node* Controller::findNextResponsible(const types::Position& position) const {
    Controller::Node* next = nullptr;

    /* test the children */
    next = Controller::findSectorInList(this->m_ownSector->children, position);

    /* test the siblings */
    if (nullptr == next)
        next = Controller::findSectorInList(this->m_ownSector->siblings, position);

    /* test the parents */
    if (nullptr == next)
        next = Controller::findSectorInList(this->m_ownSector->parents, position);

    return next;
}

Controller::Node* Controller::findNextOnline(Controller::Node* node) {
    /* avoid wrong calls */
    if (nullptr == node)
        return &this->m_unicom;

    /* this is the next online station */
    if (true == node->isOnline)
        return node;

    /* check which deputy is online */
    for (const auto& deputy : std::as_const(node->sector.borders().front().deputies())) {
        auto deputyNode = Controller::findNode(this->m_rootNode, deputy);
        if (nullptr != deputyNode && true == deputyNode->isOnline)
            return deputyNode;
    }

    /* no other real station is online */
    return &this->m_unicom;
}

void Controller::update(const types::Flight& flight) {
    if (nullptr == this->m_rootNode || nullptr == this->m_ownSector)
        return;

    auto it = this->m_handoffs.find(flight.callsign());
    if (true == this->m_ownSector->sector.isInsideSector(flight.currentPosition())) {
        /* the aircraft remains in own sector */
        auto predicted = flight.predict(30_s, 20_kn);
        if (true == this->m_ownSector->sector.isInsideSector(predicted))
            return;

        /* get the next responsible node and the next online station */
        auto nextNode = this->findNextResponsible(predicted);
        auto nextOnline = this->findNextOnline(nextNode);

        this->m_handoffs[flight.callsign()] = std::make_pair(nextOnline, false);
    }
    else {
        /* check if we have to remove the handoff information */
        if (this->m_handoffs.end() != it && true == it->second.second)
            this->m_handoffs.erase(it);
    }
}

bool Controller::handoffRequired(const std::string& callsign) const {
    auto it = this->m_handoffs.find(callsign);
    if (this->m_handoffs.cend() != it)
        return it->second.second;
    return false;
}

void Controller::handoffPerformed(const std::string& callsign) {
    auto it = this->m_handoffs.find(callsign);
    if (this->m_handoffs.end() != it)
        it->second.second = true;
}

const std::string& Controller::handoffFrequency(const std::string& callsign) const {
    auto it = this->m_handoffs.find(callsign);
    return it->second.first->sector.frequency();
}

const std::string& Controller::handoffStation(const std::string& callsign) const {
    auto it = this->m_handoffs.find(callsign);
    return it->second.first->sector.identifier();
}
