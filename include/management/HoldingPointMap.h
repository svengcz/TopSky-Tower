/*
 * @brief Defines a holding point managment structure
 * @file management/HoldingPointMap.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#ifndef DOXYGEN_IGNORE

#include <vector>

#include <GeographicLib/Gnomonic.hpp>
#include <nanoflann.hpp>

#include <system/ConfigurationRegistry.h>
#include <types/AirportConfiguration.h>
#include <types/Flight.h>

namespace topskytower {
    namespace management {
        /**
         * @brief Defines a structure to map a WGS84 holding point to Cartesian coordinates
         */
        struct HoldingPointData : public types::HoldingPoint {
            float cartesianPosition[2]; /**< The mapped Cartesian coordinates */

            /**
             * @brief Creates an empty holding point map
             */
            HoldingPointData() :
                    types::HoldingPoint(),
                    cartesianPosition{ 0.0f, 0.0f } { }
            /**
             * @brief Creats a holding point map
             * @param[in] holdingPoint The holding point definition
             */
            HoldingPointData(const types::HoldingPoint& holdingPoint) :
                    types::HoldingPoint(holdingPoint),
                    cartesianPosition{ 0.0f, 0.0f } { }
        };

        /**
         * @brief Defines and implements a holding point management structure
         * @ingroup management
         */
        template <typename T>
        class HoldingPointMap {
        private:
            struct HoldingPointTree {
                std::vector<T> holdingPoints;

                inline std::size_t kdtree_get_point_count() const {
                    return holdingPoints.size();
                }
                inline float kdtree_get_pt(const std::size_t idx, const std::size_t dimension) const {
                    return this->holdingPoints[idx].cartesianPosition[dimension];
                }
                template <class BBOX>
                bool kdtree_get_bbox(BBOX&) const {
                    return false;
                }
            };
            typedef nanoflann::KDTreeSingleIndexAdaptor<nanoflann::L2_Simple_Adaptor<float, HoldingPointTree>,
                                                        HoldingPointTree, 2> HoldingPointTreeAdaptor;

            std::string              m_airportIcao;
            types::Coordinate        m_centerPosition;
            HoldingPointTree         m_normalHoldingPointTree;
            HoldingPointTreeAdaptor* m_normalHoldingPointTreeAdaptor;
            HoldingPointTree         m_lvpHoldingPointTree;
            HoldingPointTreeAdaptor* m_lvpHoldingPointTreeAdaptor;

            static void normalize(types::Angle& angle) {
                while (-180.0 * types::degree > angle)
                    angle += 360.0 * types::degree;
                while (180.0 * types::degree < angle)
                    angle -= 360.0 * types::degree;
            }
            T* findNextHoldingPoints(const types::Flight& flight, types::Flight::Type type, bool runwayBound) {
                /* get the correct adaptor */
                HoldingPointTreeAdaptor* adaptor;
                if (false == system::ConfigurationRegistry::instance().runtimeConfiguration().lowVisibilityProcedures)
                    adaptor = this->m_normalHoldingPointTreeAdaptor;
                else
                    adaptor = this->m_lvpHoldingPointTreeAdaptor;

                /* avoid uninitialized calls */
                if (nullptr == adaptor)
                    return nullptr;

                /* project to Cartesian coordinates */
                GeographicLib::Gnomonic projection(GeographicLib::Geodesic::WGS84());
                float queryPt[2];
                projection.Forward(this->m_centerPosition.latitude().convert(types::degree),
                                   this->m_centerPosition.longitude().convert(types::degree),
                                   flight.currentPosition().coordinate().latitude().convert(types::degree),
                                   flight.currentPosition().coordinate().longitude().convert(types::degree),
                                   queryPt[0], queryPt[1]);

                std::size_t idx;
                float distance;

                /* find the neighbors */
                std::size_t found = adaptor->knnSearch(queryPt, 1, &idx, &distance);

                const auto& expectedRunway = types::Flight::Type::Departure == type ? flight.flightPlan().departureRunway() : flight.flightPlan().arrivalRunway();
                bool lvpActive = system::ConfigurationRegistry::instance().runtimeConfiguration().lowVisibilityProcedures;
                if (1 == found) {
                    T* retval;

                    if (false == lvpActive)
                        retval = &this->m_normalHoldingPointTree.holdingPoints[idx];
                    else
                        retval = &this->m_lvpHoldingPointTree.holdingPoints[idx];

                    /* check the runway bound */
                    if (true == runwayBound && expectedRunway != retval->runway)
                        return nullptr;

                    /* check if the flight is close enough */
                    auto hpDistance = retval->holdingPoint.distanceTo(flight.currentPosition().coordinate());
                    if (hpDistance > system::ConfigurationRegistry::instance().systemConfiguration().ariwsMaximumDistance)
                        return nullptr;

                    return retval;
                }

                return nullptr;
            }

        public:
            /**
             * @brief Initializes the holding point map
             * @param[in] airport The airport's ICAO code
             * @param[in] center The airport's center position
             */
            HoldingPointMap(const std::string& airport, const types::Coordinate& center) :
                    m_airportIcao(airport),
                    m_centerPosition(center),
                    m_normalHoldingPointTree(),
                    m_normalHoldingPointTreeAdaptor(nullptr),
                    m_lvpHoldingPointTree(),
                    m_lvpHoldingPointTreeAdaptor(nullptr) { }
            /**
             * @brief Deletes all internal structures
             */
            ~HoldingPointMap() {
                if (nullptr != this->m_normalHoldingPointTreeAdaptor)
                    delete this->m_normalHoldingPointTreeAdaptor;
                if (nullptr != this->m_lvpHoldingPointTreeAdaptor)
                    delete this->m_lvpHoldingPointTreeAdaptor;
            }

            /**
             * @brief Reinitializes the holding point map with all internal structures
             */
            void reinitialize() {
                /* delete all old information */
                if (nullptr != this->m_normalHoldingPointTreeAdaptor)
                    delete this->m_normalHoldingPointTreeAdaptor;
                this->m_normalHoldingPointTreeAdaptor = nullptr;
                if (nullptr != this->m_lvpHoldingPointTreeAdaptor)
                    delete this->m_lvpHoldingPointTreeAdaptor;
                this->m_lvpHoldingPointTreeAdaptor = nullptr;

                const auto& config = system::ConfigurationRegistry::instance().airportConfiguration(this->m_airportIcao);
                if (false == config.valid || 0 == config.aircraftStands.size())
                    return;

                GeographicLib::Gnomonic projection(GeographicLib::Geodesic::WGS84());
                for (const auto& holdingPoint : std::as_const(config.holdingPoints)) {
                    T data(holdingPoint);

                    projection.Forward(this->m_centerPosition.latitude().convert(types::degree),
                                       this->m_centerPosition.longitude().convert(types::degree),
                                       data.holdingPoint.latitude().convert(types::degree),
                                       data.holdingPoint.longitude().convert(types::degree),
                                       data.cartesianPosition[0],
                                       data.cartesianPosition[1]);

                    if (true == data.lowVisibility)
                        this->m_lvpHoldingPointTree.holdingPoints.push_back(std::move(data));
                    else
                        this->m_normalHoldingPointTree.holdingPoints.push_back(std::move(data));
                }

#pragma warning(disable: 4127)
                this->m_normalHoldingPointTreeAdaptor = new HoldingPointTreeAdaptor(2, this->m_normalHoldingPointTree,
                                                                                    nanoflann::KDTreeSingleIndexAdaptorParams(10));
                this->m_lvpHoldingPointTreeAdaptor = new HoldingPointTreeAdaptor(2, this->m_lvpHoldingPointTree,
                                                                                 nanoflann::KDTreeSingleIndexAdaptorParams(10));
#pragma warning(default: 4127)
                this->m_normalHoldingPointTreeAdaptor->buildIndex();
                this->m_lvpHoldingPointTreeAdaptor->buildIndex();
            }
            /**
             * @brief Checks if a flight reached a holding point but did not pass it (except to the deadbandWidth distance)
             * @param[in] flight The requested flight
             * @param[in] type The flight's type
             * @param[in] runwayBound True if the holding point is runway relevant, else false
             * @param[in] deadbandWidth The distance between the flight and the holding point that ignores if the flight passed or is standing in front
             * @param[in] threshold The angular threshold that defines if a flight is facing the holding point or not
             */
            bool reachedHoldingPoint(const types::Flight& flight, types::Flight::Type type, bool runwayBound, const types::Length& deadbandWidth,
                                     const types::Angle& threshold) {
                /* find the next holding point */
                auto node = this->findNextHoldingPoints(flight, type, runwayBound);
                if (nullptr == node)
                    return false;

                /* inside the deadband -> mark it as reached */
                auto distance = flight.currentPosition().coordinate().distanceTo(node->holdingPoint);
                if (distance <= deadbandWidth)
                    return true;

                /* calculate the heading */
                auto heading = flight.currentPosition().coordinate().bearingTo(node->holdingPoint) - node->heading;
                if (types::Flight::Type::Departure != type)
                    heading -= 180.0_deg;
                HoldingPointMap<T>::normalize(heading);

                return heading.abs() <= threshold;
            }
            /**
             * @brief Checks if a flight passed a holding point (except to the deadbandWidth distance)
             * @param[in] flight The requested flight
             * @param[in] type The flight's type
             * @param[in] runwayBound True if the holding point is runway relevant, else false
             * @param[in] deadbandWidth The distance between the flight and the holding point that ignores if the flight passed or is standing in front
             * @param[in] threshold The angular threshold that defines if a flight is facing the holding point or not
             */
            bool passedHoldingPoint(const types::Flight& flight, types::Flight::Type type, bool runwayBound, const types::Length& deadbandWidth,
                                    const types::Angle& threshold) {
                /* find the next holding point */
                auto node = this->findNextHoldingPoints(flight, type, runwayBound);
                if (nullptr == node)
                    return false;

                /* calculate the heading and compensate the offset */
                auto heading = flight.currentPosition().coordinate().bearingTo(node->holdingPoint) - node->heading;
                if (types::Flight::Type::Departure == type)
                    heading -= 180.0 * types::degree;
                HoldingPointMap<T>::normalize(heading);

                auto distance = flight.currentPosition().coordinate().distanceTo(node->holdingPoint);

                return heading.abs() <= threshold && distance > deadbandWidth;
            }
            /**
             * @brief Returns the center of the holding point map
             * @return The center of the map
             */
            const types::Coordinate& center() const {
                return this->m_centerPosition;
            }
        };
    }
}

#endif
