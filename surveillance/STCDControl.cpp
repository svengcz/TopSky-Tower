/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the Autonomous Runway Incursion Warning System
 */

#include <GeographicLib/Gnomonic.hpp>

#include <surveillance/STCDControl.h>

using namespace topskytower;
using namespace topskytower::surveillance;

STCDControl::STCDControl() {
    system::ConfigurationRegistry::instance().registerNotificationCallback(this, &STCDControl::reinitialize);

    this->reinitialize(system::ConfigurationRegistry::UpdateType::All);
}

STCDControl::~STCDControl() {
    system::ConfigurationRegistry::instance().deleteNotificationCallback(this);
}

void STCDControl::reinitialize(system::ConfigurationRegistry::UpdateType type) {

}
