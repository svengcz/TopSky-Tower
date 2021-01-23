/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the Autonomous Runway Incursion Warning System
 */

#include <GeographicLib/Gnomonic.hpp>

#include <surveillance/STCDControl.h>
#include <system/ConfigurationRegistry.h>

using namespace topskytower;
using namespace topskytower::surveillance;

STCDControl::STCDControl() { }
