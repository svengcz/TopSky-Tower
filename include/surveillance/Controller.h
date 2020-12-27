/*
 * @brief Defines a controller management system
 * @file surveillance/Controller.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#include <list>
#include <map>

#include <types/Flight.h>
#include <types/Sector.h>

namespace topskytower {
    namespace surveillance {
        /**
         * @brief Describes a controller management system
         * @ingroup surveillance
         */
        class Controller {
        private:
            struct Node {
                std::list<Node*>                 parents;
                types::Sector                    sector;
                std::list<types::ControllerInfo> controllers;
                std::list<Node*>                 siblings;
                std::list<Node*>                 children;

                Node(const types::Sector& sector) :
                        parents(),
                        sector(sector),
                        controllers(),
                        siblings(),
                        children() { }
            };

            struct FlightData {
                bool          manuallyChanged;
                bool          handoffPerformed;
                types::Flight flight;
                Node*         nextSector;
            };

            static void insertNode(std::list<Node*>& nodes, const types::Sector& sector);
            static void destroyNode(Node* node);
            static std::list<Node*> findRelevantSectors(std::list<std::string>& deputies, const std::list<types::Sector>& sectors);
            static void linkSiblings(std::list<Node*>& nodes);
            static Node* createGraph(const std::list<Node*>& nodes);
            void finalizeGraph(const std::list<types::Sector>& sectors);
            static Node* findNodeBasedOnIdentifier(Node* node, const std::string_view& identifier);
            static Node* findNodeBasedOnInformation(Node* node, const types::ControllerInfo& info);
            static Node* findSectorInList(const std::list<Node*>& nodes, const types::Position& position,
                                          types::Flight::Type type, bool lowerSectors);
            Node* findResponsible(const types::Position& position, types::Flight::Type type) const;
            Node* findOnlineResponsible(Node* node);
            static Node* findLowestSector(Node* node, const types::Position& position);
            bool isInOwnSectors(const types::Position& position) const;
            void cleanupHandoffList(Node* node);

            Node                              m_unicom;
            Node*                             m_rootNode;
            Node*                             m_ownSector;
            std::map<std::string, FlightData> m_handoffs;

        public:
            /**
             * @brief Initializes the controller manager
             * @param[in] airport The ICAO code of the controlled airport
             * @param[in] sectors All sectors that need to be controlled
             */
            Controller(const std::string& airport, const std::list<types::Sector>& sectors);
            /**
             * Destroys all internal structures
             */
            ~Controller();

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
             */
            void setOwnSector(const std::string_view& identifier);
            /**
             * @brief Analysis the flight and checks if an handoff is needed
             * @param[in] flight The checked flight
             */
            void update(const types::Flight& flight);
            /**
             * @brief Checks if an handoff is required for a specific callsign
             * @param[in] callsign The requested callsign
             * @return True if an handoff is required, else false
             */
            bool handoffRequired(const std::string& callsign) const;
            /**
             * @brief Marks that the handoff is performed for a specific callsign
             * @param[in] callsign The requested callsign
             */
            void handoffPerformed(const std::string& callsign);
            /**
             * @brief Returns the next frequency of a specific callsign
             * @param[in] callsign The requested callsign
             * @return The next frequency
             */
            const std::string& handoffFrequency(const std::string& callsign) const;
            /**
             * @brief Returns the next sector controllers of a specific callsign
             * @param[in] callsign The requested callsign
             * @return The next sector controllers
             */
            std::list<std::string> handoffStations(const std::string& callsign) const;
        };
    }
}
