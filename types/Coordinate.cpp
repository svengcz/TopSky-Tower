/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the coordinate to abstract global coordinates
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include <algorithm>

#include <GeographicLib/Geodesic.hpp>

#include <helper/Math.h>
#include <helper/String.h>
#include <types/Coordinate.h>

using namespace topskytower;
using namespace topskytower::types;

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
        return 0.0_deg;

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
        return Angle();
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

const Angle& Coordinate::latitude() const {
    return this->m_latitude;
}

Angle& Coordinate::latitude() {
    return this->m_latitude;
}

Coordinate Coordinate::projection(const Angle& heading, const Length& distance) const {
    float lat, lon;
    GeographicLib::Geodesic::WGS84().Direct(this->latitude().convert(types::degree), this->longitude().convert(types::degree),
                                            heading.convert(types::degree), distance.convert(types::metre), lat, lon);
    return std::move(Coordinate(lon * types::degree, lat * types::degree));
}

Length Coordinate::distanceTo(const Coordinate& other) const {
    float distance;
    GeographicLib::Geodesic::WGS84().Inverse(this->latitude().convert(types::degree), this->longitude().convert(types::degree),
                                             other.latitude().convert(types::degree), other.longitude().convert(types::degree),
                                             distance);
    return distance * types::metre;
}

Angle Coordinate::bearingTo(const Coordinate& other) const {
    float azimuth0, azimuth1;
    GeographicLib::Geodesic::WGS84().Inverse(this->latitude().convert(types::degree), this->longitude().convert(types::degree),
                                             other.latitude().convert(types::degree), other.longitude().convert(types::degree),
                                             azimuth0, azimuth1);
    (void)azimuth1;

    while (0.0 > azimuth0)
        azimuth0 += 360.0f;
    while (360.0 < azimuth0)
        azimuth0 += 360.0f;

    return azimuth0 * types::degree;
}
