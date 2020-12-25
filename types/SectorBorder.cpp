/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the sector definition
 */

#include <types/SectorBorder.h>

using namespace topskytower;
using namespace topskytower::types;

SectorBorder::SectorBorder() noexcept :
        m_owner(),
        m_deputies(),
        m_lowerAltitude(),
        m_upperAltitude(),
        m_edges(),
        m_segments(),
        m_boundingBox{ { std::numeric_limits<float>::max() * types::degree, -std::numeric_limits<float>::max() * types::degree },
                       { std::numeric_limits<float>::max() * types::degree, -std::numeric_limits<float>::max() * types::degree } } { }

SectorBorder::SectorBorder(std::string&& owner, std::vector<std::string>&& deputies, const types::Length& lowerAltitude,
                           const types::Length& upperAltitude) noexcept :
        m_owner(std::move(owner)),
        m_deputies(std::move(deputies)),
        m_lowerAltitude(lowerAltitude),
        m_upperAltitude(upperAltitude),
        m_edges(),
        m_segments(),
        m_boundingBox{ { std::numeric_limits<float>::max() * types::degree, -std::numeric_limits<float>::max() * types::degree },
                       { std::numeric_limits<float>::max() * types::degree, -std::numeric_limits<float>::max() * types::degree } } { }

SectorBorder::SectorBorder(const SectorBorder& other) noexcept :
        m_owner(other.m_owner),
        m_deputies(other.m_deputies),
        m_lowerAltitude(other.m_lowerAltitude),
        m_upperAltitude(other.m_upperAltitude),
        m_edges(other.m_edges),
        m_segments(other.m_segments),
        m_boundingBox{ { other.m_boundingBox[0][0], other.m_boundingBox[0][1] },
                       { other.m_boundingBox[1][0], other.m_boundingBox[1][1] } } { }

SectorBorder::SectorBorder(SectorBorder&& other) noexcept :
        m_owner(std::move(other.m_owner)),
        m_deputies(std::move(other.m_deputies)),
        m_lowerAltitude(other.m_lowerAltitude),
        m_upperAltitude(other.m_upperAltitude),
        m_edges(std::move(other.m_edges)),
        m_segments(std::move(other.m_segments)),
        m_boundingBox{ { other.m_boundingBox[0][0], other.m_boundingBox[0][1] },
                       { other.m_boundingBox[1][0], other.m_boundingBox[1][1] } } { }

