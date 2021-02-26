/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the departure sequence window
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include "stdafx.h"

#include <system/ConfigurationRegistry.h>

#include "../PlugIn.h"
#include "../RadarScreen.h"
#include "DepartureSequenceWindow.h"

using namespace topskytower;
using namespace topskytower::euroscope;
using namespace topskytower::types;

DepartureSequenceWindow::DepartureSequenceWindow(RadarScreen* parent) :
        InsetWindow("DEPARTURES", parent, Gdiplus::RectF(100, 100, 400, 300), false, false),
        m_activeDepartures(),
        m_departureTable(new TableViewer(this->m_parent, { "C/S", "SID", "RWY", "H/P", "STS", "SEP", "TIME" },
                                         Gdiplus::RectF(this->m_contentArea.X + 2.0f, this->m_contentArea.Y - 2.0f,
                                                        this->m_contentArea.Width, this->m_contentArea.Height))),
        m_firstRendering(true) {
    this->m_departureTable->setMaxVisibleRows(10);

    this->m_elements.push_back(this->m_departureTable);
}

bool DepartureSequenceWindow::click(const Gdiplus::PointF& pt, UiManager::MouseButton button) {
    if (true == UiElement::isInRectangle(pt, this->m_contentArea) && true == this->m_departureTable->click(pt, button)) {
        std::size_t row, column;

        if (true == this->m_departureTable->clickedEntry(row, column)) {
            auto callsign = this->m_departureTable->entry(row, 0);

            int tagId = 0;
            if (3 == column)
                tagId = static_cast<int>(PlugIn::TagItemFunction::HoldingPointCandidatesMenu);
            else if (4 == column)
                tagId = static_cast<int>(PlugIn::TagItemFunction::DepartureGroundStatusMenu);

            RadarScreen::EuroscopeEvent event = {
                tagId,
                callsign,
                "",
                { static_cast<int>(pt.X), static_cast<int>(pt.Y) },
                { static_cast<int>(pt.X), static_cast<int>(pt.Y),
                  static_cast<int>(pt.X) + 100, static_cast<int>(pt.Y) + 100 },
            };

            this->m_parent->registerEuroscopeEvent(std::move(event));
        }

        return true;
    }

    return InsetWindow::click(pt, button);
}

std::string DepartureSequenceWindow::translate(types::FlightPlan::AtcCommand command) {
    switch (command) {
    case types::FlightPlan::AtcCommand::StartUp:
        return "ST-UP";
    case types::FlightPlan::AtcCommand::Deicing:
        return "DEICE";
    case types::FlightPlan::AtcCommand::Pushback:
        return "PUSH";
    case types::FlightPlan::AtcCommand::TaxiOut:
        return "TAXI";
    case types::FlightPlan::AtcCommand::LineUp:
        return "LI-UP";
    case types::FlightPlan::AtcCommand::Departure:
        return "DEPA";
    default:
        return "";
    }
}

bool DepartureSequenceWindow::updateRow(const types::Flight& flight, std::size_t row) {
    types::Length separation;
    types::Time spacing;

    /* update some flight information */
    this->m_departureTable->setElement(row, 1, flight.flightPlan().departureRoute());
    this->m_departureTable->setElement(row, 2, flight.flightPlan().departureRunway());
    this->m_departureTable->setElement(row, 3, this->m_parent->departureSequenceControl().holdingPoint(flight).name);
    this->m_departureTable->setElement(row, 4, this->translate(flight.flightPlan().departureFlag()));

    this->m_parent->departureSequenceControl().departureSpacing(flight, spacing, separation);

    /* update the values */
    if (0.0_s != spacing || 0.0_m != separation) {
        /* update the values for the separation */
        if (0.0_m != separation) {
            std::stringstream stream;
            stream << std::fixed << std::setprecision(1) << separation.convert(types::nauticmile);
            this->m_departureTable->setElement(row, 5, stream.str());
        }
        else {
            this->m_departureTable->setElement(row, 5, " ");
        }

        /* update the values for the spacing */
        if (0.0_s != spacing) {
            auto mins = static_cast<std::uint16_t>(spacing.convert(types::minute));
            auto seconds = static_cast<std::uint16_t>(spacing.convert(types::second)) - mins * 60;

            std::stringstream stream;
            stream << mins << ":" << std::setw(2) << std::setfill('0') << seconds;
            this->m_departureTable->setElement(row, 6, stream.str());
        }
        else {
            this->m_departureTable->setElement(row, 6, " ");
        }

        return true;
    }
    /* ready for departure */
    else {
        return false;
    }
}

bool DepartureSequenceWindow::visualize(Gdiplus::Graphics* graphics) {
    if (false == this->m_parent->isInitialized())
        return false;

    /* update the table information */
    std::size_t idx = 0;
    auto departures = this->m_parent->departureSequenceControl().allReadyForDepartureFlights();
    for (auto it = this->m_activeDepartures.begin(); this->m_activeDepartures.end() != it;) {
        auto dIt = std::find(departures.begin(), departures.end(), *it);
        if (departures.end() != dIt) {
            const auto& flight = system::FlightRegistry::instance().flight(*it);

            if (true == this->updateRow(flight, idx)) {
                idx += 1;
                ++it;
            }
            else {
                it = this->m_activeDepartures.erase(it);
                this->m_departureTable->removeRow(idx);
            }

            departures.erase(dIt);
        }
        /* remove a departed or disconnected flight */
        else {
            it = this->m_activeDepartures.erase(it);
            this->m_departureTable->removeRow(idx);
        }
    }

    if (0 != departures.size())
        this->setActive(true);

    /* insert the new departures */
    for (const auto& departure : std::as_const(departures)) {
        if (false == system::FlightRegistry::instance().flightExists(departure))
            continue;

        const auto& flight = system::FlightRegistry::instance().flight(departure);
        idx = this->m_departureTable->numberOfRows();
        this->m_departureTable->addRow();

        if (true == this->updateRow(flight, idx)) {
            this->m_departureTable->setElement(idx, 0, departure);
            this->m_activeDepartures.push_back(departure);
        }
        else {
            this->m_departureTable->removeRow(idx);
        }
    }

    if (0 != this->m_activeDepartures.size()) {
        if (true == this->m_firstRendering) {
            float yPos = this->m_parent->GetRadarArea().bottom * 0.6f;
            this->setPosition(Gdiplus::PointF(this->area().X, yPos));
            this->m_firstRendering = false;
        }

        /* calculate the required size */
        this->m_departureTable->prepareVisualization(graphics);
        Gdiplus::SizeF tableSize;
        this->m_departureTable->area().GetSize(&tableSize);
        tableSize.Width += 4.0f;
        this->setContentSize(tableSize);

        return InsetWindow::visualize(graphics);
    }
    else {
        return true;
    }
}
