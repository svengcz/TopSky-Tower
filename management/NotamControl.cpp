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

using namespace std::chrono;
using namespace topskytower;
using namespace topskytower::management;

static std::string __receivedData;

NotamControl::NotamControl() :
        m_stopNotamThread(false),
        m_notamThread(&NotamControl::run, this),
        m_airportUpdates(),
        m_pendingQueueLock(),
        m_enqueuePending(),
        m_dequeuePending(),
        m_notams() { }

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

bool NotamControl::createNotam(const std::string& notamText, NotamControl::Notam& notam) {
    std::istringstream stream(notamText);
    bool content = false;
    std::string line;

    while (std::getline(stream, line)) {
        if (0 == notam.title.length())
            notam.title = line;

        /* found the time-entries */
        if (std::string::npos != line.find("B)") || std::string::npos != line.find("C)")) {
            auto split = helper::String::splitString(line, " ");

            std::size_t startMarker = split.size();
            std::size_t endMarker = split.size();

            for (std::size_t i = 0; i < split.size(); ++i) {
                if ("B)" == split[i] && (i + 1) < split.size())
                    startMarker = i + 1;
                else if ("C)" == split[i] && (i + 1) < split.size())
                    endMarker = i + 1;
            }

            if (split.size() <= startMarker && split.size() <= endMarker)
                return false;

            if (split.size() > startMarker)
                notam.startTime = helper::Time::stringToTime(split[startMarker]);
            if (split.size() > endMarker) {
                if ("PERM" == split[endMarker])
                    notam.endTime = helper::Time::stringToTime("6812312359"); /* 31.12.2068 23:59 */
                else
                    notam.endTime = helper::Time::stringToTime(split[endMarker]);
            }
        }
        else if (std::string::npos != line.find("E)")) {
            line.erase(0, line.find("E)") + 3);
            if (0 != line.length())
                notam.message = line + "\n";
            content = true;
        }
        else if (true == content) {
            notam.message += line + "\n";
        }
    }

    notam.rawMessage = notamText;

    return true == content && 0 != notam.title.length();
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
                this->m_notams[airport].push_back(NotamControl::Notam());
                if (false == NotamControl::createNotam(notam, this->m_notams[airport].back()))
                    this->m_notams[airport].pop_back();
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

void NotamControl::run() {
    while (false == this->m_stopNotamThread) {
        this->m_pendingQueueLock.lock();

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
                this->m_airportUpdates[enqueue] = TimePoint();
        }
        this->m_enqueuePending.clear();

        this->m_pendingQueueLock.unlock();

        /* check if a new update is required */
        auto now = std::chrono::system_clock::now();

        for (auto it = this->m_airportUpdates.begin(); this->m_airportUpdates.end() != it; ++it) {
            /* perform the update step */
            if (60 <= std::chrono::duration_cast<std::chrono::minutes>(now - it->second).count()) {
                if (true == this->receiveNotams(it->first))
                    it->second = now;
            }
        }

        std::this_thread::sleep_for(1s);
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

const std::map<std::string, std::list<NotamControl::Notam>>& NotamControl::notams() const {
    return this->m_notams;
}

NotamControl& NotamControl::instance() {
    static NotamControl __instance;
    return __instance;
}