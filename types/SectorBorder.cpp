/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the sector definition
 */

#include <algorithm>

#include <GeographicLib/Gnomonic.hpp>

#include <types/SectorBorder.h>

using namespace topskytower;
using namespace topskytower::types;

SectorBorder::SectorBorder() noexcept :
        m_owner(),
        m_deputies(),
        m_lowerAltitude(),
        m_upperAltitude(),
        m_centroid(),
        m_edges(),
        m_shape(),
        m_boundingBox{ { std::numeric_limits<float>::max() * types::degree, -std::numeric_limits<float>::max() * types::degree },
                       { std::numeric_limits<float>::max() * types::degree, -std::numeric_limits<float>::max() * types::degree } } { }

SectorBorder::SectorBorder(std::string&& owner, std::vector<std::string>&& deputies, const types::Length& lowerAltitude,
                           const types::Length& upperAltitude) noexcept :
        m_owner(std::move(owner)),
        m_deputies(std::move(deputies)),
        m_lowerAltitude(lowerAltitude),
        m_upperAltitude(upperAltitude),
        m_centroid(),
        m_edges(),
        m_shape(),
        m_boundingBox{ { std::numeric_limits<float>::max() * types::degree, -std::numeric_limits<float>::max() * types::degree },
                       { std::numeric_limits<float>::max() * types::degree, -std::numeric_limits<float>::max() * types::degree } } { }

SectorBorder::SectorBorder(const SectorBorder& other) noexcept :
        m_owner(other.m_owner),
        m_deputies(other.m_deputies),
        m_lowerAltitude(other.m_lowerAltitude),
        m_upperAltitude(other.m_upperAltitude),
        m_centroid(other.m_centroid),
        m_edges(other.m_edges),
        m_shape(other.m_shape),
        m_boundingBox{ { other.m_boundingBox[0][0], other.m_boundingBox[0][1] },
                       { other.m_boundingBox[1][0], other.m_boundingBox[1][1] } } { }

SectorBorder::SectorBorder(SectorBorder&& other) noexcept :
        m_owner(std::move(other.m_owner)),
        m_deputies(std::move(other.m_deputies)),
        m_lowerAltitude(other.m_lowerAltitude),
        m_upperAltitude(other.m_upperAltitude),
        m_centroid(std::move(other.m_centroid)),
        m_edges(std::move(other.m_edges)),
        m_shape(std::move(other.m_shape)),
        m_boundingBox{ { other.m_boundingBox[0][0], other.m_boundingBox[0][1] },
                       { other.m_boundingBox[1][0], other.m_boundingBox[1][1] } } { }

SectorBorder& SectorBorder::operator=(const SectorBorder& other) noexcept {
    if (&other != this) {
        this->m_owner = other.m_owner;
        this->m_deputies = other.m_deputies;
        this->m_lowerAltitude = other.m_lowerAltitude;
        this->m_upperAltitude = other.m_upperAltitude;
        this->m_centroid = other.m_centroid;
        this->m_edges = other.m_edges;
        this->m_shape = other.m_shape;

        for (int i = 0; i < 2; ++i) {
            for (int c = 0; c < 2; ++c)
                this->m_boundingBox[i][c] = other.m_boundingBox[i][c];
        }
    }
    return *this;
}

SectorBorder& SectorBorder::operator=(SectorBorder&& other) noexcept {
    if (&other != this) {
        this->m_owner = std::move(other.m_owner);
        this->m_deputies = std::move(other.m_deputies);
        this->m_lowerAltitude = other.m_lowerAltitude;
        this->m_upperAltitude = other.m_upperAltitude;
        this->m_centroid = std::move(other.m_centroid);
        this->m_edges = std::move(other.m_edges);
        this->m_shape = std::move(other.m_shape);

        for (int i = 0; i < 2; ++i) {
            for (int c = 0; c < 2; ++c)
                this->m_boundingBox[i][c] = other.m_boundingBox[i][c];
        }
    }
    return *this;
}

const std::string& SectorBorder::owner() const {
    return this->m_owner;
}

const std::vector<std::string>& SectorBorder::deputies() const {
    return this->m_deputies;
}

const types::Length& SectorBorder::lowerAltitude() const {
    return this->m_lowerAltitude;
}

const types::Length& SectorBorder::upperAltitude() const {
    return this->m_upperAltitude;
}

void SectorBorder::setEdges(const std::list<types::Coordinate>& edges) {
    this->m_edges.clear();

    if (3 > edges.size())
        return;

    this->m_boundingBox[0][0] = std::numeric_limits<float>::max() * types::degree;
    this->m_boundingBox[0][1] = -std::numeric_limits<float>::max() * types::degree;
    this->m_boundingBox[1][0] = std::numeric_limits<float>::max() * types::degree;
    this->m_boundingBox[1][1] = -std::numeric_limits<float>::max() * types::degree;

    /* create the polygon and calculate the center point */
    float avgLon = 0.0f, avgLat = 0.0f;
    for (const auto& edge : std::as_const(edges)) {
        this->m_boundingBox[0][0] = std::min(this->m_boundingBox[0][0], edge.longitude());
        this->m_boundingBox[0][1] = std::max(this->m_boundingBox[0][1], edge.longitude());
        this->m_boundingBox[1][0] = std::min(this->m_boundingBox[1][0], edge.latitude());
        this->m_boundingBox[1][1] = std::max(this->m_boundingBox[1][1], edge.latitude());

        avgLon += edge.longitude().convert(types::degree);
        avgLat += edge.latitude().convert(types::degree);
    }

    /* calculate the centroid */
    avgLon /= edges.size();
    avgLat /= edges.size();
    this->m_centroid = types::Coordinate(avgLon * types::degree, avgLat * types::degree);

    /* calculate the Cartesian shape */
    this->m_shape = bg::model::polygon<bg::model::point<float, 2, bg::cs::cartesian>>();
    GeographicLib::Gnomonic projection(GeographicLib::Geodesic::WGS84());
    for (const auto& edge : std::as_const(edges)) {
        float x, y;

        projection.Forward(avgLat, avgLon, edge.latitude().convert(types::degree),
                           edge.longitude().convert(types::degree), x, y);

        bg::append(this->m_shape, bg::model::point<float, 2, bg::cs::cartesian>(x, y));
    }

    /* finalize the shape */
    bg::correct(this->m_shape);

    this->m_edges = edges;
}

const std::list<types::Coordinate>& SectorBorder::edges() const {
    return this->m_edges;
}

bool SectorBorder::isInsideBorder(const types::Coordinate& coordinate) const {
    /* check if the bounding box does not contain the coordinate */
    if (this->m_boundingBox[0][0] > coordinate.longitude() || this->m_boundingBox[0][1] < coordinate.longitude())
        return false;
    if (this->m_boundingBox[1][0] > coordinate.latitude() || this->m_boundingBox[1][1] < coordinate.latitude())
        return false;

    /* convert the point into cartesian coordinates */
    float x, y;
    GeographicLib::Gnomonic projection(GeographicLib::Geodesic::WGS84());
    projection.Forward(this->m_centroid.latitude().convert(types::degree), this->m_centroid.longitude().convert(types::degree),
                       coordinate.latitude().convert(types::degree), coordinate.longitude().convert(types::degree), x, y);

    return bg::within(bg::model::point<float, 2, bg::cs::cartesian>(x, y), this->m_shape);
}

bool SectorBorder::isInsideBorder(const types::Position& position) const {
    if (this->m_lowerAltitude > position.altitude() || this->m_upperAltitude < position.altitude())
        return false;
    return this->isInsideBorder(position.coordinate());
}
