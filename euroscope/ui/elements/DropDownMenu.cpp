/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the drop down menu
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include "stdafx.h"

#include "../../PlugIn.h"
#include "../../RadarScreen.h"
#include "DropDownMenu.h"

using namespace topskytower::euroscope;

DropDownMenu::DropDownMenu(RadarScreen* parent, const std::string& headline, const Gdiplus::RectF& dimension) :
        UiElement(parent, dimension),
        m_headline(headline),
        m_content(),
        m_headlineVisualization(),
        m_contentVisualization(),
        m_fieldRectangle(dimension),
        m_elements() {
    /* define the headline */
    this->m_headlineVisualization.setFontColor(UiElement::foregroundColor());

    /* define the content */
    this->m_contentVisualization.setFontColor(UiElement::foregroundColor());
}

void DropDownMenu::setEntries(std::initializer_list<std::string> entries) {
    this->m_elements.clear();

    for (auto& entry : entries)
        this->m_elements.push_back(entry);
}

void DropDownMenu::addEntry(const std::string& entry) {
    this->m_elements.push_back(entry);
}

const std::string& DropDownMenu::selected() const {
    return this->m_content;
}

int DropDownMenu::selectedIndex() const {
    if (0 != this->m_content.length()) {
        int idx = 0;
        for (const auto& element : this->m_elements) {
            if (element == this->m_content)
                return idx;
            idx += 1;
        }
    }

    return -1;
}

void DropDownMenu::setPosition(const Gdiplus::PointF& position) {
    UiElement::setPosition(position);

    /* set the new overall position */
    this->m_fieldRectangle.X = position.X;
    this->m_fieldRectangle.Y = position.Y;
}

void DropDownMenu::move(const Gdiplus::PointF& direction) {
    UiElement::move(direction);

    /* move the complete dimension */
    this->m_fieldRectangle.X += direction.X;
    this->m_fieldRectangle.Y += direction.Y;
}

void DropDownMenu::uiCallback(const std::string& string) {
    this->m_content = string;
}

bool DropDownMenu::click(const Gdiplus::PointF& pt, UiManager::MouseButton button) {
    /* ignore non-left button clicks */
    if (UiManager::MouseButton::Left != button)
        return false;

    /* check if we are clicked */
    if (true == UiElement::isInRectangle(pt, this->m_fieldRectangle)) {
        auto plugin = static_cast<PlugIn*>(this->m_parent->GetPlugIn());
        plugin->registerUiCallback(this, &DropDownMenu::uiCallback);

        /* calculate the box of the edit text */
        POINT point{ static_cast<int>(pt.X), static_cast<int>(pt.Y) };
        RECT rect{
            point.x, point.y,
            static_cast<int>(this->m_fieldRectangle.GetRight() + 0.5f),
            static_cast<int>(this->m_fieldRectangle.GetBottom() + 0.5f)
        };

        std::string entries;
        for (const auto& element : std::as_const(this->m_elements))
            entries += element + ";";
        plugin->OnFunctionCall(static_cast<int>(PlugIn::TagItemFunction::UiDropDownRequest),
                               entries.c_str(), point, rect);
    }

    return false;
}

bool DropDownMenu::visualize(Gdiplus::Graphics* graphics) {
    /* prepare the headline */
    this->m_headlineVisualization.setGraphics(graphics);
    this->m_headlineVisualization.setText(this->m_headline);

    /* prepare the content */
    this->m_contentVisualization.setGraphics(graphics);
    this->m_contentVisualization.setText(this->m_content);

    /* draw the box */
    Gdiplus::Pen pen(UiElement::foregroundColor(), 1.0f);
    graphics->DrawRectangle(&pen, this->m_fieldRectangle);

    /* draw the headline */
    Gdiplus::PointF position(this->m_fieldRectangle.X + 2.0f, this->m_fieldRectangle.Y - 2.0f - this->m_headlineVisualization.rectangle().Height * 0.8f);
    this->m_headlineVisualization.setPosition(position);
    this->m_headlineVisualization.visualize();

    /* initialize the overall area */
    this->m_area = this->m_headlineVisualization.rectangle();

    /* draw the content */
    position = Gdiplus::PointF(this->m_fieldRectangle.X + 2.0f, this->m_fieldRectangle.Y + 2.0f);
    this->m_contentVisualization.setPosition(position);
    this->m_contentVisualization.visualize();

    /* finalize the overall area */
    this->m_area.X = min(this->m_area.X, this->m_contentVisualization.rectangle().X);
    this->m_area.Y = min(this->m_area.Y, this->m_contentVisualization.rectangle().Y);
    this->m_area.Width = max(this->m_area.Width, this->m_contentVisualization.rectangle().Width);
    this->m_area.Height += this->m_contentVisualization.rectangle().Y - this->m_area.Y - this->m_area.Height + this->m_contentVisualization.rectangle().Height;

    return true;
}
