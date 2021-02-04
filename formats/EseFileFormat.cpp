/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the euroscope file format
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include <helper/Exception.h>
#include <helper/String.h>
#include <formats/EseFileFormat.h>

#include "IniFileFormat.h"

using namespace topskytower;
using namespace topskytower::formats;

static __inline bool __createEdges(const std::vector<std::string>& borderDef,
                                   const std::map<std::string, std::list<types::Coordinate>>& sectorlines,
                                   types::SectorBorder& border) {
    std::list<types::Coordinate> borderEdges;
    bool resolved = true;

    /* resolve the borders */
    for (std::size_t i = 1; i < borderDef.size(); ++i) {
        auto it = sectorlines.find(borderDef[i]);
        /* unable to resolve all sector lines */
        if (sectorlines.cend() == it) {
            resolved = false;
            break;
        }

        /* insert the coordinates */
        for (std::size_t c = 0; c < it->second.size() - 1; ++c) {
            auto sit = it->second.cbegin();
            std::advance(sit, c);
            auto eit = it->second.cbegin();
            std::advance(eit, c + 1);

            if (0 == borderEdges.size()) {
                borderEdges.push_back(*sit);
                borderEdges.push_back(*eit);
            }
            else {
                auto coord0It = std::find(borderEdges.cbegin(), borderEdges.cend(), *sit);
                auto coord1It = std::find(borderEdges.cbegin(), borderEdges.cend(), *eit);

                /* avoid duplicated points */
                if (borderEdges.cend() != coord0It && borderEdges.cend() == coord1It) {
                    if (borderEdges.cbegin() == coord0It)
                        borderEdges.insert(borderEdges.begin(), *eit);
                    else
                        borderEdges.push_back(*eit);
                }
                else if (borderEdges.cend() != coord1It && borderEdges.cend() == coord0It) {
                    if (borderEdges.cbegin() == coord1It)
                        borderEdges.insert(borderEdges.begin(), *sit);
                    else
                        borderEdges.push_back(*sit);
                }
            }
        }
    }

    border.setEdges(borderEdges);

    return resolved;
}

static __inline void __parseAirspace(const std::vector<std::string>& airspace,
                                     std::map<std::string, std::list<types::SectorBorder>>& borders) {
    std::vector<std::string> sectorDef, ownerDef, borderDef;
    std::string lineIdx;

    std::list<std::pair<types::SectorBorder, std::vector<std::string>>> unresolved;
    std::map<std::string, std::list<types::Coordinate>> sectorlines;

    for (const auto& line : std::as_const(airspace)) {
        /* found a comment line */
        if (std::string::npos != line.find(';', 0))
            continue;

        auto elements = helper::String::splitString(line, ":");

        /* found a new sectorline */
        if ("SECTORLINE" == elements[0]) {
            lineIdx = elements[1];
        }
        /* found a coordinate of a defined sector line */
        else if (0 != lineIdx.length() && "COORD" == elements[0]) {
            sectorlines[lineIdx].push_back(types::Coordinate(elements[2], elements[1]));
        }
        /* found a new sector definition */
        else if ("SECTOR" == elements[0]) {
            sectorDef = elements;
            ownerDef.clear();
            borderDef.clear();
        }
        /* found the owners */
        else if ("OWNER" == elements[0]) {
            ownerDef = elements;
        }
        /* found the border */
        else if ("BORDER" == elements[0]) {
            borderDef = elements;
        }

        /* found all information for a border */
        if (0 != sectorDef.size() && 0 != ownerDef.size() && 0 != borderDef.size()) {
            types::Length lower = static_cast<float>(std::atoi(sectorDef[2].c_str())) * types::feet;
            types::Length upper = static_cast<float>(std::atoi(sectorDef[3].c_str())) * types::feet;

            /* check if we have a deputies */
            std::vector<std::string> deputies;
            std::string owner = std::move(ownerDef[1]);
            if (2 < ownerDef.size()) {
                deputies = std::move(ownerDef);
                deputies.erase(deputies.begin());
                deputies.erase(deputies.begin());
            }

            types::SectorBorder border(std::move(owner), std::move(deputies), lower, upper);

            /* resolve the borders */
            bool resolved = __createEdges(borderDef, sectorlines, border);

            /* insert the border to the output list */
            if (true == resolved)
                borders[border.owner()].push_back(std::move(border));
            else
                throw helper::Exception("ESE-Airspace", "Unable to parse border for " + border.owner());

            /* cleanup temporary variables */
            sectorDef.clear();
            ownerDef.clear();
            borderDef.clear();
        }
    }
}

