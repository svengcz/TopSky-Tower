/*
 * @brief Defines a viewer to visualize table information
 * @file ui/elements/TableViewer.h
 * @author Sven Czarnian <devel@svcz.de>
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
            std::vector<std::string>            m_headerContent;
            std::vector<Text>                   m_header;
            std::list<std::vector<std::string>> m_rowContent;
            std::list<std::vector<Text>>        m_rows;

            float calculateRequiredArea(std::vector<float>& columnWidths, Gdiplus::Graphics* graphics, float& height);

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
             * @brief Returns the content of a cell
             * @param[in] rowIdx The row index
             * @param[in] columnIdx The column index
             * @return The content of the cell
             */
            const std::string& entry(std::size_t rowIdx, std::size_t columnIdx) const;
            /**
             * @brief Handles the click events
             * @param[in] pt The position of the mouse
             * @param[in] button The clicked mouse button
             * @return True if the click was handled, else false
             */
            bool click(const Gdiplus::PointF& pt, UiManager::MouseButton button) override;
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
