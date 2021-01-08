/*
 * @brief Defines a sector control system
 * @file surveillance/SectorControl.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <list>
#include <map>
#include <memory>

#include <types/Flight.h>
#include <types/Sector.h>

namespace topskytower {
    namespace surveillance {
        /**
         * @brief Describes a sector control system
         * @ingroup surveillance
         */
        class SectorControl {
        private:
            struct Node {
                std::list<std::shared_ptr<Node>> parents;
                types::Sector                    sector;
                std::list<types::ControllerInfo> controllers;
                std::list<std::shared_ptr<Node>> children;

                Node(const types::Sector& sector) :
                        parents(),
                        sector(sector),
                        controllers(),
                        children() { }
            };

            struct FlightData {
                bool                                 manuallyChanged;
                bool                                 handoffPerformed;
                types::Flight                        flight;
                std::shared_ptr<SectorControl::Node> nextSector;
                std::string                          handoffReceivedBy;
            };

            static void insertNode(std::list<std::shared_ptr<Node>>& nodes, const types::Sector& sector);
            static void destroyNode(std::shared_ptr<Node>& node);
            static std::list<std::shared_ptr<Node>> findRelevantSectors(std::list<std::string>& deputies,
                                                                        const std::list<types::Sector>& sectors);
            static std::list<std::list<std::shared_ptr<Node>>> linkSiblings(std::list<std::shared_ptr<Node>>& nodes);
            static void createGraph(std::list<std::list<std::shared_ptr<Node>>>& siblings);
            static std::shared_ptr<Node> findNodeBasedOnIdentifier(const std::shared_ptr<Node>& node,
                                                                   const std::string_view& identifier);
            static std::shared_ptr<Node> findNodeBasedOnInformation(const std::shared_ptr<Node>& node,
                                                                    const types::ControllerInfo& info);
            static std::shared_ptr<Node> findSectorInList(const std::list<std::shared_ptr<Node>>& nodes,
                                                          const types::Position& position,
                                                          types::Flight::Type type, bool lowerSectors);
            std::shared_ptr<Node> findOnlineResponsible(const types::Flight& flight, const types::Position& position,
                                                        bool ignoreClearanceFlag) const;
            std::list<std::shared_ptr<Node>> findSectorCandidates(const std::shared_ptr<Node>& node) const;
            static std::shared_ptr<SectorControl::Node> findLowestSector(const std::shared_ptr<Node>& node,
                                                                         const types::Flight& flight,
                                                                         const types::Position& position,
                                                                         bool ignoreClearanceFlag);
            bool isInOwnSectors(const types::Flight& flight, const types::Position& position, bool ignoreClearanceFlag) const;
            void cleanupHandoffList(std::shared_ptr<Node>& node);

            std::shared_ptr<Node>                                       m_unicom;
            std::shared_ptr<Node>                                       m_rootNode;
            std::shared_ptr<Node>                                       m_ownSector;
            std::map<std::string, FlightData>                           m_handoffs;
            std::map<std::string, std::shared_ptr<SectorControl::Node>> m_sectorsOfFlights;
            std::map<std::string, std::string>                          m_handoffOfFlightsToMe;

        public:
            /**
             * @brief Initializes an empty controller manager
             */
            SectorControl();
            /**
             * @brief Initializes the controller manager
             * @param[in] airport The ICAO code of the controlled airport
             * @param[in] sectors All sectors that need to be controlled
             */
            SectorControl(const std::string& airport, const std::list<types::Sector>& sectors);
            /**
             * Destroys all internal structures
             */
            ~SectorControl();

            /**
             * @brief Updates the online state of a sector based on the information
             * @param[in] info The controller's information
             */
            void controllerUpdate(const types::ControllerInfo& info);
            /**
             * @brief Marks a controller as offline
             * @param[in] info The controller's information
             */
            void controllerOffline(const types::ControllerInfo& info);
            /**
             * @brief Defines the sector of this controller
             * @param[in] info The controller information
             */
            void setOwnSector(const types::ControllerInfo& info);
            /**
             * @brief Returns the own sector
             * @return The own sector
             */
            const types::ControllerInfo& ownSector() const;
            /**
             * @brief Analysis the flight and checks if an handoff is needed
             * @param[in] flight The checked flight
             */
            void updateFlight(const types::Flight& flight);
            /**
             * @brief Removes a flight out of the sector control
             * @param[in] callsign The deletable callsign
             */
            void removeFlight(const std::string& callsign);
            /**
             * @brief Checks if a specific flight is in the own sector
             * @param[in] flight The requested flight
             * @return True if it is in the own sector, else false
             */
            bool isInOwnSector(const types::Flight& flight);
            /**
             * @brief Checks if an handoff is required for a specific callsign
             * @param[in] flight The requested flight
             * @return True if an handoff is required, else false
             */
            bool handoffRequired(const types::Flight& flight) const;
            /**
             * @brief Checks if the aircraft is in the area sector and a handoff is possible
             * @param[in] flight The requested flight
             * @return Trie if an handoff is possible, else false
             */
            bool handoffPossible(const types::Flight& flight) const;
            /**
             * @brief Marks that the handoff is performed for a specific callsign
             * @param[in] flight The requested flight
             */
            void handoffPerformed(const types::Flight& flight);
            /**
             * @brief Returns the next frequency of a specific callsign
             * @param[in] flight The requested flight
             * @return The next frequency
             */
            const types::ControllerInfo& handoffSector(const types::Flight& flight) const;
            /**
             * @brief Returns the next sector controllers of a specific callsign
             * @param[in] flight The requested flight
             * @return The next sector controllers
             */
            std::list<std::string> handoffStations(const types::Flight& flight) const;
            /**
             * @brief Returns all controller information of all sectors that can theoretically take over the flight
             * @return The controller informations of all potential sectors
             */
            std::list<types::ControllerInfo> handoffSectors() const;
            /**
             * @brief Sets the handoff sector manually which avoids automatic overwrites
             * @param[in] flight The requested flight
             * @param[in] identifier The sector's identifier
             */
            void handoffSectorSelect(const types::Flight& flight, const std::string& identifier);
            /**
             * @brief Checks if an handover of the sector is possible
             * @return True if the handover is possible, else false
             */
            bool sectorHandoverPossible() const;
            /**
             * @brief Returns all sector handover candidates
             * @return All handover candidates
             */
            std::list<types::ControllerInfo> sectorHandoverCandidates() const;
            /**
             * @brief Checks if a flight is in one of the handled sectors
             * @param[in] flight The requested flight
             * @return True if it is in the sector tree, else false
             */
            bool isInSector(const types::Flight& flight) const;
        };
    }
}
