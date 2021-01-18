/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the table viewer
 */

#include "stdafx.h"

#include "TableViewer.h"

using namespace topskytower;
using namespace topskytower::euroscope;

TableViewer::TableViewer(RadarScreen* parent, const std::vector<std::string>& header, const Gdiplus::RectF& dimension) :
        UiElement(parent, dimension),
        m_header(),
        m_rows() {
    this->m_header.reserve(header.size());

    for (const auto& row : std::as_const(header)) {
        this->m_header.push_back(Text());
        this->m_header.back().setText(row);
        this->m_header.back().setFontColor(UiElement::backgroundColor());
    }
}

std::size_t TableViewer::numberOfRows() const {
    return this->m_rows.size();
}

std::size_t TableViewer::numberOfColumns() const {
    return this->m_header.size();
}

void TableViewer::addRow() {
    this->m_rows.push_back(std::move(std::vector<Text>(this->m_header.size())));
}

void TableViewer::removeRow(std::size_t idx) {
    if (idx < this->m_rows.size()) {
        auto it = this->m_rows.begin();
        std::advance(it, idx);
        this->m_rows.erase(it);
    }
}

void TableViewer::setElement(std::size_t rowIdx, std::size_t columnIdx, const std::string& element) {
    if (rowIdx < this->m_rows.size() && columnIdx < this->m_header.size()) {
        auto it = this->m_rows.begin();
        std::advance(it, rowIdx);
        (*it)[columnIdx].setText(element);
    }
}

bool TableViewer::click(const Gdiplus::PointF& pt, UiManager::MouseButton button) {
    if (UiManager::MouseButton::Left != button)
        return false;

    return UiElement::isInRectangle(pt, this->m_area);
}

float TableViewer::calculateRequiredArea(std::vector<float>& columnWidths, Gdiplus::Graphics* graphics) {
    /* contains the per-column widths */
    columnWidths = std::vector<float>(this->m_header.size(), 0.0f);
    std::size_t emptyLines = 0;

    /* get the widths of the header */
    for (std::size_t i = 0; i < this->m_header.size(); ++i) {
        this->m_header[i].setGraphics(graphics);
        columnWidths[i] = std::max(columnWidths[i], this->m_header[i].rectangle().Width);
    }

    /* get the width of every row element */
    for (auto& row : this->m_rows) {
        bool emptyRow = true;

        for (std::size_t i = 0; i < row.size(); ++i) {
            row[i].setGraphics(graphics);
            columnWidths[i] = std::max(columnWidths[i], row[i].rectangle().Width);
            if (false == helper::Math::almostEqual(0.0f, row[i].rectangle().Width))
                emptyRow = false;
        }

        if (true == emptyRow)
            emptyLines += 1;
    }

    /* get the overall width */
    float overallWidth = 0.0f;
    for (const auto& width : std::as_const(columnWidths))
        overallWidth += width;

    this->m_area = Gdiplus::RectF(this->m_area.X, this->m_area.Y, overallWidth,
                                  (this->m_rows.size() - emptyLines + 1) * this->m_header[0].rectangle().Height);

    return overallWidth;
}

void TableViewer::prepareVisualization(Gdiplus::Graphics* graphics) {
    std::vector<float> columnWidths;
    this->calculateRequiredArea(columnWidths, graphics);
}

bool TableViewer::visualize(Gdiplus::Graphics* graphics) {
    std::vector<float> columnWidths;

    float overallWidth = this->calculateRequiredArea(columnWidths, graphics);
    float offsetX = this->m_area.X;
    float offsetY = this->m_area.Y;

    /* draw the background of the header */
    Gdiplus::SolidBrush headerBrush(UiElement::foregroundColor());
    graphics->FillRectangle(&headerBrush, Gdiplus::RectF(offsetX, offsetY, overallWidth, this->m_header[0].rectangle().Height));

    /* draw the header */
    for (std::size_t i = 0; i < this->m_header.size(); ++i) {
        this->m_header[i].setPosition(Gdiplus::PointF(offsetX, offsetY));
        this->m_header[i].setGraphics(graphics);
        this->m_header[i].visualize();
        offsetX += columnWidths[i];
    }
    offsetY += this->m_header[0].rectangle().Height;

    /* draw the rows */
    Gdiplus::Pen pen(UiElement::foregroundColor(), 1.0f);
    for (auto& row : this->m_rows) {
        offsetX = this->m_area.X;

        for (std::size_t i = 0; i < this->m_header.size(); ++i) {
            row[i].setPosition(Gdiplus::PointF(offsetX, offsetY));
            row[i].setGraphics(graphics);
            row[i].visualize();
            offsetX += columnWidths[i];
        }

        offsetY += this->m_header[0].rectangle().Height;
        graphics->DrawLine(&pen, Gdiplus::PointF(this->m_area.X, offsetY), Gdiplus::PointF(this->m_area.X + overallWidth, offsetY));
    }

    return true;
}
