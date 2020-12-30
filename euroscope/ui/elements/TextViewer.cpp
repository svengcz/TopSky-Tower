/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the text viewer
 */

#include "stdafx.h"

#include <helper/String.h>

#include "TextViewer.h"

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

const Gdiplus::RectF& TextViewer::textRectangle() const {
    return this->m_visualization.rectangle();
}

bool TextViewer::prepareVisualization(Gdiplus::Graphics* graphics) {
    this->m_visualization.setGraphics(graphics);
    this->m_visualization.setText(this->m_text);
    return true;
}

bool TextViewer::visualize(Gdiplus::Graphics* graphics) {
    this->prepareVisualization(graphics);

    this->m_visualization.setPosition(Gdiplus::PointF(this->m_area.X, this->m_area.Y));
    this->m_visualization.visualize();

    return true;
}