void EseFileFormat::parseSectors(const std::vector<std::string>& positions, const std::vector<std::string>& airspace) {
    /* parse the sectorlines */
    std::map<std::string, std::list<types::SectorBorder>> borders;
    __parseAirspace(airspace, borders);

    /* fill the list of the sector borders */
    for (const auto& border : std::as_const(borders))
        this->m_sectorBorders.insert(this->m_sectorBorders.end(), border.second.cbegin(), border.second.cend());

    for (const auto& line : std::as_const(positions)) {
        /* found a comment line */
        if (std::string::npos != line.find(';', 0))
            continue;

        auto elements = helper::String::splitString(line, ":");

        types::Sector sector;

        /* found a position with a center */
        if (13 == elements.size())
            sector = std::move(types::Sector(std::move(elements[3]), std::move(elements[5]), std::move(elements[4]),
                                             std::move(elements[6]), std::move(elements[2]), elements[11], elements[12]));
        else
            sector = std::move(types::Sector(std::move(elements[3]), std::move(elements[5]), std::move(elements[4]),
                                             std::move(elements[6]), std::move(elements[2])));

        /* found an invalid sector */
        if (types::Sector::Type::Undefined == sector.type())
            continue;

        /* find the borders */
        auto it = borders.find(sector.controllerInfo().identifier());
        if (borders.end() != it)
            sector.setBorders(std::move(it->second));

        this->m_sectors.push_back(std::move(sector));
    }
}

void EseFileFormat::parseRunways(const std::vector<std::string>& runways) {
    for (const auto& line : std::as_const(runways)) {
        auto split = helper::String::splitString(line, " ");
        if (9 != split.size())
            continue;

        /* create the relevant information */
        types::Coordinate p0(split[5], split[4]), p1(split[7], split[6]);
        types::Length rwyLength = p0.distanceTo(p1);
        types::Runway rwy0(std::move(split[0]), p0, p1);
        types::Runway rwy1(std::move(split[1]), p1, p0);

        /* safe the runways */
        this->m_runways[split[8]].push_back(std::move(rwy0));
        this->m_runways[split[8]].push_back(std::move(rwy1));
    }
}

void EseFileFormat::parserThread(const std::string& sectorFile) {
    while (true) {
        /* analyze the next file or abort the thread */
        this->m_pathsMutex.lock();
        if (0 == this->m_paths.size()) {
            this->m_pathsMutex.unlock();
            break;
        }
        auto file = this->m_paths.front();
        this->m_paths.pop_front();
        this->m_pathsMutex.unlock();

        /* check if an other task found the correct file */
        if (true == this->m_foundSectorFile)
            break;

        /* try the file */
        IniFileFormat sctFile(file.path().string());

        /* check if the sct-file contains an INFO block */
        auto infoIt = sctFile.m_blocks.find("[INFO]");
        if (sctFile.m_blocks.cend() != infoIt) {
            /* found the correct SCT version */
            if (infoIt->second[0] == sectorFile) {
                auto esePath = file.path().filename().replace_extension(".ese");
                IniFileFormat eseFile(esePath.string());

                /* parse the sectors */
                auto positionsIt = eseFile.m_blocks.find("[POSITIONS]");
                if (eseFile.m_blocks.cend() == positionsIt)
                    throw helper::Exception("ESE File", "Unable to find positions in " + esePath.string());
                auto airspaceIt = eseFile.m_blocks.find("[AIRSPACE]");
                if (eseFile.m_blocks.cend() == airspaceIt)
                    throw helper::Exception("ESE File", "Unable to find airspaces in " + esePath.string());
                this->parseSectors(positionsIt->second, airspaceIt->second);

                /* parse the runways */
                auto runwaysIt = sctFile.m_blocks.find("[RUNWAY]");
                if (sctFile.m_blocks.cend() == runwaysIt)
                    throw helper::Exception("SCT File", "Unable to find the runways in " + file.path().string());
                this->parseRunways(runwaysIt->second);

                this->m_foundSectorFile = true;
            }
        }
    }
}

EseFileFormat::EseFileFormat() :
        m_sectors(),
        m_runways(),
        m_parserThreads(),
        m_pathsMutex(),
        m_paths(),
        m_foundSectorFile(false) { }

bool EseFileFormat::parse(const std::string& sectorName) {
    auto threadCount = std::thread::hardware_concurrency();
    if (0 == threadCount)
        threadCount = 3;

    /* find all SCT-files to anaylize the info-block */
    for (const auto& file : std::filesystem::directory_iterator(".")) {
        if (".sct" == file.path().extension())
            this->m_paths.push_back(file);
    }

    /* start the threads */
    for (std::size_t i = 0; i < threadCount; ++i)
        this->m_parserThreads.push_back(new std::thread(&EseFileFormat::parserThread, this, sectorName));

    /* wait for the threads */
    for (auto it = this->m_parserThreads.begin(); this->m_parserThreads.end() != it; ++it) {
        (*it)->join();
        delete *it;
    }

    return true == this->m_foundSectorFile;
}

const std::list<types::Sector>& EseFileFormat::sectors() const {
    return this->m_sectors;
}

const std::list<types::SectorBorder>& EseFileFormat::sectorBorders() const {
    return this->m_sectorBorders;
}

const std::list<types::Runway>& EseFileFormat::runways(const std::string& airport) const {
    static std::list<types::Runway> __fallback;
    auto it = this->m_runways.find(airport);

    if (this->m_runways.cend() != it)
        return it->second;
    else
        return __fallback;
}
