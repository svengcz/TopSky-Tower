/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the NOTAM control
 */

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <sstream>

#define CURL_STATICLIB 1
#include <curl/curl.h>

#include <helper/String.h>
#include <management/NotamControl.h>
#include <system/ConfigurationRegistry.h>

using namespace std::chrono;
using namespace topskytower;
using namespace topskytower::management;

NotamControl::NotamControl() { }

void NotamControl::run() { }

NotamControl& NotamControl::instance() {
    static NotamControl __instance;
    return __instance;
}
