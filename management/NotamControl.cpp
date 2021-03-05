/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the NOTAM control
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#define _CRT_SECURE_NO_WARNINGS 1
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <sstream>

#define CURL_STATICLIB 1
#include <curl/curl.h>

#include <helper/String.h>
#include <helper/Time.h>
#include <management/NotamControl.h>
#include <system/ConfigurationRegistry.h>
#include <types/FlightPlan.h>

#include "grammar/Notam.hpp"
#include "grammar/Parser.hpp"
#include "grammar/Runway.hpp"
#include "grammar/Stand.hpp"

using namespace std::chrono;
using namespace topskytower;
using namespace topskytower::management;

static std::string __receivedData;

NotamControl::NotamControl() :
        m_stopNotamThread(false),
        m_airportUpdates(),
        m_pendingQueueLock(),
        m_enqueuePending(),
        m_dequeuePending(),
        m_notams(),
        m_notificationCallbacks(),
        m_notamThread(&NotamControl::run, this) { }

NotamControl::~NotamControl() {
    this->m_stopNotamThread = true;
    this->m_notamThread.join();
}

static std::size_t receiveCurl(void* ptr, std::size_t size, std::size_t nmemb, void* stream) {
    (void)stream;

    std::string serverResult = static_cast<char*>(ptr);
    __receivedData += serverResult;
    return size * nmemb;
}

NotamCategory NotamControl::parseQCode(const std::string& qCode) {
    /* found an invalid code */
    if (5 != qCode.length() && 'Q' != qCode[0])
        return NotamCategory::Unknown;

    /* found a non-airport relevant NOTAM */
    if ('M' != qCode[1])
        return NotamCategory::Other;

    switch (qCode[2]) {
    case 'A':
        return NotamCategory::MovementArea;
    case 'B':
        return NotamCategory::BearingStrength;
    case 'C':
        return NotamCategory::Clearway;
    case 'D':
        return NotamCategory::DeclaredDistances;
    case 'G':
        return NotamCategory::TaxiGuidance;
    case 'H':
        return NotamCategory::RunwayArrestingGear;
    case 'K':
        return NotamCategory::Parking;
    case 'M':
        return NotamCategory::DaylightMarkings;
    case 'N':
        return NotamCategory::Apron;
    case 'O':
        return NotamCategory::Stopbar;
    case 'P':
        return NotamCategory::Stands;
    case 'R':
        return NotamCategory::Runway;
    case 'S':
        return NotamCategory::Stopbar;
    case 'T':
        return NotamCategory::Threshold;
    case 'U':
        return NotamCategory::RunwayTurningBay;
    case 'W':
        return NotamCategory::Strip;
    case 'X':
        return NotamCategory::Taxiway;
    case 'Y':
        return NotamCategory::RapidExit;
    default:
        return NotamCategory::Unknown;
    }
}

std::shared_ptr<Notam> NotamControl::createNotamStructure(const std::string& qCode, const std::string& content) {
    auto category = NotamControl::parseQCode(qCode);
    if (NotamCategory::Unknown == category)
        return nullptr;

    std::shared_ptr<Notam> retval;
    grammar::AstNode node;

    /* check NOTAMs that close elements */
    if ('L' == qCode[3] && 'C' == qCode[4]) {
        switch (category) {
        case NotamCategory::Runway:
            retval = std::shared_ptr<Notam>(new RunwayNotam());
            if (true == grammar::Parser::parse(content, grammar::RunwayGrammar(), node)) {
                retval->activationState = NotamActiveState::Automatic;
                retval->interpreterState = NotamInterpreterState::Success;
                const auto& runways = boost::get<grammar::AstRunway>(node);
                static_cast<RunwayNotam*>(retval.get())->sections = std::move(runways.names);
            }
            else {
                retval->interpreterState = NotamInterpreterState::Failed;
            }
            break;
        case NotamCategory::Stands:
            retval = std::shared_ptr<Notam>(new StandNotam());
            if (true == grammar::Parser::parse(content, grammar::StandGrammar(), node)) {
                retval->activationState = NotamActiveState::Automatic;
                retval->interpreterState = NotamInterpreterState::Success;
                const auto& stands = boost::get<grammar::AstStand>(node);
                static_cast<RunwayNotam*>(retval.get())->sections = std::move(stands.stands);
            }
            else {
                retval->interpreterState = NotamInterpreterState::Failed;
            }
            break;
        default:
            retval = std::shared_ptr<Notam>(new Notam());
            retval->interpreterState = NotamInterpreterState::Ignored;
            break;
        }
    }
    else {
        retval = std::shared_ptr<Notam>(new Notam());
        retval->interpreterState = NotamInterpreterState::Ignored;
    }

    retval->category = category;
    return std::move(retval);
}

