/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the toolbar button
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include "stdafx.h"

#include "ToolbarButton.h"

using namespace topskytower::euroscope;

ToolbarButton::ToolbarButton(RadarScreen* parent, const std::string& text, const Gdiplus::RectF& dimension) :
        UiElement(parent, dimension),
        m_text(text),
        m_visualization() {
    this->m_visualization.setBold(true);
    this->m_visualization.setFontColor(UiElement::foregroundColor());
}

void ToolbarButton::setText(const std::string& text) {
    this->m_text = text;
}

bool ToolbarButton::click(const Gdiplus::PointF& pt, UiManager::MouseButton button) {
    /* only left mouse clicks are allowed */
    if (UiManager::MouseButton::Left != button)
        return false;

    /* check if the button is clicked */
    if (true == UiElement::isInRectangle(pt, this->m_area)) {
        this->clicked();
        return true;
    }
    else {
        return false;
    }
}

bool ToolbarButton::visualize(Gdiplus::Graphics* graphics) {
    bool isActive = this->active();

    /* get the correct colors */
    Gdiplus::Color background, foreground;
    if (true == isActive) {
        background = UiElement::activeBackgroundColor();
        foreground = UiElement::activeForegroundColor();
    }
    else {
        background = UiElement::foregroundColor();
        foreground = UiElement::backgroundColor();
    }

    /* draw the background */
    Gdiplus::SolidBrush brush(background);
    graphics->FillRectangle(&brush, this->m_area);

    /* draw the border */
    Gdiplus::Pen pen(foreground, 0.5f);
    graphics->DrawRectangle(&pen, this->m_area);

    /* prepare the text */
    this->m_visualization.setGraphics(graphics);
    this->m_visualization.setFontColor(foreground);
    this->m_visualization.setText(this->m_text);

    /* calculate the position of the text */
    float x = (this->m_area.Width - this->m_visualization.rectangle().Width) * 0.5f;
    float y = (this->m_area.Height - this->m_visualization.rectangle().Height) * 0.5f;
    Gdiplus::PointF position(x + this->m_area.X, y + this->m_area.Y + 1.0f);
    this->m_visualization.setPosition(position);

    /* visualize the text */
    this->m_visualization.visualize();

    return true;
}
