/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the METAR viewer
 */

#include "stdafx.h"

#include <system/ConfigurationRegistry.h>

#include "../RadarScreen.h"
#include "elements/TableViewer.h"
#include "MetarViewerWindow.h"

using namespace std::literals;
using namespace topskytower;
using namespace topskytower::euroscope;

MetarViewerWindow::MetarViewerWindow(RadarScreen* parent) :
        InsetWindow("METAR", parent, Gdiplus::RectF(100, 100, 400, 300), false, true) {
    /* add the message viewer */
    this->m_elements.push_back(new TableViewer(this->m_parent, { " ", "ICAO", "WIND", "QNH" }, this->m_contentArea));
}

MetarViewerWindow::~MetarViewerWindow() {
    if (0 != this->m_elements.size())
        delete this->m_elements.front();
    this->m_elements.clear();
}

void MetarViewerWindow::updateData() {
    for (auto it = this->m_qnhValues.begin(); this->m_qnhValues.end() != it; ++it) {
        auto value = system::ConfigurationRegistry::instance().runtimeConfiguration().weatherInformation.find(it->first)->second.qnh;
        if (it->second.qnhValue != value) {
            it->second.lastSwitch = std::chrono::system_clock::now();
            it->second.blinkUntil = it->second.lastSwitch + 10s;
            it->second.qnhValue = value;
        }
    }
}

bool MetarViewerWindow::visualize(Gdiplus::Graphics* graphics) {
    const auto& config = system::ConfigurationRegistry::instance().runtimeConfiguration();
    auto viewer = static_cast<TableViewer*>(this->m_elements.front());

    TimePoint currentStamp = std::chrono::system_clock::now();
    this->updateData();

    /* set up the table data */
    std::size_t rowIdx = 0;
    for (const auto& data : config.weatherInformation) {
        if (rowIdx + 1 > viewer->numberOfRows())
            viewer->addRow();

        viewer->setElement(rowIdx, 1, data.first);

        std::stringstream stream;
        if (false == data.second.wind.variable)
            stream << std::setw(3) << std::setfill('0') << static_cast<int>(data.second.wind.direction.convert(types::degree));
        else
            stream << "VRB";
        stream << std::setw(2) << std::setfill('0') << static_cast<int>(data.second.wind.speed.convert(types::knot));
        if (false == helper::Math::almostEqual(0.0f, data.second.wind.gusts.value()))
            stream << "G" << std::setw(2) << std::setfill('0') << static_cast<int>(data.second.wind.gusts.convert(types::knot));
        stream << "KT";

        viewer->setElement(rowIdx, 2, stream.str());

        stream.str("");
        stream << std::setw(4) << std::setfill(' ') << data.second.qnh;
        viewer->setElement(rowIdx, 3, stream.str());

        /* the color change is needed */
        auto& qnhData = this->m_qnhValues[data.first];
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(currentStamp - qnhData.lastSwitch).count();
        if (qnhData.blinkUntil >= currentStamp) {
            if (1000 <= milliseconds) {
                /* switch the color */
                if (qnhData.currentColor.GetValue() == UiElement::foregroundBlinkColor().GetValue())
                    qnhData.currentColor = UiElement::foregroundColor();
                else if (qnhData.currentColor.GetValue() == UiElement::foregroundColor().GetValue())
                    qnhData.currentColor = UiElement::foregroundBlinkColor();
                qnhData.lastSwitch = currentStamp;
            }
        }
        else {
            qnhData.currentColor = UiElement::foregroundColor();
        }
        viewer->setTextColor(rowIdx, 3, qnhData.currentColor);
    }

    /* prepare the information of the viewer */
    viewer->prepareVisualization(graphics);

    /* resize the complete table*/
    Gdiplus::SizeF tableSize;
    viewer->area().GetSize(&tableSize);
    this->setContentSize(tableSize);

    return InsetWindow::visualize(graphics);
}
