/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the version checker
 */

#include "stdafx.h"

#define CURL_STATICLIB 1
#include <curl/curl.h>

#include <helper/String.h>
#include <system/ConfigurationRegistry.h>

#include "ui/MessageViewerWindow.h"
#include "version.h"
#include "VersionChecker.h"

using namespace topskytower::euroscope;

static std::string __receivedVersion;

static std::size_t receiveCurl(void* ptr, std::size_t size, std::size_t nmemb, void* stream) {
    (void)stream;

    std::string serverResult = static_cast<char*>(ptr);
    __receivedVersion += serverResult;
    return size * nmemb;
}

void VersionChecker::checkForUpdates(RadarScreen* screen) {
    CURLcode result;
    CURL* curl;

    curl = curl_easy_init();
    if (nullptr != curl) {
        /* configure the connection */
        curl_easy_setopt(curl, CURLOPT_URL, system::ConfigurationRegistry::instance().systemConfiguration().versionCheckUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, receiveCurl);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2L);

        /* request the NOTAMs */
        result = curl_easy_perform(curl);

        /* handle the return code of the receive-function */
        if (CURLE_OK != result) {
            /* show an error message */
            auto window = new MessageViewerWindow(screen, "TopSky-Tower", "Unable to check for updates");
            window->setActive(true);
            return;
        }
        curl_easy_cleanup(curl);
    }

    /* get the elements of the version */
    auto split = helper::String::splitString(__receivedVersion, ".");
    if (3 != split.size()) {
        auto window = new MessageViewerWindow(screen, "TopSky-Tower", "Received an invalid version");
        window->setActive(true);
        return;
    }

    /* create the version hashs */
    auto receivedHash = VersionChecker::versionHash(static_cast<std::uint8_t>(std::atoi(split[0].c_str())),
                                                    static_cast<std::uint8_t>(std::atoi(split[1].c_str())),
                                                    static_cast<std::uint8_t>(std::atoi(split[2].c_str())));
    auto currentHash = VersionChecker::versionHash(PLUGIN_MAJOR_VERSION, PLUGIN_MINOR_VERSION, PLUGIN_PATCH_VERSION);

    /* compare the versions */
    if (currentHash < receivedHash) {
        std::string message = "An update is available\n";
        message += "Current version: " + std::string(PLUGIN_VERSION) + "\n";
        message += "Available version: " + __receivedVersion;

        auto window = new MessageViewerWindow(screen, "TopSky-Tower", message);
        window->setActive(true);
        return;
    }
}

std::uint32_t VersionChecker::versionHash(std::uint8_t major, std::uint8_t minor, std::uint8_t patch) {
    auto retval = static_cast<std::uint32_t>(major << 16);
    retval |= static_cast<std::uint32_t>(minor << 8);
    retval |= static_cast<std::uint32_t>(patch);
    return retval;
}