bool NotamControl::createNotam(const std::string& notamText, std::shared_ptr<Notam>& notam) {
    grammar::AstNode node;
    bool retval;

    retval = grammar::Parser::parse(notamText, grammar::NotamGrammar(), node);
    if (true == retval) {
        const auto& notamTree = boost::get<grammar::AstNotam>(node);

        notam = NotamControl::createNotamStructure(notamTree.info.code, notamTree.content);
        if (nullptr == notam)
            return false;

        /* translate the generic topics of the NOTAM */
        notam->title = notamTree.title;
        notam->category = NotamControl::parseQCode(notamTree.info.code);
        notam->startTime = notamTree.startTime;
        notam->endTime = notamTree.endTime;
        notam->message = notamTree.content;

        /* translate the NOTAM information */
        notam->information.fir = notamTree.info.fir;
        notam->information.code = notamTree.info.code;
        notam->information.flightRule = 0;
        if (std::string::npos != notamTree.info.flightRule.find("I"))
            notam->information.flightRule |= static_cast<std::uint8_t>(types::FlightPlan::Type::IFR);
        if (std::string::npos != notamTree.info.flightRule.find("V"))
            notam->information.flightRule |= static_cast<std::uint8_t>(types::FlightPlan::Type::VFR);
        notam->information.purpose = notamTree.info.purpose;
        notam->information.scope = notamTree.info.scope;
        notam->information.lowerAltitude = notamTree.info.lowerAltitude;
        notam->information.upperAltitude = notamTree.info.upperAltitude;
        notam->information.coordinate = notamTree.info.coordinate;
        notam->information.radius = notamTree.info.radius;
        notam->activeDueTime = NotamControl::activeDueTime(notam);

        notam->rawMessage = notamText;
    }
    else {
        return false;
    }

    return retval;
}

bool NotamControl::parseNotams(const std::string& airport) {
    std::istringstream stream(__receivedData);
    int foundMarker = 0; /* 0 - no marker found, 1 - start marker found, 2 - end marker found */
    std::string line, notam;

    this->m_notams[airport].clear();

    while (std::getline(stream, line)) {
        std::size_t startPos = line.find(system::ConfigurationRegistry::instance().systemConfiguration().notamMarkerStart);
        std::size_t endPos = line.find(system::ConfigurationRegistry::instance().systemConfiguration().notamMarkerEnd);

        if (std::string::npos != startPos) {
            if (2 == foundMarker) {
                std::shared_ptr<Notam> notamPtr;
                if (true == NotamControl::createNotam(notam, notamPtr))
                    this->m_notams[airport].push_back(notamPtr);
            }
            notam.clear();

            line.erase(0, startPos + system::ConfigurationRegistry::instance().systemConfiguration().notamMarkerStart.length());
            if (0 != line.length())
                notam += line + "\n";
            foundMarker = 1;
        }
        else if (1 == foundMarker && std::string::npos != endPos) {
            line.erase(endPos, line.length());
            if (0 != line.length())
                notam += line + "\n";
            foundMarker = 2;
        }
        else if (1 == foundMarker) {
            notam += line + "\n";
        }
    }

    return 0 != this->m_notams[airport].size();
}

