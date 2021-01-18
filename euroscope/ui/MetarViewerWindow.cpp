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

bool MetarViewerWindow::visualize(Gdiplus::Graphics* graphics) {
    const auto& config = system::ConfigurationRegistry::instance().runtimeConfiguration();
    auto viewer = static_cast<TableViewer*>(this->m_elements.front());

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
    }

    /* prepare the information of the viewer */
    viewer->prepareVisualization(graphics);

    /* resize the complete table*/
    Gdiplus::SizeF tableSize;
    viewer->area().GetSize(&tableSize);
    this->setContentSize(tableSize);

    return InsetWindow::visualize(graphics);
}
