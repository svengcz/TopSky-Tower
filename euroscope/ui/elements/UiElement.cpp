/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the base UI element
 */

#include "stdafx.h"

#include <system/ConfigurationRegistry.h>

#include "UiElement.h"

using namespace topskytower;
using namespace topskytower::euroscope;

UiElement::UiElement(RadarScreen* parent, const Gdiplus::RectF& rectangle) :
        m_parent(parent),
        m_area(rectangle) { }

bool UiElement::isInRectangle(const Gdiplus::PointF& pt, const Gdiplus::RectF& rectangle) {
    if (pt.X < rectangle.GetLeft() || pt.X > rectangle.GetRight())
        return false;
    if (pt.Y < rectangle.GetTop() || pt.Y > rectangle.GetBottom())
        return false;
    return true;
}

void UiElement::setPosition(const Gdiplus::PointF& position) {
    this->m_area.X = position.X;
    this->m_area.Y = position.Y;
}

void UiElement::move(const Gdiplus::PointF& direction) {
    this->m_area.X += direction.X;
    this->m_area.Y += direction.Y;
}

void UiElement::setArea(Gdiplus::RectF& area) {
    this->m_area = area;
}

Gdiplus::Color UiElement::backgroundColor() {
    const auto& color = system::ConfigurationRegistry::instance().systemConfiguration().uiBackgroundColor;
    return Gdiplus::Color(255, color[0], color[1], color[2]);
}

Gdiplus::Color UiElement::foregroundColor() {
    const auto& color = system::ConfigurationRegistry::instance().systemConfiguration().uiForegroundColor;
    return Gdiplus::Color(255, color[0], color[1], color[2]);
}

const Gdiplus::RectF& UiElement::area() const {
    return this->m_area;
}
