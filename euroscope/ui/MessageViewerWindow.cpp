/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the generic message viewer
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include "stdafx.h"

#include <system/ConfigurationRegistry.h>

#include "../RadarScreen.h"
#include "elements/TextViewer.h"
#include "MessageViewerWindow.h"

using namespace topskytower;
using namespace topskytower::euroscope;

MessageViewerWindow::MessageViewerWindow(RadarScreen* parent, const std::string& title, const std::string& message) :
        InsetWindow(title, parent, Gdiplus::RectF(0, 40, 400, 300), false, true),
        m_firstRendering(true) {
    /* add the message viewer */
    this->m_elements.push_back(new TextViewer(this->m_parent, message, this->m_contentArea));
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

        this->setCenteredPosition();

        this->m_firstRendering = false;
    }

    return InsetWindow::visualize(graphics);
}
