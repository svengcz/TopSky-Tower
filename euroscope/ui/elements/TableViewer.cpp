/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the table viewer
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include "stdafx.h"

#include "TableViewer.h"

using namespace topskytower;
using namespace topskytower::euroscope;

TableViewer::TableViewer(RadarScreen* parent, const std::vector<std::string>& header, const Gdiplus::RectF& dimension) :
        UiElement(parent, dimension),
        m_headerContent(header),
        m_header(),
        m_rowContent(),
        m_rows(),
        m_visibleRows(std::numeric_limits<std::size_t>::max()),
        m_visibleRowOffset(0),
        m_scrollUp(),
        m_scrollDown(),
        m_sliderRectangle(),
        m_columnWidths(header.size(), 0.0f),
        m_rowHeight(0.0f),
        m_clickedRow(std::numeric_limits<std::size_t>::max()),
        m_clickedColumn(std::numeric_limits<std::size_t>::max()) {
    this->m_header.reserve(header.size());

    for (std::size_t i = 0; i < header.size(); ++i) {
        this->m_header.push_back(Text());
        this->m_header.back().setFontColor(UiElement::foregroundColor());
        this->m_header.back().setBold(true);
    }
}

std::size_t TableViewer::numberOfRows() const {
    return this->m_rows.size();
}

std::size_t TableViewer::numberOfColumns() const {
    return this->m_header.size();
}

void TableViewer::setMaxVisibleRows(std::size_t count) {
    this->m_visibleRowOffset = 0;
    this->m_visibleRows = count;
}

void TableViewer::addRow() {
    this->m_rowContent.push_back(std::move(std::vector<std::string>(this->m_header.size())));
    this->m_rows.push_back(std::move(std::vector<Text>(this->m_header.size())));
}

void TableViewer::removeRow(std::size_t idx) {
    if (idx < this->m_rows.size()) {
        auto cit = this->m_rowContent.begin();
        std::advance(cit, idx);
        this->m_rowContent.erase(cit);

        auto it = this->m_rows.begin();
        std::advance(it, idx);
        this->m_rows.erase(it);
    }
}

void TableViewer::setElement(std::size_t rowIdx, std::size_t columnIdx, const std::string& element) {
    if (rowIdx < this->m_rows.size() && columnIdx < this->m_header.size()) {
        auto it = this->m_rows.begin();
        std::advance(it, rowIdx);
        (*it)[columnIdx].setFontColor(UiElement::foregroundColor());

        auto cit = this->m_rowContent.begin();
        std::advance(cit, rowIdx);
        (*cit)[columnIdx] = element;
    }
}

void TableViewer::setTextColor(std::size_t rowIdx, std::size_t columnIdx, const Gdiplus::Color& color) {
    if (rowIdx < this->m_rows.size() && columnIdx < this->m_header.size()) {
        auto it = this->m_rows.begin();
        std::advance(it, rowIdx);
        (*it)[columnIdx].setFontColor(color);
    }
}

const std::string& TableViewer::entry(std::size_t rowIdx, std::size_t columnIdx) const {
    if (rowIdx < this->m_rowContent.size() && columnIdx < this->m_header.size()) {
        auto it = this->m_rowContent.cbegin();
        std::advance(it, rowIdx);
        return (*it)[columnIdx];
    }

    static std::string __fallback;
    return __fallback;
}

bool TableViewer::click(const Gdiplus::PointF& pt, UiManager::MouseButton button) {
    if (UiManager::MouseButton::Left != button)
        return false;

    if (true == UiElement::isInRectangle(pt, this->m_scrollUp) && 0 != this->m_visibleRowOffset) {
        this->m_visibleRowOffset -= 1;
    }
    if (true == UiElement::isInRectangle(pt, this->m_scrollDown) && this->m_rows.size() > (this->m_visibleRowOffset + this->m_visibleRows)) {
        this->m_visibleRowOffset += 1;
    }
    else {
        auto retval = UiElement::isInRectangle(pt, this->m_area);

        if (true == retval) {
            float xPos = pt.X - this->m_area.X;
            float yPos = pt.Y - this->m_area.Y;

            /* clicked on the header*/
            if (yPos < this->m_rowHeight) {
                this->m_clickedRow = this->m_clickedColumn = std::numeric_limits<std::size_t>::max();
            }
            else {
                this->m_clickedRow = static_cast<std::size_t>((yPos - this->m_rowHeight) / this->m_rowHeight);
                this->m_clickedRow += this->m_visibleRowOffset;

                this->m_clickedColumn = 0;
                for (const auto& width : std::as_const(this->m_columnWidths)) {
                    if (xPos < width)
                        break;

                    this->m_clickedColumn += 1;
                    xPos -= width;
                }
            }
        }

        return retval;
    }

    return true;
}

bool TableViewer::clickedEntry(std::size_t& rowIdx, std::size_t& columnIdx) const {
    if (this->m_clickedRow >= this->m_rows.size() || this->m_clickedColumn >= this->m_header.size())
        return false;

    rowIdx = this->m_clickedRow;
    columnIdx = this->m_clickedColumn;

    return true;
}

