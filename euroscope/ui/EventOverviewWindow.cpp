/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the event overview window
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include "stdafx.h"

#include <system/ConfigurationRegistry.h>

#include "../RadarScreen.h"
#include "EventOverviewWindow.h"

using namespace topskytower;
using namespace topskytower::euroscope;

EventOverviewWindow::EventOverviewWindow(RadarScreen* parent) :
        InsetWindow("Events", parent, Gdiplus::RectF(0, 40, 400, 300), false, true),
        m_windowLock(),
        m_updateViaClick(false),
        m_eventOverview(new TableViewer(this->m_parent, { "TITLE" },
                                        Gdiplus::RectF(this->m_contentArea.X, this->m_contentArea.Y,
                                                       this->m_contentArea.Width, this->m_contentArea.Height))) {
    this->m_eventOverview->visualizeHeader(false);
    this->m_eventOverview->setMaxVisibleRows(15);
    this->m_elements.push_back(this->m_eventOverview);

    system::ConfigurationRegistry::instance().registerNotificationCallback(this, &EventOverviewWindow::setOverviewContent);
}

EventOverviewWindow::~EventOverviewWindow() {
    system::ConfigurationRegistry::instance().deleteNotificationCallback(this);

    while (0 != this->m_elements.size()) {
        delete this->m_elements.front();
        this->m_elements.pop_front();
    }
}

void EventOverviewWindow::setOverviewContent(system::ConfigurationRegistry::UpdateType type) {
    if (system::ConfigurationRegistry::UpdateType::All != type && system::ConfigurationRegistry::UpdateType::Events != type)
        return;

    if (true == this->m_updateViaClick) {
        this->m_updateViaClick = false;
        return;
    }

    std::lock_guard guard(this->m_windowLock);

    this->m_eventOverview->clear();

    std::size_t i = 0;
    const auto& events = system::ConfigurationRegistry::instance().eventRoutesConfiguration().events;
    for (const auto& event : std::as_const(events)) {
        this->m_eventOverview->addRow();
        this->m_eventOverview->setElement(i++, 0, event.name);
    }
}

bool EventOverviewWindow::click(const Gdiplus::PointF& pt, UiManager::MouseButton button) {
    std::lock_guard guard(this->m_windowLock);

    if (true == UiElement::isInRectangle(pt, this->m_contentArea) && true == this->m_eventOverview->click(pt, button)) {
        std::size_t row, column;

        if (true == this->m_eventOverview->clickedEntry(row, column)) {
            const auto& eventName = this->m_eventOverview->entry(row, 0);
            bool activate = UiElement::backgroundColor().GetValue() == this->m_eventOverview->backgroundColor(row, column).GetValue();

            /* change the colorization */
            if (true == activate) {
                this->m_eventOverview->setTextColor(row, column, UiElement::backgroundColor());
                this->m_eventOverview->setBackgroundColor(row, column, UiElement::foregroundColor());
            }
            else {
                this->m_eventOverview->setTextColor(row, column, UiElement::foregroundColor());
                this->m_eventOverview->setBackgroundColor(row, column, UiElement::backgroundColor());
            }

            /* update the configuration */
            this->m_updateViaClick = true;
            system::ConfigurationRegistry::instance().activateEvent(eventName, activate);
        }

        return true;
    }

    return InsetWindow::click(pt, button);
}

bool EventOverviewWindow::visualize(Gdiplus::Graphics* graphics) {
    if (0 == this->m_eventOverview->numberOfRows())
        this->setOverviewContent(system::ConfigurationRegistry::UpdateType::Events);

    std::lock_guard guard(this->m_windowLock);

    /* calculate the required size */
    this->m_eventOverview->prepareVisualization(graphics);
    Gdiplus::SizeF tableSize;
    this->m_eventOverview->area().GetSize(&tableSize);
    this->setContentSize(tableSize);

    return InsetWindow::visualize(graphics);
}
