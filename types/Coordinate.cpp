/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the coordinate to abstract global coordinates
 */

#define _USE_MATH_DEFINES
#include <algorithm>
#include <cmath>

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/algorithms/detail/course.hpp>
#include <boost/geometry/geometries/register/point.hpp>

#include <helper/Exception.h>
#include <helper/Math.h>
#include <helper/String.h>
#include <types/Coordinate.h>

using namespace topskytower;
using namespace topskytower::types;

namespace bg = boost::geometry;

BOOST_GEOMETRY_REGISTER_POINT_2D_GET_SET(
    Coordinate,
    float,
    bg::cs::geographic<bg::degree>,
    Coordinate::latitudeDegree,
    Coordinate::longitudeDegree,
    Coordinate::setLatitudeDegree,
    Coordinate::setLongitudeDegree
)

Coordinate::Coordinate() :
        m_longitude(0.0f),
        m_latitude(0.0f) { }

Coordinate::Coordinate(const Angle& longitude, const Angle& latitude) :
        m_longitude(longitude),
        m_latitude(latitude) { }

static __inline Angle __coordinateToDecimal(const std::string& coordinate) {
    /* split the coordinate and validate the number of elements */
    auto split = helper::String::splitString(coordinate, ".");
    if (4 != split.size())
        helper::Exception("Coordinate", "Unable to convert string coordinate to decimal. Expecting 4 entries, splitted by '.'");

    /* get the sign factor depending on the global direction flag */
    float signFactor = 1.0;
    switch (static_cast<char>(std::tolower(split[0][0]))) {
    case 'n':
    case 'e':
        break;
    case 'w':
    case 's':
        signFactor = -1.0;
        break;
    default:
        throw helper::Exception("Coordinate", "Invalid N,E,S,W entry found.");
    }

    /* convert the values */
    float degrees = static_cast<float>(std::atoi(split[0].substr(1).c_str()));
    float minutes = static_cast<float>(std::atoi(split[1].c_str()));
    float seconds = static_cast<float>(std::atoi(split[2].c_str()));
    float milliseconds = static_cast<float>(std::atoi(split[3].c_str()));

    /* calculate the dd value */
    float dd = degrees + minutes / 60.0f + seconds / 3600.0f + milliseconds / 3600000.0f;

    return signFactor * dd * types::degree;
}

Coordinate::Coordinate(const std::string& longitude, const std::string& latitude) :
        m_longitude(__coordinateToDecimal(longitude)),
        m_latitude(__coordinateToDecimal(latitude)) { }

bool Coordinate::operator==(const Coordinate& other) const {
    return this->m_longitude == other.m_longitude && this->m_latitude == other.m_latitude;
}

bool Coordinate::operator!=(const Coordinate& other) const {
    return false == this->operator==(other);
}

const Angle& Coordinate::longitude() const {
    return this->m_longitude;
}

Angle& Coordinate::longitude() {
    return this->m_longitude;
}

float Coordinate::longitudeDegree() const {
    return this->m_longitude.convert(types::degree);
}

void Coordinate::setLongitudeDegree(float value) {
    this->m_longitude = value * types::degree;
}

const Angle& Coordinate::latitude() const {
    return this->m_latitude;
}

Angle& Coordinate::latitude() {
    return this->m_latitude;
}

float Coordinate::latitudeDegree() const {
    return this->m_latitude.convert(types::degree);
}

void Coordinate::setLatitudeDegree(float value) {
    this->m_latitude = value * types::degree;
}

Coordinate Coordinate::projection(const Angle& heading, const Length& distance) const {
    auto proj = bg::formula::vincenty_direct<float>::apply(this->m_longitude.convert(types::radian),
                                                           this->m_latitude.convert(types::radian),
                                                           distance.convert(types::metre),
                                                           heading.convert(types::radian),
                                                           bg::srs::spheroid<float>());
    return std::move(Coordinate(proj.lon2 * types::radian, proj.lat2 * types::radian));
}

Length Coordinate::distanceTo(const Coordinate& other) const {
    return bg::distance(*this, other) * 6371.0f * types::kilometre;
}

Angle Coordinate::bearingTo(const Coordinate& other) const {
    float lon1 = this->m_longitude.convert(types::radian);
    float lon2 = other.m_longitude.convert(types::radian);
    float lat1 = this->m_latitude.convert(types::radian);
    float lat2 = other.m_latitude.convert(types::radian);

    float y = std::sin(lon2 - lon1) * std::cos(lat2);
    float x = std::cos(lat1) * std::sin(lat2) - std::sin(lat1) * std::cos(lat2) * std::cos(lon2 - lon1);
    float theta = std::atan2(y, x);

    /* ensure that angle is between [0, 360[ */
    while (0.0 > theta)
        theta += 2_pi;
    while (2_pi <= theta)
        theta -= 2_pi;

    return theta * types::radian;
}
