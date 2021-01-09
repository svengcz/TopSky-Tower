/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the PDC departure clearance window
 */

#include "stdafx.h"

#include <surveillance/PdcControl.h>
#include <system/ConfigurationRegistry.h>

#include "../RadarScreen.h"
#include "../PlugIn.h"
#include "elements/Button.h"
#include "PdcDepartureClearanceWindow.h"

using namespace topskytower;
using namespace topskytower::euroscope;

/* special implementation of the button */
class ClearanceButton : public Button {
private:
    PdcDepartureClearanceWindow* m_window;
    bool                         m_cancleButton;

    void clicked() {
        if (false == this->m_cancleButton)
            this->m_window->sendMessage();
        this->m_window->setActive(false);
    }

public:
    ClearanceButton(RadarScreen* parent, PdcDepartureClearanceWindow* window, bool cancle,
                    const Gdiplus::RectF& dimension) :
            Button(parent, (true == cancle ? "CANCEL" : "SEND"), dimension),
            m_window(window),
            m_cancleButton(cancle) { }
};

PdcDepartureClearanceWindow::PdcDepartureClearanceWindow(RadarScreen* parent,
                                                         const surveillance::PdcControl::ClearanceMessagePtr& message) :
        InsetWindow("Departure Clearance", parent, Gdiplus::RectF(0, 40, 450, 145), false),
        m_message(message),
        m_nextFrequencyField(nullptr) {
    Gdiplus::RectF dimension;
    ClearanceButton* button;
    EditText* field;

    /* add a non-changable edit-field for the callsign */
    dimension = Gdiplus::RectF(this->m_contentArea.X + 10.0f, this->m_contentArea.Y + 20.0f, 100.0f, 15.0f);
    field = new EditText(this->m_parent, "Callsign", dimension);
    field->setContent(message->receiver);
    field->setEditable(false);
    this->m_elements.push_back(field);

    /* add a non-changable edit-field for the destination */
    dimension = Gdiplus::RectF(this->m_contentArea.X + 120.0f, this->m_contentArea.Y + 20.0f, 100.0f, 15.0f);
    field = new EditText(this->m_parent, "Destination", dimension);
    field->setContent(message->destination);
    field->setEditable(false);
    this->m_elements.push_back(field);

    /* add a non-changable edit-field for the departure route */
    dimension = Gdiplus::RectF(this->m_contentArea.X + 230.0f, this->m_contentArea.Y + 20.0f, 100.0f, 15.0f);
    field = new EditText(this->m_parent, "SID", dimension);
    field->setContent(message->sid);
    field->setEditable(false);
    this->m_elements.push_back(field);

    /* add a non-changable edit-field for the clearance limit */
    dimension = Gdiplus::RectF(this->m_contentArea.X + 340.0f, this->m_contentArea.Y + 20.0f, 100.0f, 15.0f);
    field = new EditText(this->m_parent, "Init climb", dimension);
    field->setContent(message->clearanceLimit);
    field->setEditable(false);
    this->m_elements.push_back(field);

    /* add a changable edit-field for the target startup time */
    dimension = Gdiplus::RectF(this->m_contentArea.X + 10.0f, this->m_contentArea.Y + 55.0f, 100.0f, 15.0f);
    field = new EditText(this->m_parent, "TSAT", dimension);
    field->setContent(message->targetStartupTime);
    this->m_elements.push_back(field);

    /* add a changable edit-field for the calculated takeoff time */
    dimension = Gdiplus::RectF(this->m_contentArea.X + 120.0f, this->m_contentArea.Y + 55.0f, 100.0f, 15.0f);
    field = new EditText(this->m_parent, "CTOT", dimension);
    field->setContent(message->calculatedTakeOffTime);
    this->m_elements.push_back(field);

    /* add a non-changable edit-field for the next frequency */
    dimension = Gdiplus::RectF(this->m_contentArea.X + 230.0f, this->m_contentArea.Y + 55.0f, 100.0f, 15.0f);
    field = new EditText(this->m_parent, "Next freq.", dimension);
    field->setContent(message->frequency);
    field->setEditable(false);
    this->m_nextFrequencyField = field;
    this->m_elements.push_back(field);

    /* add a non-changable edit-field for the squawk */
    dimension = Gdiplus::RectF(this->m_contentArea.X + 340.0f, this->m_contentArea.Y + 55.0f, 100.0f, 15.0f);
    field = new EditText(this->m_parent, "Squawk", dimension);
    field->setContent(message->squawk);
    field->setEditable(false);
    this->m_elements.push_back(field);

    /* add a changable edit-field for the custom message */
    dimension = Gdiplus::RectF(this->m_contentArea.X + 10.0f, this->m_contentArea.Y + 90.0f, 430.0f, 15.0f);
    field = new EditText(this->m_parent, "Custom message", dimension);
    field->setContent(message->message);
    this->m_elements.push_back(field);

    /* add a button for cancle */
    dimension = Gdiplus::RectF(this->m_contentArea.X + 290.0f, this->m_contentArea.Y + 115.f, 70.0f, 20.0f);
    button = new ClearanceButton(this->m_parent, this, true, dimension);
    this->m_elements.push_back(button);

    /* add a button for send */
    dimension = Gdiplus::RectF(this->m_contentArea.X + 370.0f, this->m_contentArea.Y + 115.f, 70.0f, 20.0f);
    button = new ClearanceButton(this->m_parent, this, false, dimension);
    this->m_elements.push_back(button);
}

