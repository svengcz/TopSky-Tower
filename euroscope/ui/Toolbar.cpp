/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the toolbar of the system
 */

#include "stdafx.h"

#include <system/ConfigurationRegistry.h>

#include "../PlugIn.h"
#include "../RadarScreen.h"
#include "Toolbar.h"
#include "UiManager.h"

using namespace topskytower;
using namespace topskytower::euroscope;

Toolbar::Level::Level() :
        active(false),
        elements(),
        rectangle() { }

Toolbar::Toolbar(RadarScreen* parent, UiManager* manager) :
        UiElement(parent, Gdiplus::RectF()),
        m_toplevel(nullptr),
        m_manager(manager) {
}

void Toolbar::createElement(const std::string& text, Toolbar::ClickId uid, std::shared_ptr<Toolbar::Level>& parent) {
    parent->elements.push_back(Toolbar::Element());

    auto color = UiElement::foregroundColor();

    /* set the entry */
    parent->elements.back().visualization.setFontColor(color);
    parent->elements.back().visualization.setBold(true);
    parent->elements.back().clickId = uid;
    parent->elements.back().text = text;
}

Toolbar::ClickId Toolbar::findClickedElement(const std::shared_ptr<Toolbar::Level>& level, const Gdiplus::PointF& pt) {
    /* an inactive element cannot be clicked */
    if (nullptr != level && false == level->active)
        return Toolbar::ClickId::Undefined;

    /* found the clicked element */
    if (true == UiElement::isInRectangle(pt, level->rectangle)) {
        Toolbar::ClickId retval = Toolbar::ClickId::Group;

        /* deactivate all children of the elements and test all elements */
        for (auto& element : level->elements) {
            /* deactivate the child */
            if (nullptr != element.child)
                element.child->active = false;

            /* found the clicked element */
            if (true == element.visualization.isInBox(pt)) {
                /* activate the child, if it exists */
                if (nullptr != element.child)
                    element.child->active = true;
                retval = element.clickId;
            }
        }

        return retval;
    }
    else {
        /* test all children of the elements */
        for (auto& element : level->elements) {
            if (nullptr != element.child) {
                auto retval = Toolbar::findClickedElement(element.child, pt);
                /* found a child or element */
                if (Toolbar::ClickId::Undefined != retval)
                    return retval;
            }
        }
    }

    /* nothing found */
    return Toolbar::ClickId::Undefined;
}

void Toolbar::deactivate(std::shared_ptr<Toolbar::Level>& level) {
    /* no more recursive calls needed */
    if (nullptr == level || false == level->active)
        return;

    /* deactivate the children */
    level->active = false;
    for (auto& element : level->elements)
        Toolbar::deactivate(element.child);
}

bool Toolbar::click(const Gdiplus::PointF& pt, UiManager::MouseButton button) {
    /* handle only left mouse button clicks */
    if (UiManager::MouseButton::Left != button) {
        this->resetClickStates();
        return false;
    }

    bool resetUi = true, retval = true;

    switch (Toolbar::findClickedElement(this->m_toplevel, pt)) {
    case Toolbar::ClickId::Settings:
    case Toolbar::ClickId::Systems:
        resetUi = false;
        break;
    case Toolbar::ClickId::Reload:
        system::ConfigurationRegistry::instance().configure(static_cast<PlugIn*>(this->m_parent->GetPlugIn())->settingsPath(),
                                                            system::ConfigurationRegistry::UpdateType::All);
        break;
    case Toolbar::ClickId::ReloadSystem:
        system::ConfigurationRegistry::instance().configure(static_cast<PlugIn*>(this->m_parent->GetPlugIn())->settingsPath(),
                                                            system::ConfigurationRegistry::UpdateType::System);
        break;
    case Toolbar::ClickId::ReloadAirports:
        system::ConfigurationRegistry::instance().configure(static_cast<PlugIn*>(this->m_parent->GetPlugIn())->settingsPath(),
                                                            system::ConfigurationRegistry::UpdateType::Airports);
        break;
    case Toolbar::ClickId::ReloadAircrafts:
        system::ConfigurationRegistry::instance().configure(static_cast<PlugIn*>(this->m_parent->GetPlugIn())->settingsPath(),
                                                            system::ConfigurationRegistry::UpdateType::Aircrafts);
        break;
    case Toolbar::ClickId::PDC:
        this->m_manager->activateUi(UiManager::WindowId::PdcLogon);
        break;
    case Toolbar::ClickId::Group:
        break;
    case Toolbar::ClickId::Undefined:
    default:
        retval = false;
        break;
    }

    /* reset the UI */
    if (true == resetUi)
        this->resetClickStates();

    return retval;
}

void Toolbar::resetClickStates() {
    for (auto& element : this->m_toplevel->elements)
        Toolbar::deactivate(element.child);
}

