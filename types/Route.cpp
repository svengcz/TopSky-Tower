/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the route
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include <types/Route.h>

using namespace topskytower;
using namespace topskytower::types;

Route::Route() :
        m_waypoints() { }

Route::Route(std::vector<Waypoint>&& waypoints) :
        m_waypoints(std::move(waypoints)) { }

const std::vector<Waypoint>& Route::waypoints() const {
    return this->m_waypoints;
}