float TableViewer::calculateRequiredArea(Gdiplus::Graphics* graphics) {
    /* contains the per-column widths */
    this->m_columnWidths = std::vector<float>(this->m_header.size(), 0.0f);
    std::size_t emptyLines = 0;

    /* get the widths of the header */
    for (std::size_t i = 0; i < this->m_header.size(); ++i) {
        this->m_header[i].setGraphics(graphics);
        this->m_header[i].setText(this->m_headerContent[i]);
        this->m_columnWidths[i] = std::max(this->m_columnWidths[i], this->m_header[i].rectangle().Width);
        this->m_rowHeight = std::max(this->m_rowHeight, this->m_header[i].rectangle().Height);
    }

    /* get the width of every row element */
    std::size_t maxRowCount = this->m_visibleRows;
    auto cit = this->m_rowContent.cbegin();
    for (auto it = this->m_rows.begin(); this->m_rows.end() != it; ++it) {
        bool emptyRow = true;

        for (std::size_t i = 0; i < it->size(); ++i) {
            (*it)[i].setGraphics(graphics);
            (*it)[i].setText((*cit)[i]);
            this->m_columnWidths[i] = std::max(this->m_columnWidths[i], (*it)[i].rectangle().Width);
            this->m_rowHeight = std::max(this->m_rowHeight, (*it)[i].rectangle().Height);
            if (false == helper::Math::almostEqual(0.0f, (*it)[i].rectangle().Width))
                emptyRow = false;
        }

        if (true == emptyRow)
            emptyLines += 1;
        else if (0 != maxRowCount)
            maxRowCount -= 1;

        std::advance(cit, 1);
    }

    /* get the overall width */
    float overallWidth = 0.0f;
    for (const auto& width : std::as_const(this->m_columnWidths))
        overallWidth += width;

    this->m_area = Gdiplus::RectF(this->m_area.X, this->m_area.Y, overallWidth + 12.0f + 3.0f * (this->m_header.size() - 1),
                                  (0 == maxRowCount ? (this->m_visibleRows + 1) : (this->m_rows.size() - emptyLines + 1)) * this->m_rowHeight);

    return overallWidth + 3.0f * (this->m_header.size() - 1);
}

void TableViewer::prepareVisualization(Gdiplus::Graphics* graphics) {
    this->calculateRequiredArea(graphics);
}

bool TableViewer::visualize(Gdiplus::Graphics* graphics) {
    float overallWidth = this->calculateRequiredArea(graphics);
    float offsetX = this->m_area.X;
    float offsetY = this->m_area.Y;

    /* draw the background of the table */
    Gdiplus::SolidBrush tableBrush(UiElement::backgroundColor());
    graphics->FillRectangle(&tableBrush, this->m_area);

    Gdiplus::Pen pen(UiElement::foregroundColor(), 1.0f);

    /* draw the header */
    for (std::size_t i = 0; i < this->m_header.size(); ++i) {
        this->m_header[i].setPosition(Gdiplus::PointF(offsetX, offsetY));
        this->m_header[i].setGraphics(graphics);
        this->m_header[i].visualize();
        offsetX += this->m_columnWidths[i] + 3.0f;
    }
    offsetY += this->m_rowHeight;
    graphics->DrawLine(&pen, Gdiplus::PointF(this->m_area.X, offsetY), Gdiplus::PointF(this->m_area.X + overallWidth, offsetY));

    /* draw the rows */
    auto it = this->m_rows.begin();
    std::advance(it, this->m_visibleRowOffset);
    std::size_t maxRowCount = this->m_visibleRows;
    while (this->m_rows.cend() != it) {
        offsetX = this->m_area.X;
        bool emptyRow = true;

        for (std::size_t i = 0; i < this->m_header.size(); ++i) {
            if (false == helper::Math::almostEqual(0.0f, (*it)[i].rectangle().Height)) {
                (*it)[i].setPosition(Gdiplus::PointF(offsetX, offsetY));
                (*it)[i].setGraphics(graphics);
                (*it)[i].visualize();
                emptyRow = false;
            }
            offsetX += this->m_columnWidths[i] + 3.0f;
        }

        if (false == emptyRow) {
            graphics->DrawLine(&pen, Gdiplus::PointF(this->m_area.X, offsetY), Gdiplus::PointF(this->m_area.X + overallWidth, offsetY));
            offsetY += this->m_rowHeight;
            maxRowCount -= 1;
        }

        if (0 == maxRowCount)
            break;

        std::advance(it, 1);
    }

    this->m_scrollUp = Gdiplus::RectF(offsetX, this->m_area.Y + 4.0f, 8.0f, 8.0f);
    this->m_scrollDown = Gdiplus::RectF(offsetX, this->m_area.Y + this->m_area.Height - 12.0f, 8.0f, 8.0f);

    /* draw the slider button */
    Gdiplus::SolidBrush sliderBrush(UiElement::foregroundColor());
    graphics->FillEllipse(&sliderBrush, this->m_scrollUp);
    graphics->FillEllipse(&sliderBrush, this->m_scrollDown);

    /* calculate the ratio of the slider */
    float ratio = 1.0f;
    float maxHeight = this->m_scrollDown.Y - 2.0f - (this->m_scrollUp.Y + this->m_scrollUp.Height + 2.0f);
    if (this->m_visibleRows < this->m_rows.size()) {
        ratio = static_cast<float>(this->m_visibleRows) / static_cast<float>(this->m_rows.size());

        /* calculate the dimensions and position of the slider */
        float height = ratio * maxHeight;
        float freePixels = maxHeight - height;
        float pixelsPerStep = freePixels / (this->m_rows.size() - this->m_visibleRows);

        this->m_sliderRectangle = Gdiplus::RectF(offsetX, this->m_scrollUp.Y + this->m_scrollUp.Height + 2.0f + pixelsPerStep * this->m_visibleRowOffset,
                                                 8.0f, height);
    }
    else {
        this->m_sliderRectangle = Gdiplus::RectF(offsetX, this->m_scrollUp.Y + this->m_scrollUp.Height + 2.0f,
                                                 8.0f, maxHeight);
    }

    /* draw the slider */
    graphics->FillRectangle(&sliderBrush, this->m_sliderRectangle);

    return true;
}