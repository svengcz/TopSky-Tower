/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the text viewer
 */

#include "stdafx.h"

#include <helper/String.h>
#include <system/ConfigurationRegistry.h>

#include "TextViewer.h"

using namespace topskytower;
using namespace topskytower::euroscope;

TextViewer::TextViewer(RadarScreen* parent, const std::string& text, const Gdiplus::RectF& dimension) :
        UiElement(parent, dimension),
        m_text(text),
        m_visualization() {
    this->m_visualization.setBold(true);
    this->m_visualization.setFontColor(UiElement::foregroundColor());
}

bool TextViewer::click(const Gdiplus::PointF& pt, UiManager::MouseButton button) {
    (void)button;
    return this->isInRectangle(pt, this->m_area);
}

bool TextViewer::prepareVisualization(Gdiplus::Graphics* graphics) {
    this->m_visualization.setGraphics(graphics);
    this->m_visualization.setText(this->m_text);

    this->m_area.Width = this->m_visualization.rectangle().Width + 8.0f;
    this->m_area.Height = this->m_visualization.rectangle().Height + 8.0f;

    return true;
}

bool TextViewer::visualize(Gdiplus::Graphics* graphics) {
    this->m_visualization.setGraphics(graphics);
    this->m_visualization.setText(this->m_text);

    this->m_visualization.setPosition(Gdiplus::PointF(this->m_area.X + 4.0f, this->m_area.Y + 4.0f));
    this->m_visualization.visualize();

    auto color = system::ConfigurationRegistry::instance().systemConfiguration().uiForegroundColor;
    Gdiplus::Pen pen(Gdiplus::Color(255, color[0], color[1], color[2]), 1.0f);
    graphics->DrawRectangle(&pen, this->m_area);

    return true;
}