bool NotamControl::receiveNotams(const std::string& airport) {
    auto message = system::ConfigurationRegistry::instance().systemConfiguration().notamUrl;
    if (0 == message.length())
        return false;
    helper::String::stringReplace(message, "%AIRPORT%", airport);

    __receivedData.clear();

    CURLcode result = CURLE_CONV_FAILED;
    CURL* curl;

    curl = curl_easy_init();
    if (nullptr != curl) {
        /* configure the connection */
        curl_easy_setopt(curl, CURLOPT_URL, message.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, receiveCurl);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

        /* send the command */
        result = curl_easy_perform(curl);

        /* handle the return code of the receive-function */
        if (CURLE_OK != result)
            __receivedData.clear();
        curl_easy_cleanup(curl);
    }

    return CURLE_OK == result && true == this->parseNotams(airport);
}

bool NotamControl::activeDueTime(const std::shared_ptr<Notam>& notam) {
    auto now = helper::Time::currentUtc();
    return notam->startTime <= now && notam->endTime > now;
}

void NotamControl::updateActiveDueTime() {
    bool updated = false;

    for (const auto& airport : std::as_const(this->m_notams)) {
        for (auto& notam : airport.second) {
            if (NotamActiveState::Automatic == notam->activationState) {
                auto newState = this->activeDueTime(notam);
                updated |= newState != notam->activeDueTime;
                notam->activeDueTime = newState;
            }
        }
    }

    if (true == updated)
        this->notamActivationChanged();
}

void NotamControl::run() {
    std::size_t counter = 0;

    while (false == this->m_stopNotamThread) {
        bool updated = false;

        this->m_pendingQueueLock.lock();

        updated |= 0 != this->m_dequeuePending.size();

        /* check if we have to delete airports */
        for (const auto& dequeue : this->m_dequeuePending) {
            auto it = this->m_airportUpdates.find(dequeue);
            if (this->m_airportUpdates.end() != it)
                this->m_airportUpdates.erase(it);

            auto nit = this->m_notams.find(dequeue);
            if (this->m_notams.end() != nit)
                this->m_notams.erase(nit);
        }
        this->m_dequeuePending.clear();

        /* check if we have to add a new airport */
        for (const auto& enqueue : this->m_enqueuePending) {
            auto it = this->m_airportUpdates.find(enqueue);
            if (this->m_airportUpdates.end() == it)
                this->m_airportUpdates[enqueue] = NotamTimePoint();
        }
        this->m_enqueuePending.clear();

        this->m_pendingQueueLock.unlock();

        /* check if a new update is required */
        auto now = std::chrono::system_clock::now();

        for (auto it = this->m_airportUpdates.begin(); this->m_airportUpdates.end() != it; ++it) {
            /* perform the update step */
            if (60 <= std::chrono::duration_cast<std::chrono::minutes>(now - it->second).count()) {
                if (true == this->receiveNotams(it->first)) {
                    it->second = now;
                    updated = true;
                }
            }
        }

        if (true == updated)
            this->notamActivationChanged();

        std::this_thread::sleep_for(1s);

        /* check if all automated NOTAM activations are still stable */
        if (10 <= counter++) {
            this->updateActiveDueTime();
            counter = 0;
        }
    }
}

void NotamControl::addAirport(const std::string& airport) {
    std::lock_guard guard(this->m_pendingQueueLock);
    this->m_enqueuePending.push_back(airport);
}

void NotamControl::removeAirport(const std::string& airport) {
    std::lock_guard guard(this->m_pendingQueueLock);
    this->m_dequeuePending.push_back(airport);
}

const std::map<std::string, std::list<std::shared_ptr<Notam>>>& NotamControl::notams() const {
    return this->m_notams;
}

std::list<std::shared_ptr<Notam>> NotamControl::notams(const std::string& airport, NotamCategory category) {
    std::list<std::shared_ptr<Notam>> retval;
    auto it = this->m_notams.find(airport);

    if (this->m_notams.cend() != it) {
        for (const auto& notam : it->second) {
            if (NotamCategory::Unknown == category || notam->category == category)
                retval.push_back(notam);
        }
    }

    return retval;
}

void NotamControl::notamActivationChanged() {
    for (auto& callback : this->m_notificationCallbacks)
        callback.second();
}

NotamControl& NotamControl::instance() {
    static NotamControl __instance;
    return __instance;
}
