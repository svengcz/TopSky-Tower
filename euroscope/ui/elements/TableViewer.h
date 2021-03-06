/*
 * @brief Defines a viewer to visualize table information
 * @file ui/elements/TableViewer.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <string>
#include <vector>

#include "Text.h"
#include "UiElement.h"

namespace topskytower {
    namespace euroscope {
        class RadarScreen;

        /**
         * @brief Defines the table viewer
         * @ingroup euroscope
         */
        class TableViewer : public UiElement {
        private:
            struct Cell {
                Gdiplus::Color backgroundColor;
                Text           content;

                Cell() :
                        backgroundColor(UiElement::backgroundColor()),
                        content() { }
            };

            bool                                m_visualizeHeader;
            std::vector<std::string>            m_headerContent;
            std::vector<Text>                   m_header;
            std::list<std::vector<std::string>> m_rowContent;
            std::list<std::vector<Cell>>        m_rows;
            std::size_t                         m_minimumVisibleRows;
            std::size_t                         m_visibleRows;
            std::size_t                         m_visibleRowOffset;
            Gdiplus::RectF                      m_scrollUp;
            Gdiplus::RectF                      m_scrollDown;
            Gdiplus::RectF                      m_sliderRectangle;
            std::vector<float>                  m_minimumColumnWidths;
            std::vector<float>                  m_columnWidths;
            float                               m_rowHeight;
            std::size_t                         m_clickedRow;
            std::size_t                         m_clickedColumn;
            bool                                m_calculateArea;
            float                               m_overallWidth;

            bool calculateRequiredArea(Gdiplus::Graphics* graphics);

        public:
            /**
             * @brief Creates a new text viewer
             * @param[in] parent The parent RADAR screen
             * @param[in] header The header definition
             * @param[in] dimension The viewer's dimension
             */
            TableViewer(RadarScreen* parent, const std::vector<std::string>& header, const Gdiplus::RectF& dimension);
            /**
             * @brief Destroys the button
             */
            virtual ~TableViewer() { }

            /**
             * @brief Clears all rows
             */
            void clear();
            /**
             * @brief Returns the number of rows without the headline
             * @return The number of rows
             */
            std::size_t numberOfRows() const;
            /**
             * @brief Returns the number of columns
             * @return The column count
             */
            std::size_t numberOfColumns() const;
            /**
             * @brief Defines if the header is visualized or not
             * @param[in] visible True if the header needs to be visible, else false
             */
            void visualizeHeader(bool visible);
            /**
             * @brief Sets the minimum number of visible rows
             * @param[in] count The number of minimum visible rows
             */
            void setMinVisibleRows(std::size_t count);
            /**
             * @brief Sets the maximum number of visible rows
             * @param[in] count The number of maximum visible rows
             */
            void setMaxVisibleRows(std::size_t count);
            /**
             * @brief Define the minimum widths of the columns
             * @param[in] widths All widths for the columns
             */
            void setMinimumColumnWidths(std::initializer_list<float> widths);
            /**
             * @brief Defines the width for a specific column
             * @param[in] idx The column's index
             * @param[in] width The new minimum width
             */
            void setMinimumColumnWidth(std::size_t idx, float width);
            /**
             * @brief Adds an empty row
             */
            void addRow();
            /**
             * @brief Removes a row
             * @param[in] idx The row's index
             */
            void removeRow(std::size_t idx);
            /**
             * @brief Sets the element in the table
             * @param[in] rowIdx The row index
             * @param[in] columnIdx The column index
             * @param[in] element The new content
             */
            void setElement(std::size_t rowIdx, std::size_t columnIdx, const std::string& element);
            /**
             * @brief Sets the font color for a specific element
             * @param[in] rowIdx The row index
             * @param[in] columnIdx The column index
             * @param[in] color The new color
             */
            void setTextColor(std::size_t rowIdx, std::size_t columnIdx, const Gdiplus::Color& color);
            /**
             * @brief Sets the background color for a specific element
             * @param[in] rowIdx The row index
             * @param[in] columnIdx The column index
             * @param[in] color The new color
             */
            void setBackgroundColor(std::size_t rowIdx, std::size_t columnIdx, const Gdiplus::Color& color);
            /**
             * @brief Returns the content of a cell
             * @param[in] rowIdx The row index
             * @param[in] columnIdx The column index
             * @return The content of the cell
             */
            const std::string& entry(std::size_t rowIdx, std::size_t columnIdx) const;
            /**
             * @brief Returns the background color of a cell
             * @param[in] rowIdx The row index
             * @param[in] columnIdx The column index
             * @return The background color of the cell
             */
            const Gdiplus::Color& backgroundColor(std::size_t rowIdx, std::size_t columnIdx) const;
            /**
             * @brief Handles the click events
             * @param[in] pt The position of the mouse
             * @param[in] button The clicked mouse button
             * @return True if the click was handled, else false
             */
            bool click(const Gdiplus::PointF& pt, UiManager::MouseButton button) override;
            /**
             * @brief Returns the clicked entry
             * @param[out] rowIdx The clicked row index
             * @param[out] columnIdx The clicked column index
             * @return True if it was clicked, else false
             */
            bool clickedEntry(std::size_t& rowIdx, std::size_t& columnIdx) const;
            /**
             * @brief prepares the visualization data
             * @param[in] graphics The graphic-interface
             */
            void prepareVisualization(Gdiplus::Graphics* graphics);
            /**
             * @brief Visualizes the toolbar with all active sub-menus
             * @param[in] graphics The graphics container
             * @return True if the element was visualized, else false
             */
            bool visualize(Gdiplus::Graphics* graphics) override;
        };
    }
}
