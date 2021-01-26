/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the checkbox
 */

#include "stdafx.h"

#include "../../PlugIn.h"
#include "../../RadarScreen.h"
#include "Checkbox.h"

using namespace topskytower::euroscope;

Checkbox::Checkbox(RadarScreen* parent, const std::string& title, const Gdiplus::RectF& dimension) :
        UiElement(parent, dimension),
        m_title(title),
        m_titleVisualization(),
        m_checked(false) {
    /* define the headline */
    this->m_titleVisualization.setFontColor(UiElement::foregroundColor());
}

bool Checkbox::clicked() const {
    return this->m_checked;
}

bool Checkbox::click(const Gdiplus::PointF& pt, UiManager::MouseButton button) {
    if (UiManager::MouseButton::Left == button && true == UiElement::isInRectangle(pt, this->m_area)) {
        this->m_checked = !this->m_checked;
        return true;
    }
    else {
        return false;
    }
}

bool Checkbox::visualize(Gdiplus::Graphics* graphics) {
    /* prepare the headline */
    this->m_titleVisualization.setGraphics(graphics);
    this->m_titleVisualization.setText(this->m_title);
    this->m_titleVisualization.setPosition(Gdiplus::PointF(this->area().X, this->area().Y));
    this->m_titleVisualization.visualize();

    /* draw the checkbox*/
    float size = this->m_titleVisualization.rectangle().Height - 5.0f;
    Gdiplus::RectF rect(this->m_titleVisualization.rectangle().GetRight() + 5.0f,
                        this->m_titleVisualization.rectangle().GetTop() + this->m_titleVisualization.rectangle().Height - size - 3.0f,
                        size, size);
    if (false == this->m_checked) {
        Gdiplus::Pen pen(UiElement::foregroundColor(), 1.0f);
        graphics->DrawRectangle(&pen, rect);
    }
    else {
        Gdiplus::SolidBrush brush(UiElement::foregroundColor());
        graphics->FillRectangle(&brush, rect);
    }

    return true;
}
