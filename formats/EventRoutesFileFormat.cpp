/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the event routes file format
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include <fstream>

#include <formats/EventRoutesFileFormat.h>
#include <helper/String.h>
#include <types/Quantity.hpp>

using namespace topskytower;
using namespace topskytower::formats;
using namespace topskytower::types;

EventRoutesFileFormat::EventRoutesFileFormat(const std::string& filename) :
        FileFormat(),
        m_filename(filename) { }

bool EventRoutesFileFormat::mergeEvents(std::map<std::string, types::Event>& events, types::Event& event, std::size_t lineCount) {
    if (0 == event.routes.size()) {
        this->m_errorMessage = "No route defined for " + event.name;
        this->m_errorLine = static_cast<std::uint32_t>(lineCount);
        return false;
    }

    /* check if the event already exists */
    auto it = events.find(event.name);
    if (events.end() == it) {
        std::string name = event.name;
        events[name] = std::move(event);
        return true;
    }

    /* copy the routes */
    for (auto& route : event.routes)
        it->second.routes.push_back(route);

    return true;
}

bool EventRoutesFileFormat::parse(types::EventRoutesConfiguration& config) {
    std::map<std::string, types::Event> events;

    config.valid = false;

    std::ifstream stream(this->m_filename);
    if (false == stream.is_open()) {
        this->m_errorMessage = "Unable to open the events configuration file";
        this->m_errorLine = 0;
        return false;
    }

    types::Event event;
    types::Length minLvl, maxLvl;
    types::EventRoute::EvenOddRule rule = types::EventRoute::EvenOddRule::Undefined;
    std::string origin, destination;

    std::string line;
    std::uint32_t lineOffset = 0;
    while (std::getline(stream, line)) {
        lineOffset += 1;
        if (0 == line.length())
            continue;

        auto split = helper::String::splitString(line, ":");

        if (2 <= split.size() && "EVENT" == split[0]) {
            if (0 != event.name.length()) {
                if (false == this->mergeEvents(events, event, lineOffset))
                    return false;
            }

            for (std::size_t i = 1; i < split.size() - 1; ++i)
                event.name += split[i] + ":";
            event.name += split[split.size() - 1];

            if (0 == event.name.length()) {
                this->m_errorMessage = "No event name defined";
                this->m_errorLine = lineOffset;
                return false;
            }

            event.routes.clear();
            minLvl = 0_ft;
            maxLvl = 99900_ft;
            rule = types::EventRoute::EvenOddRule::Undefined;
            origin.clear();
            destination.clear();
        }
        else if (3 == split.size() && "AIRPORTS" == split[0]) {
            origin = split[1];
            destination = split[2];
        }
        else if (4 == split.size() && "LEVELS" == split[0]) {
            minLvl = static_cast<float>(std::atoi(split[1].c_str()) * 100) * types::feet;
            maxLvl = static_cast<float>(std::atoi(split[2].c_str()) * 100) * types::feet;
            if (0 != split[3].length()) {
                if ('E' == split[3][0]) {
                    rule = types::EventRoute::EvenOddRule::Even;
                }
                else if ('O' == split[3][0]) {
                    rule = types::EventRoute::EvenOddRule::Odd;
                }
                else {
                    this->m_errorMessage = "No valid even-odd rule defined";
                    this->m_errorLine = lineOffset;
                    return false;
                }
            }
        }
        else if (2 == split.size() && "ROUTE" == split[0]) {
            if (0 == origin.length() || 0 == destination.length() || 0 == event.name.length()) {
                this->m_errorMessage = "No origin, destination or event name defined for the route";
                this->m_errorLine = lineOffset;
                return false;
            }

            types::EventRoute route;
            route.origin = origin;
            route.destination = destination;
            route.route = split[1];
            route.minimumLevel = minLvl;
            route.maximumLevel = maxLvl;
            route.rule = rule;

            event.routes.push_back(route);
        }
        else {
            this->m_errorMessage = "Unknown entry: " + split[0];
            this->m_errorLine = lineOffset;
            return false;
        }
    }

    if (0 != event.name.length()) {
        if (false == this->mergeEvents(events, event, lineOffset))
            return false;
    }

    config.valid = true;
    for (auto& eventRoutes : events)
        config.events.push_back(std::move(eventRoutes.second));

    return true;
}
