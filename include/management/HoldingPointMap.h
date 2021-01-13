/*
 * @brief Defines a holding point managment structure
 * @file management/HoldingPointMap.h
 * @author Sven Czarnian <devel@svcz.de>
 */

#pragma once

#ifndef DOXYGEN_IGNORE

#include <vector>

#include <nanoflann.hpp>

#include <system/ConfigurationRegistry.h>
#include <types/AirportConfiguration.h>
#include <types/Flight.h>

namespace topskytower {
    namespace management {
        struct HoldingPointData : public types::HoldingPoint {
            float cartesianPosition[2];

            HoldingPointData() :
                    types::HoldingPoint(),
                    cartesianPosition{ 0.0f, 0.0f } { }
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
        protected:
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
             * @brief Creates an holding point map
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

            T* findNextHoldingPoint(const types::Flight& flight) {
                /* get the correct adaptor */
                HoldingPointTreeAdaptor* adaptor;
                if (false == system::ConfigurationRegistry::instance().runtimeConfiguration().lowVisibilityProcedures)
                    adaptor = this->m_normalHoldingPointTreeAdaptor;
                else
                    adaptor = this->m_lvpHoldingPointTreeAdaptor;

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

                /* something went wrong in the search-call */
                if (1 != adaptor->knnSearch(queryPt, 1, &idx, &distance))
                    return nullptr;;

                if (false == system::ConfigurationRegistry::instance().runtimeConfiguration().lowVisibilityProcedures)
                    return &this->m_normalHoldingPointTree.holdingPoints[idx];
                else
                    return &this->m_lvpHoldingPointTree.holdingPoints[idx];
            }

        public:
            /**
             * @brief Deletes all internal structures
             */
            ~HoldingPointMap() {
                if (nullptr != this->m_normalHoldingPointTreeAdaptor)
                    delete this->m_normalHoldingPointTreeAdaptor;
                if (nullptr != this->m_lvpHoldingPointTreeAdaptor)
                    delete this->m_lvpHoldingPointTreeAdaptor;
            }
        };
    }
}

#endif
