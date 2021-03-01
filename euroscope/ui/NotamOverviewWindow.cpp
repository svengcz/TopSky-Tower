/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the NOTAM overview window
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include "stdafx.h"

#include <helper/Time.h>
#include <system/ConfigurationRegistry.h>

#include "../RadarScreen.h"
#include "MessageViewerWindow.h"
#include "NotamOverviewWindow.h"

using namespace topskytower;
using namespace topskytower::euroscope;

#define MENU_HEIGHT          20.0f
#define MENU_CONTENT_SPACING 15.0f
#define MENU_TABLE_SPACING   25.0f

NotamOverviewWindow::NotamOverviewWindow(RadarScreen* parent) :
        InsetWindow("NOTAMs", parent, Gdiplus::RectF(0, 40, 400, 300), false, true),
        m_firstRendering(true),
        m_airportFilter(new EditText(this->m_parent, "ICAO", Gdiplus::RectF(this->m_contentArea.X + 5.0f, this->m_contentArea.Y + 15.0f,
                                                                            100.0f, MENU_HEIGHT))),
        m_categoryFilter(new DropDownMenu(this->m_parent, "CATEGORY", Gdiplus::RectF(this->m_contentArea.X + 130.0f, this->m_contentArea.Y + 15.0f,
                                                                                     200.0f, MENU_HEIGHT))),
        m_activeFilter(new Checkbox(this->m_parent, "Show active ", Gdiplus::RectF(this->m_contentArea.X + this->m_contentArea.Width - 115.0f, this->m_contentArea.Y + 15.0f,
                                                                                   100.0f, MENU_HEIGHT))),
        m_notamOverview(new TableViewer(this->m_parent, { "ICAO", "CATEGORY", "TITLE", "START", "END" },
                                        Gdiplus::RectF(this->m_contentArea.X + 2.0f, this->m_contentArea.Y + 10.0f + MENU_HEIGHT + MENU_CONTENT_SPACING,
                                                       this->m_contentArea.Width, this->m_contentArea.Height - MENU_TABLE_SPACING))) {
    this->m_notamOverview->setMinVisibleRows(10);
    this->m_notamOverview->setMaxVisibleRows(10);
    this->m_notamOverview->setMinimumColumnWidths({35.0f, 105.0f, 165.0f, 115.0f, 115.0f});

    this->m_elements.push_back(this->m_airportFilter);
    this->m_elements.push_back(this->m_categoryFilter);
    this->m_elements.push_back(this->m_activeFilter);
    this->m_elements.push_back(this->m_notamOverview);

    this->m_categoryFilter->setEntries(
        {
            "All",
            "Other",
            "Movement area",
            "Bearing strength",
            "Clearway",
            "Declared distances",
            "Taxiing guidance system",
            "Runway arresting gear",
            "Parking area",
            "Daylight markings",
            "Apron",
            "Stopbar",
            "Aircraft stands",
            "Runway",
            "Stopbar",
            "Threshold",
            "Runway turning bay",
            "Strip/shoulder",
            "Taxiway",
            "Rapid exit taxiway"
        }
    );
}

std::string NotamOverviewWindow::translateCategory(management::NotamCategory category) {
    switch (category) {
    case management::NotamCategory::Other:
        return "Other";
    case management::NotamCategory::MovementArea:
        return "Movement area";
    case management::NotamCategory::BearingStrength:
        return "Bearing strength";
    case management::NotamCategory::Clearway:
        return "Clearway";
    case management::NotamCategory::DeclaredDistances:
        return "Declared distances";
    case management::NotamCategory::TaxiGuidance:
        return "Taxiing guidance system";
    case management::NotamCategory::RunwayArrestingGear:
        return "Runway arresting gear";
    case management::NotamCategory::Parking:
        return "Parking area";
    case management::NotamCategory::DaylightMarkings:
        return "Daylight markings";
    case management::NotamCategory::Apron:
        return "Apron";
    case management::NotamCategory::Stopbar:
        return "Stopbar";
    case management::NotamCategory::Stands:
        return "Aircraft stands";
    case management::NotamCategory::Runway:
        return "Runway";
    case management::NotamCategory::Stopway:
        return "Stopbar";
    case management::NotamCategory::Threshold:
        return "Threshold";
    case management::NotamCategory::RunwayTurningBay:
        return "Runway turning bay";
    case management::NotamCategory::Strip:
        return "Strip/shoulder";
    case management::NotamCategory::Taxiway:
        return "Taxiway";
    case management::NotamCategory::RapidExit:
        return "Rapid exit taxiway";
    default:
        return "Unknown";
    }
}