PdcDepartureClearanceWindow::~PdcDepartureClearanceWindow() {
    for (auto it = this->m_elements.begin(); this->m_elements.end() != it; ++it)
        delete* it;
    this->m_elements.clear();
}

void PdcDepartureClearanceWindow::setActive(bool active) {
    InsetWindow::setActive(active);

    if (false == active) {
        this->m_parent->uiManager().removeCustomWindow(this);
        delete this;
    }
    else {
        this->centeredPosition();
        this->m_parent->uiManager().addCustomWindow(this);
    }
}

bool PdcDepartureClearanceWindow::click(const Gdiplus::PointF& pt, UiManager::MouseButton button) {
    /* we are only interested in left clicks */
    if (UiManager::MouseButton::Left != button)
        return InsetWindow::click(pt, button);

    /* check if the user clicked on the frequency */
    if (true == UiElement::isInRectangle(pt, this->m_nextFrequencyField->area())) {
        POINT point = { static_cast<int>(pt.X), static_cast<int>(pt.Y) };
        RECT area = {
            static_cast<int>(this->m_nextFrequencyField->area().GetLeft()),
            static_cast<int>(this->m_nextFrequencyField->area().GetTop()),
            static_cast<int>(this->m_nextFrequencyField->area().GetRight()),
            static_cast<int>(this->m_nextFrequencyField->area().GetBottom())
        };

        RadarScreen::EuroscopeEvent esEvent = {
            static_cast<int>(PlugIn::TagItemFunction::HandoffSectorChangeEvent),
            this->m_message->receiver,
            "",
            point,
            area
        };
        this->m_parent->registerEuroscopeEvent(std::move(esEvent));

        return true;
    }
    /* forward all other clicks */
    else {
        return InsetWindow::click(pt, button);
    }
}

void PdcDepartureClearanceWindow::centeredPosition() {
    auto oldWindowPos = Gdiplus::PointF(this->m_contentArea.X, this->m_contentArea.Y);

    /* place the window in the center of the screen */
    auto width = this->m_parent->GetRadarArea().right - this->m_parent->GetRadarArea().left;
    auto height = this->m_parent->GetRadarArea().bottom - this->m_parent->GetRadarArea().top;
    InsetWindow::setPosition(Gdiplus::PointF(static_cast<float>(width) * 0.5f - this->m_area.Width * 0.5f,
                             static_cast<float>(height) * 0.5f - this->m_area.Height * 0.5f));

    for (auto& element : this->m_elements) {
        auto oldPos = Gdiplus::PointF(element->area().X, element->area().Y);
        element->setPosition(Gdiplus::PointF(oldPos.X - oldWindowPos.X + this->m_contentArea.X,
                                             oldPos.Y - oldWindowPos.Y + this->m_contentArea.Y));
    }
}

void PdcDepartureClearanceWindow::sendMessage() {
    auto it = this->m_elements.cbegin();

    std::advance(it, 4);
    this->m_message->targetStartupTime = static_cast<const EditText*>(*it)->content();

    std::advance(it, 1);
    this->m_message->calculatedTakeOffTime = static_cast<const EditText*>(*it)->content();

    std::advance(it, 3);
    this->m_message->message = static_cast<const EditText*>(*it)->content();

    this->m_message->frequency = this->m_nextFrequencyField->content();

    surveillance::PdcControl::instance().sendClearanceMessage(this->m_message);
}

bool PdcDepartureClearanceWindow::visualize(Gdiplus::Graphics* graphics) {
    if (nullptr != this->m_nextFrequencyField) {
        /* controller select an other sector */
        if (true == this->m_parent->sectorControl().handoffRequired(this->m_message->receiver)) {
            auto& info = this->m_parent->sectorControl().handoffSector(this->m_message->receiver);
            this->m_nextFrequencyField->setContent(info.primaryFrequency());
        }
    }

    return InsetWindow::visualize(graphics);
}