SectorBorder& SectorBorder::operator=(const SectorBorder& other) noexcept {
    if (&other != this) {
        this->m_owner = other.m_owner;
        this->m_deputies = other.m_deputies;
        this->m_lowerAltitude = other.m_lowerAltitude;
        this->m_upperAltitude = other.m_upperAltitude;
        this->m_edges = other.m_edges;
        this->m_segments = other.m_segments;

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
        this->m_edges = std::move(other.m_edges);
        this->m_segments = std::move(other.m_segments);

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

void SectorBorder::calculateLineParameters(LineSegment& segment) {
    segment.parameters[0] = segment.points[1].latitude().value() - segment.points[0].latitude().value();
    segment.parameters[1] = segment.points[0].longitude().value() - segment.points[1].longitude().value();
    segment.parameters[2] = segment.points[1].longitude().value() * segment.points[0].latitude().value();
    segment.parameters[2] -= segment.points[0].longitude().value() * segment.points[1].latitude().value();
}

void SectorBorder::addEdge(const types::Coordinate& coord0, const types::Coordinate& coord1) {
    /* calculate the bounding box */
    this->m_boundingBox[0][0] = std::min(coord0.longitude(), this->m_boundingBox[0][0]);
    this->m_boundingBox[0][1] = std::max(coord0.longitude(), this->m_boundingBox[0][1]);
    this->m_boundingBox[1][0] = std::min(coord0.latitude(), this->m_boundingBox[1][0]);
    this->m_boundingBox[1][1] = std::max(coord0.latitude(), this->m_boundingBox[1][1]);
    this->m_boundingBox[0][0] = std::min(coord1.longitude(), this->m_boundingBox[0][0]);
    this->m_boundingBox[0][1] = std::max(coord1.longitude(), this->m_boundingBox[0][1]);
    this->m_boundingBox[1][0] = std::min(coord1.latitude(), this->m_boundingBox[1][0]);
    this->m_boundingBox[1][1] = std::max(coord1.latitude(), this->m_boundingBox[1][1]);

    /* calculate and add the line segment */
    LineSegment segment;
    segment.points[0] = coord0;
    segment.points[1] = coord1;
    SectorBorder::calculateLineParameters(segment);
    this->m_segments.push_back(segment);

    /* insert the point if it does not exist */
    auto it = std::find(this->m_edges.cbegin(), this->m_edges.cend(), coord0);
    if (it == this->m_edges.cend())
        this->m_edges.push_back(coord0);

    /* insert the point if it does not exist */
    it = std::find(this->m_edges.cbegin(), this->m_edges.cend(), coord1);
    if (it == this->m_edges.cend())
        this->m_edges.push_back(coord0);
}

const std::vector<types::Coordinate>& SectorBorder::edges() const {
    return this->m_edges;
}

const std::vector<SectorBorder::LineSegment>& SectorBorder::segments() const {
    return this->m_segments;
}

bool SectorBorder::intersects(const LineSegment& segment0, const LineSegment& segment1) {
    float d1 = segment0.parameters[0] * segment1.points[0].longitude().value() +
               segment0.parameters[1] * segment1.points[0].latitude().value() +
               segment0.parameters[2];
    float d2 = segment0.parameters[0] * segment1.points[1].longitude().value() +
               segment0.parameters[1] * segment1.points[1].latitude().value() +
               segment0.parameters[2];
    if (0.0f >= d1 * d2)
        return false;

    d1 = segment1.parameters[0] * segment0.points[0].longitude().value() +
         segment1.parameters[1] + segment0.points[0].latitude().value() +
         segment1.parameters[2];
    d2 = segment1.parameters[0] * segment0.points[1].longitude().value() +
         segment1.parameters[1] + segment0.points[1].latitude().value() +
         segment1.parameters[2];
    if (0.0f >= d1 * d2)
        return false;

    /* test if the point is inside the line */
    if (true == helper::Math::almostEqual(segment1.parameters[0] * segment0.parameters[1] -
                                          segment0.parameters[0] * segment1.parameters[1], 0.0f)) {
        auto checkDistance = segment1.points[0].distanceTo(segment1.points[1]);

        /* check if the tested point is inside the segment of the second line */
        if (checkDistance > segment1.points[0].distanceTo(segment0.points[1]) || checkDistance > segment1.points[1].distanceTo(segment0.points[1]))
            return false;
    }

    return true;
}

bool SectorBorder::isInsideBorder(const types::Coordinate& coordinate) const {
    /* a polygon needs to contain at least three line segments */
    if (3 > this->m_segments.size())
        return false;

    /* check if the bounding box does not contain the coordinate */
    if (this->m_boundingBox[0][0] > coordinate.longitude() || this->m_boundingBox[0][1] < coordinate.longitude())
        return false;
    if (this->m_boundingBox[1][0] > coordinate.latitude() || this->m_boundingBox[1][1] < coordinate.latitude())
        return false;

    /* calculate the line segment of the searched point and a point outside of the polygon */
    LineSegment testSegment;
    testSegment.points[0] = coordinate;
    testSegment.points[1] = types::Coordinate(types::Angle(this->m_boundingBox[0][0] - 0.1f * types::degree), coordinate.latitude());
    SectorBorder::calculateLineParameters(testSegment);

    /* check how many line segments are intersected */
    std::size_t intersections = 0;
    for (const auto& segment : std::as_const(this->m_segments)) {
        if (true == SectorBorder::intersects(segment, testSegment))
            intersections += 1;
    }

    return 0 != (intersections % 2);
}

bool SectorBorder::isInsideBorder(const types::Position& position) const {
    if (this->m_lowerAltitude > position.altitude() || this->m_upperAltitude < position.altitude())
        return false;
    return this->isInsideBorder(position.coordinate());
}