void Toolbar::drawLevel(Gdiplus::Graphics* graphics, std::shared_ptr<Toolbar::Level>& level, int startX, int startY, bool horizontal) {
    float maxHeight = 0.0f, maxWidth = 0.0f, offsetX, offsetY;

    /* do not visualize it */
    if (nullptr == level || false == level->active)
        return;

    /* prepare the texts */
    for (auto& element : level->elements) {
        element.visualization.setGraphics(graphics);
        element.visualization.setText(element.text);
        maxHeight = max(element.visualization.rectangle().Height, maxHeight);
        maxWidth = max(element.visualization.rectangle().Width, maxWidth);
    }
    int rowHeight = static_cast<int>(maxHeight + 0.5f) + 4;
    int columnWidth = static_cast<int>(maxWidth + 0.5f) + 20;

    /* calculate the rectangle and the offsets for the positions */
    if (true == horizontal) {
        auto area = this->m_parent->GetRadarArea();
        level->rectangle.Width = static_cast<float>(area.right - area.left);
        level->rectangle.Height = static_cast<float>(rowHeight);
        offsetX = static_cast<float>(columnWidth);
        offsetY = 0.0f;
    }
    else {
        level->rectangle.Width = static_cast<float>(columnWidth);
        level->rectangle.Height = static_cast<float>(rowHeight * level->elements.size());
        offsetX = 0.0f;
        offsetY = static_cast<float>(rowHeight);
    }
    level->rectangle.X = static_cast<float>(startX);
    level->rectangle.Y = static_cast<float>(startY);

    /* draw the background of the elements */
    auto color = UiElement::backgroundColor();
    Gdiplus::SolidBrush brush(color);
    Gdiplus::Rect rectangle(startX, startY, static_cast<int>(level->rectangle.Width), static_cast<int>(level->rectangle.Height));
    graphics->FillRectangle(&brush, rectangle);

    /* register the rectangle */
    RECT rect = { rectangle.GetLeft(), rectangle.GetTop(), rectangle.GetRight(), rectangle.GetBottom() };
    this->m_parent->AddScreenObject(static_cast<int>(RadarScreen::ClickId::UserWindow), "Toolbar", rect, false, "");

    /* draw the elements */
    Gdiplus::PointF position(static_cast<float>(startX) + 10.0f, static_cast<float>(startY) + (false == horizontal ? 2.0f : 3.0f));
    for (auto& element : level->elements) {
        element.visualization.setPosition(position);
        element.visualization.visualize();

        if (false == horizontal) {
            this->drawLevel(graphics, element.child,
                            static_cast<int>(element.rectangle.GetRight() + 0.5f) + 10,
                            static_cast<int>(element.rectangle.GetTop()) - 2, false);
        }

        position.X += offsetX;
        position.Y += offsetY;
    }
}

void Toolbar::draw(Gdiplus::Graphics* graphics, std::shared_ptr<Level>& level, int startX, int startY, bool horizontal) {
    /* not needed to draw the group */
    if (false == level->active)
        return;

    /* draw the level */
    this->drawLevel(graphics, level, startX, startY, horizontal);

    /* check the children */
    if (false == horizontal) {
        for (auto& element : level->elements) {
            if (nullptr != element.child && true == element.child->active)
                this->drawLevel(graphics, element.child, static_cast<int>(element.visualization.rectangle().GetRight() + 0.5f) - 10,
                                static_cast<int>(element.visualization.rectangle().GetTop() + 0.5f) - 2, false);
        }
    }
}

void Toolbar::setPosition(const Gdiplus::PointF& position) {
    (void)position;
}

void Toolbar::move(const Gdiplus::PointF& direction) {
    (void)direction;
}

void Toolbar::initialize() {
    this->m_toplevel = std::shared_ptr<Toolbar::Level>(new Toolbar::Level());

    this->m_toplevel->active = true;

    /* set the settings menu */
    Toolbar::createElement("SETTINGS", Toolbar::ClickId::Settings, this->m_toplevel);
    this->m_toplevel->elements.back().child = std::shared_ptr<Toolbar::Level>(new Toolbar::Level);
    Toolbar::createElement("RELOAD ALL", Toolbar::ClickId::Reload, this->m_toplevel->elements.back().child);
    Toolbar::createElement("RELOAD SYSTEM", Toolbar::ClickId::ReloadSystem, this->m_toplevel->elements.back().child);
    Toolbar::createElement("RELOAD AIRCRAFTS", Toolbar::ClickId::ReloadAircrafts, this->m_toplevel->elements.back().child);
    Toolbar::createElement("RELOAD AIRPORTS", Toolbar::ClickId::ReloadAirports, this->m_toplevel->elements.back().child);

    /* set the systems meny */
    Toolbar::createElement("SYSTEMS", Toolbar::ClickId::Systems, this->m_toplevel);
    this->m_toplevel->elements.back().child = std::shared_ptr<Toolbar::Level>(new Toolbar::Level);
    Toolbar::createElement("PDC", Toolbar::ClickId::PDC, this->m_toplevel->elements.back().child);
}

bool Toolbar::visualize(Gdiplus::Graphics* graphics) {
    if (nullptr == this->m_toplevel)
        this->initialize();

    /* draw the root element */
    auto area = this->m_parent->GetRadarArea();
    this->draw(graphics, this->m_toplevel, area.left, area.top - 1, true);

    /* check all child elements */
    for (auto& element : this->m_toplevel->elements) {
        if (nullptr != element.child && true == element.child->active)
            this->drawLevel(graphics, element.child, static_cast<int>(element.visualization.rectangle().X) - 10,
                            static_cast<int>(this->m_toplevel->rectangle.GetBottom()) - 2, false);
    }

    return true;
}
