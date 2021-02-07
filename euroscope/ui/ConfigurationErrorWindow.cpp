/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the configuration error viewer
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include "stdafx.h"

#include <system/ConfigurationRegistry.h>

#include "../RadarScreen.h"
#include "ConfigurationErrorWindow.h"
#include "elements/TextViewer.h"

using namespace topskytower::euroscope;
using namespace topskytower::system;

ConfigurationErrorWindow::ConfigurationErrorWindow(RadarScreen* parent) :
        MessageViewerWindow(parent, "Configuration", "") {
    auto viewer = static_cast<TextViewer*>(this->m_elements.front());

    std::string message;
    if (false == ConfigurationRegistry::instance().errorFound()) {
        message = "No error found!";
    }
    else {
        message = ConfigurationRegistry::instance().errorMessage();
        if (0 != ConfigurationRegistry::instance().errorLine())
            message += "\nLine: " + std::to_string(ConfigurationRegistry::instance().errorLine());
    }

    viewer->setText(message);
}
