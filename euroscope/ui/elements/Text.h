/*
 * @brief Defines the base class for text visualizers
 * @file ui/elements/Text.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <memory>
#include <string>

#include <types/SystemConfiguration.h>

namespace topskytower {
    namespace euroscope {
        /**
         * @brief The text visualizer
         * @ingroup visualization
         */
        class Text {
        private:
            Gdiplus::Graphics*                   m_graphics;
            Gdiplus::RectF                       m_rectangle;
            std::wstring                         m_text;
            std::shared_ptr<Gdiplus::FontFamily> m_fontFamily;
            std::shared_ptr<Gdiplus::Font>       m_font;
            Gdiplus::Color                       m_fontColor;
            float                                m_fontSize;
            Gdiplus::PointF                      m_position;
            bool                                 m_bold;
            bool                                 m_italic;

            void updateFontFamily();
            void updateFont();

        public:
            /**
             * @brief Creates an uninitialized text visualization
             * @param[in] configuration The system configuration
             */
            Text();

            /**
             * @brief Checks if position is inside the visualized are
             * @param[in] position The requested screen position
             * @return True if position is inside the box, else false
             */
            bool isInBox(const Gdiplus::PointF& position) const;
            /**
             * @brief Returns the rectangle of the text
             * @return The rectangle
             */
            const Gdiplus::RectF& rectangle() const;
            /**
             * @brief Sets the graphics structure
             * @param[in] graphics The graphics structure
             */
            void setGraphics(Gdiplus::Graphics* graphics);
            /**
             * @brief Sets the text which will be visualized
             * @param[in] text The visualizable text
             */
            void setText(const std::string& text);
            /**
             * @brief Sets the visualization position
             * @param[in] position The new position to visualize the data
             */
            virtual void setPosition(const Gdiplus::PointF& position);
            /**
             * @brief Sets the font color
             * @param[in] color The new color
             */
            void setFontColor(const Gdiplus::Color& color);
            /**
             * @brief Sets the font size
             * @param[in] size The new size
             */
            void setFontSize(float size);
            /**
             * @brief Sets if the text is bold or not
             * @param[in] bold Marks if the text is bold or not
             */
            void setBold(bool bold);
            /**
             * @brief Sets if the text is italic or not
             * @param[in] italic Marks if the text is italic or not
             */
            void setItalic(bool italic);
            /**
             * @brief Visualizes the text
             */
            void visualize();
        };
    }
}
