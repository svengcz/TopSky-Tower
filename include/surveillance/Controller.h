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
                std::list<Node*> parents;
                types::Sector    sector;
                bool             isOnline;
                std::list<Node*> siblings;
                std::list<Node*> children;

                Node(const types::Sector& sector) :
                        parents(),
                        sector(sector),
                        isOnline(false),
                        siblings(),
                        children() { }
            };

            static void insertNode(std::list<Node*>& nodes, const types::Sector& sector);
            static void destroyNode(Node* node);
            static std::list<Node*> findRelevantSectors(std::list<std::string>& deputies, const std::list<types::Sector>& sectors);
            static void linkSiblings(std::list<Node*>& nodes);
            static Node* createGraph(const std::list<Node*>& nodes);
            void finalizeGraph(const std::list<types::Sector>& sectors);
            static Node* findNode(Controller::Node* node, const std::string_view& identifier);
            static Node* findSectorInList(const std::list<Node*>& nodes, const types::Position& position);
            Node* findNextResponsible(const types::Position& position) const;
            Node* findNextOnline(Node* node);

            Node                                          m_unicom;
            Node*                                         m_rootNode;
            Node*                                         m_ownSector;
            std::map<std::string, std::pair<Node*, bool>> m_handoffs;

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
             * @brief Marks a controller as online
             * @param[in] identifier The sector's identifier
             */
            void controllerOnline(const std::string_view& identifier);
            /**
             * @brief Marks a controller as offline
             * @param[in] identifier The sector's identifier
             */
            void controllerOffline(const std::string_view& identifier);
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
             * @brief Returns the next sector identifier of a specific callsign
             * @param[in] callsign The requested callsign
             * @return The next sector identifier
             */
            const std::string& handoffStation(const std::string& callsign) const;
        };
    }
}
