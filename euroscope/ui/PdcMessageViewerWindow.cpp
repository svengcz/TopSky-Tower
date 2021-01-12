/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the PDC message viewer
 */

#include "stdafx.h"

#include <management/PdcControl.h>
#include <system/ConfigurationRegistry.h>

#include "../RadarScreen.h"
#include "elements/TextViewer.h"
#include "PdcMessageViewerWindow.h"

using namespace topskytower;
using namespace topskytower::euroscope;

static __inline std::string __title(const management::PdcControl::MessagePtr& message) {
    std::string retval = message->sender + ": ";

    switch (message->type) {
    case management::PdcControl::MessageType::Telex:
        retval += "TELEX";
        break;
    case management::PdcControl::MessageType::CPDLC:
        retval += "CPDLC";
        break;
    default:
        retval += "UNKNOWN";
        break;
    }

    return retval;
}

PdcMessageViewerWindow::PdcMessageViewerWindow(RadarScreen* parent, const management::PdcControl::MessagePtr& message) :
        MessageViewerWindow(parent, __title(message), message->message) { }
