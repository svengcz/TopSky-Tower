/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the NOTAM overview window
 */

#define _CRT_SECURE_NO_WARNINGS 1
#include "stdafx.h"

#include <helper/Time.h>
#include <management/NotamControl.h>
#include <system/ConfigurationRegistry.h>

#include "../RadarScreen.h"
#include "NotamOverviewWindow.h"

using namespace topskytower;
using namespace topskytower::euroscope;

#define MENU_HEIGHT          20.0f
#define MENU_CONTENT_SPACING 15.0f
#define MENU_TABLE_SPACING   25.0f

NotamOverviewWindow::NotamOverviewWindow(RadarScreen* parent) :
        InsetWindow("NOTAMs", parent, Gdiplus::RectF(0, 40, 400, 300), false, true),
        m_airportFilter(new EditText(this->m_parent, "ICAO", Gdiplus::RectF(this->m_contentArea.X + 5.0f, this->m_contentArea.Y + 15.0f,
                                                                            100.0f, MENU_HEIGHT))),
        m_activeFilter(new Checkbox(this->m_parent, "Show active ", Gdiplus::RectF(this->m_contentArea.X + this->m_contentArea.Width - 115.0f, this->m_contentArea.Y + 15.0f,
                                                                                   100.0f, MENU_HEIGHT))),
        m_notamOverview(new TableViewer(this->m_parent, { "ICAO", "TITLE", "START", "END" },
                                        Gdiplus::RectF(this->m_contentArea.X + 2.0f, this->m_contentArea.Y + 10.0f + MENU_HEIGHT + MENU_CONTENT_SPACING,
                                                       this->m_contentArea.Width, this->m_contentArea.Height - MENU_TABLE_SPACING))) {
    this->m_notamOverview->setMaxVisibleRows(10);

    this->m_elements.push_back(this->m_airportFilter);
    this->m_elements.push_back(this->m_activeFilter);
    this->m_elements.push_back(this->m_notamOverview);
}

NotamOverviewWindow::~NotamOverviewWindow() {
    if (0 != this->m_elements.size())
        delete this->m_elements.front();
    this->m_elements.clear();
}

void NotamOverviewWindow::setOverviewContent() {
    std::string filter = this->m_airportFilter->content();
    std::list<std::size_t> foundIndices;
    std::size_t oldSize = this->m_notamOverview->numberOfRows();

#pragma warning(disable: 4244)
    std::transform(filter.begin(), filter.end(), filter.begin(), ::toupper);
#pragma warning (default: 4244)

    for (const auto& notams : std::as_const(management::NotamControl::instance().notams())) {
        if (0 != filter.length() && std::string::npos == notams.first.find(filter))
            continue;

        for (const auto& notam : std::as_const(notams.second)) {
            bool found = false;

            /* check if the NOTAM exists */
            for (std::size_t row = 0; row < this->m_notamOverview->numberOfRows(); ++row) {
                if (this->m_notamOverview->entry(row, 0) == notams.first && this->m_notamOverview->entry(row, 1) == notam.title) {
                    foundIndices.push_back(row);
                    found = true;
                }
            }

            /* create a new entry */
            if (false == found) {
                this->m_notamOverview->addRow();
                this->m_notamOverview->setElement(this->m_notamOverview->numberOfRows() - 1, 0, notams.first);
                this->m_notamOverview->setElement(this->m_notamOverview->numberOfRows() - 1, 1, notam.title);
                this->m_notamOverview->setElement(this->m_notamOverview->numberOfRows() - 1, 2, helper::Time::timeToString(notam.startTime, "%Y-%m-%d %H:%Mz"));
                this->m_notamOverview->setElement(this->m_notamOverview->numberOfRows() - 1, 3, helper::Time::timeToString(notam.endTime, "%Y-%m-%d %H:%Mz"));
            }
        }
    }

    if (0 != oldSize) {
        /* create the negative list to delete the deleted rows */
        std::list<std::size_t> missingRows;
        for (std::size_t i = oldSize - 1; i > 0; --i) {
            if (foundIndices.cend() == std::find(foundIndices.cbegin(), foundIndices.cend(), i))
                missingRows.push_back(i);
        }
        if (foundIndices.cend() == std::find(foundIndices.cbegin(), foundIndices.cend(), 0UL))
            missingRows.push_back(0UL);

        for (const auto& idx : std::as_const(missingRows))
            this->m_notamOverview->removeRow(idx);
    }
}

bool NotamOverviewWindow::visualize(Gdiplus::Graphics* graphics) {
    if (0 != this->m_elements.size()) {
        this->setOverviewContent();
    }

    /* calculate the required size */
    this->m_notamOverview->prepareVisualization(graphics);
    Gdiplus::SizeF tableSize;
    this->m_notamOverview->area().GetSize(&tableSize);
    tableSize.Width += 4.0f;
    tableSize.Height += MENU_HEIGHT + MENU_TABLE_SPACING;
    this->setContentSize(tableSize);

    this->m_activeFilter->setPosition(Gdiplus::PointF(this->m_contentArea.X + this->m_contentArea.Width - 115.0f, this->m_contentArea.Y + 15.0f));

    return InsetWindow::visualize(graphics);
}