void NotamOverviewWindow::setOverviewContent() {
    std::size_t oldSize = this->m_notamOverview->numberOfRows();
    std::string filter = this->m_airportFilter->content();
    auto currentTime = helper::Time::currentUtc();
    std::list<std::size_t> foundIndices;

#pragma warning(disable: 4244)
    std::transform(filter.begin(), filter.end(), filter.begin(), ::toupper);
#pragma warning (default: 4244)

    for (const auto& notams : std::as_const(management::NotamControl::instance().notams())) {
        if (0 != filter.length() && std::string::npos == notams.first.find(filter))
            continue;

        for (const auto& notam : std::as_const(notams.second)) {
            bool found = false;

            /* check if the NOTAM is active or not */
            if (true == this->m_activeFilter->checked()) {
                if (notam->startTime > currentTime || notam->endTime < currentTime)
                    continue;
            }

            /* check if the type filter is active */
            if (0 != this->m_categoryFilter->selected().length()) {
                auto category = static_cast<management::NotamCategory>(this->m_categoryFilter->selectedIndex());
                if (management::NotamCategory::Unknown != category && notam->category != category)
                    continue;
            }

            /* check if the NOTAM exists */
            for (std::size_t row = 0; row < this->m_notamOverview->numberOfRows(); ++row) {
                if (this->m_notamOverview->entry(row, 0) == notams.first && this->m_notamOverview->entry(row, 2) == notam->title) {
                    foundIndices.push_back(row);
                    found = true;
                }
            }

            /* create a new entry */
            if (false == found) {
                /* get the correct colorization of the row based on the interpreter state */
                Gdiplus::Color textColor;
                const std::uint8_t* rgb;

                switch (notam->state) {
                case management::NotamInterpreterState::Failed:
                    rgb = system::ConfigurationRegistry::instance().systemConfiguration().uiWarningColor;
                    textColor = Gdiplus::Color(rgb[0], rgb[1], rgb[2]);
                    break;
                case management::NotamInterpreterState::Ignored:
                case management::NotamInterpreterState::Pending:
                    textColor = UiElement::foregroundColor();
                    textColor = Gdiplus::Color(120, textColor.GetR(), textColor.GetG(), textColor.GetB());
                    break;
                case management::NotamInterpreterState::Success:
                    textColor = UiElement::foregroundColor();
                    break;
                default:
                    rgb = system::ConfigurationRegistry::instance().systemConfiguration().uiErrorColor;
                    textColor = Gdiplus::Color(rgb[0], rgb[1], rgb[2]);
                    break;
                }

                this->m_notamOverview->addRow();
                this->m_notamOverview->setElement(this->m_notamOverview->numberOfRows() - 1, 0, notams.first);
                this->m_notamOverview->setElement(this->m_notamOverview->numberOfRows() - 1, 1, NotamOverviewWindow::translateCategory(notam->category));
                this->m_notamOverview->setElement(this->m_notamOverview->numberOfRows() - 1, 2, notam->title);
                this->m_notamOverview->setElement(this->m_notamOverview->numberOfRows() - 1, 3, helper::Time::timeToString(notam->startTime, "%Y-%m-%d %H:%M"));
                this->m_notamOverview->setElement(this->m_notamOverview->numberOfRows() - 1, 4, helper::Time::timeToString(notam->endTime, "%Y-%m-%d %H:%M"));

                for (std::size_t i = 0; i < 5; ++i)
                    this->m_notamOverview->setTextColor(this->m_notamOverview->numberOfRows() - 1, i, textColor);
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

bool NotamOverviewWindow::click(const Gdiplus::PointF& pt, UiManager::MouseButton button) {
    if (true == UiElement::isInRectangle(pt, this->m_contentArea)) {
        if (true == this->m_notamOverview->click(pt, button)) {
            std::size_t row, column;

            if (true == this->m_notamOverview->clickedEntry(row, column)) {
                const auto& airport = this->m_notamOverview->entry(row, 0);
                const auto& title = this->m_notamOverview->entry(row, 2);

                auto notamsIt = management::NotamControl::instance().notams().find(airport);
                if (management::NotamControl::instance().notams().cend() != notamsIt) {
                    for (const auto& notam : std::as_const(notamsIt->second)) {
                        if (notam->title == title && false == this->m_parent->uiManager().windowIsActive(title)) {
                            auto viewer = new MessageViewerWindow(this->m_parent, title, notam->rawMessage);
                            viewer->setActive(true);
                            return true;
                        }
                    }
                }
            }

            return true;
        }
        else if (true == this->m_activeFilter->click(pt, button) || true == this->m_airportFilter->click(pt, button)) {
            return true;
        }
    }

    return InsetWindow::click(pt, button);
}

bool NotamOverviewWindow::visualize(Gdiplus::Graphics* graphics) {
    if (0 != this->m_elements.size())
        this->setOverviewContent();

    if (true == this->m_firstRendering) {
        auto area = this->m_parent->GetRadarArea();

        float width = static_cast<float>(area.right - area.left);
        float height = static_cast<float>(area.bottom - area.top);
        float x = width - this->m_notamOverview->area().Width - 250.0f;
        if (0.0f > x)
            x = 0.0f;
        float y = height - this->m_notamOverview->area().Height - 20.0f;
        if (0.0f > y)
            y = 0.0f;
        this->setPosition(Gdiplus::PointF(x, y));

        this->m_firstRendering = false;
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
