/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the PDC logon window of the system
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include "stdafx.h"

#include <management/PdcControl.h>
#include <system/ConfigurationRegistry.h>

#include "../RadarScreen.h"
#include "elements/Button.h"
#include "elements/EditText.h"
#include "HoppiesLogonWindow.h"

using namespace topskytower;
using namespace topskytower::euroscope;

/* special implementation of the button */
class LogonButton : public Button {
private:
    HoppiesLogonWindow* m_window;

    void clicked() {
        if (0 == this->m_window->station().length())
            return;
        if (0 == this->m_window->code().length())
            return;

        management::PdcControl::instance().addAirport(this->m_window->station());
        this->m_window->setActive(false);
    }

public:
    LogonButton(RadarScreen* parent, HoppiesLogonWindow* window, const Gdiplus::RectF& dimension) :
            Button(parent, "LOGON", dimension),
            m_window(window) { }
};

HoppiesLogonWindow::HoppiesLogonWindow(RadarScreen* parent) :
        InsetWindow("PDC", parent, Gdiplus::RectF(0, 40, 200, 105), false, true) {
    this->centeredPosition();

    /* add an edit text for the station */
    Gdiplus::RectF dimension = Gdiplus::RectF(this->m_contentArea.X + 2.0f, this->m_contentArea.Y + 15.0f,
                                              this->m_contentArea.Width - 4.0f, 23.0f);
    this->m_elements.push_back(new EditText(this->m_parent, "Station", dimension));
    static_cast<EditText*>(this->m_elements.back())->setContent(this->m_parent->airportIcao());

    /* add an edit text for the password */
    dimension = Gdiplus::RectF(this->m_contentArea.X + 2.0f, this->m_contentArea.Y + 55.0f, this->m_contentArea.Width - 4.0f, 23.0f);
    this->m_elements.push_back(new EditText(this->m_parent, "Password", dimension));
    static_cast<EditText*>(this->m_elements.back())->setPasswordField(true);
    const auto& hoppies = system::ConfigurationRegistry::instance().systemConfiguration().hoppiesCode;
    static_cast<EditText*>(this->m_elements.back())->setContent(hoppies);

    /* create the elements */
    dimension = Gdiplus::RectF(this->m_contentArea.X + 145.0f, this->m_contentArea.Y + 83.0f, 53.0f, 21.0f);
    this->m_elements.push_back(new LogonButton(parent, this, dimension));
}

void HoppiesLogonWindow::setActive(bool active) {
    if (true == active) {
        if (true == management::PdcControl::instance().airportOnline(this->station())) {
            management::PdcControl::instance().removeAirport(this->station());
            return;
        }
    }

    if (false == active)
        this->centeredPosition();

    InsetWindow::setActive(active);
}

void HoppiesLogonWindow::centeredPosition() {
    /* place the window in the center of the screen */
    auto width = this->m_parent->GetRadarArea().right - this->m_parent->GetRadarArea().left;
    auto height = this->m_parent->GetRadarArea().bottom - this->m_parent->GetRadarArea().top;
    InsetWindow::setPosition(Gdiplus::PointF(static_cast<float>(width) * 0.5f - 100.0f, static_cast<float>(height) * 0.5f - 50.0f));

    /* update the positions of the elements */
    if (0 != this->m_elements.size()) {
        auto it = this->m_elements.begin();

        /* set the station's EditText */
        (*it)->setPosition(Gdiplus::PointF(this->m_contentArea.X + 2.0f, this->m_contentArea.Y + 15.0f));

        /* set the password's EditText */
        std::advance(it, 1);
        (*it)->setPosition(Gdiplus::PointF(this->m_contentArea.X + 2.0f, this->m_contentArea.Y + 55.0f));

        /* set the logon button */
        std::advance(it, 1);
        (*it)->setPosition(Gdiplus::PointF(this->m_contentArea.X + 145.0f, this->m_contentArea.Y + 83.0f));
    }
}

const std::string& HoppiesLogonWindow::station() const {
    auto it = this->m_elements.cbegin();
    return static_cast<const EditText*>(*it)->content();
}

const std::string& HoppiesLogonWindow::code() const {
    auto it = this->m_elements.cbegin();
    std::advance(it, 1);
    return static_cast<const EditText*>(*it)->content();
}
