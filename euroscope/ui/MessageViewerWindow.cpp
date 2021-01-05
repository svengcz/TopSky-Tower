/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the generic message viewer
 */

#include "stdafx.h"

#include <system/ConfigurationRegistry.h>

#include "../RadarScreen.h"
#include "elements/TextViewer.h"
#include "MessageViewerWindow.h"

using namespace topskytower;
using namespace topskytower::euroscope;

MessageViewerWindow::MessageViewerWindow(RadarScreen* parent, const std::string& title, const std::string& message) :
        InsetWindow(title, parent, Gdiplus::RectF(0, 40, 400, 300), false),
        m_firstRendering(true) {
    /* add the message viewer */
    this->m_elements.push_back(new TextViewer(this->m_parent, message, this->m_contentArea));
}

MessageViewerWindow::~MessageViewerWindow() {
    if (0 != this->m_elements.size())
        delete this->m_elements.front();
    this->m_elements.clear();
}

void MessageViewerWindow::setActive(bool active) {
    InsetWindow::setActive(active);

    if (false == active) {
        this->m_parent->uiManager().removeCustomWindow(this);
        delete this;
    }
    else {
        this->m_parent->uiManager().addCustomWindow(this);
    }
}

void MessageViewerWindow::centeredPosition() {
    /* place the window in the center of the screen */
    auto width = this->m_parent->GetRadarArea().right - this->m_parent->GetRadarArea().left;
    auto height = this->m_parent->GetRadarArea().bottom - this->m_parent->GetRadarArea().top;
    InsetWindow::setPosition(Gdiplus::PointF(static_cast<float>(width) * 0.5f - this->m_area.Width * 0.5f,
                             static_cast<float>(height) * 0.5f - this->m_area.Height * 0.5f));

    if (0 != this->m_elements.size())
        this->m_elements.front()->setPosition(Gdiplus::PointF(this->m_contentArea.X, this->m_contentArea.Y));
}

bool MessageViewerWindow::visualize(Gdiplus::Graphics* graphics) {
    if (0 != this->m_elements.size() && true == this->m_firstRendering) {
        auto viewer = static_cast<TextViewer*>(this->m_elements.front());

        viewer->prepareVisualization(graphics);

        this->m_titleVisualization.setGraphics(graphics);
        this->m_titleVisualization.setText(this->title());
        this->m_titleVisualization.visualize();
        const auto& titleRectangle = this->m_titleVisualization.rectangle();

        Gdiplus::SizeF size(max(titleRectangle.Width + 20.0f, viewer->area().Width), viewer->area().Height);
        this->setContentSize(size);

        Gdiplus::RectF newArea(Gdiplus::PointF(viewer->area().X, viewer->area().Y), size);
        viewer->setArea(newArea);

        this->centeredPosition();

        this->m_firstRendering = false;
    }

    return InsetWindow::visualize(graphics);
}