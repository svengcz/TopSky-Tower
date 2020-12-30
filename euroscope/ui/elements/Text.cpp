/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the text visualization
 */

#include "stdafx.h"

#include <surveillance/ConfigurationRegistry.h>

#include "Text.h"

using namespace topskytower;
using namespace topskytower::euroscope;

static bool __fontErrorPrinted = false;

Text::Text() :
        m_graphics(nullptr),
        m_rectangle(),
        m_text(),
        m_fontFamily(new Gdiplus::FontFamily(std::wstring(surveillance::ConfigurationRegistry::instance().systemConfiguration().fontFamily.begin(),
                                                          surveillance::ConfigurationRegistry::instance().systemConfiguration().fontFamily.end()).c_str(), nullptr)),
        m_font(nullptr),
        m_fontColor(),
        m_fontSize(surveillance::ConfigurationRegistry::instance().systemConfiguration().fontSize),
        m_position(),
        m_bold(false),
        m_italic(false) { }

bool Text::isInBox(const Gdiplus::PointF& position) const {
    if (position.X < this->m_rectangle.GetLeft() || position.X > this->m_rectangle.GetRight())
        return false;
    if (position.Y < this->m_rectangle.GetTop() || position.Y > this->m_rectangle.GetBottom())
        return false;
    return true;
}

const Gdiplus::RectF& Text::rectangle() const {
    return this->m_rectangle;
}

void Text::updateFont() {
    /* set the correct font style */
    Gdiplus::FontStyle style;
    if (true == this->m_bold && true == this->m_italic)
        style = Gdiplus::FontStyleBoldItalic;
    else if (true == this->m_bold)
        style = Gdiplus::FontStyleBold;
    else if (true == this->m_italic)
        style = Gdiplus::FontStyleItalic;
    else
        style = Gdiplus::FontStyleRegular;

    /* set the font */
    this->m_font = std::shared_ptr<Gdiplus::Font>(new Gdiplus::Font(this->m_fontFamily.get(),
                                                                    static_cast<Gdiplus::REAL>(this->m_fontSize),
                                                                    style, Gdiplus::UnitMillimeter));

    /* check if the initialization was successful */
    if (false == __fontErrorPrinted && Gdiplus::Status::Ok != this->m_font->GetLastStatus())
        __fontErrorPrinted = true;
}

void Text::setGraphics(Gdiplus::Graphics* graphics) {
    this->m_graphics = graphics;
}

void Text::setText(const std::string& text) {
    std::wstring newText(text.cbegin(), text.cend());

    /* no update needed if we need to draw the same text */
    if (this->m_text == newText)
        return;

    if (nullptr == this->m_graphics)
        return;

    if (nullptr == this->m_font)
        this->updateFont();

    Gdiplus::StringFormat format;
    this->m_text = std::move(newText);
    this->m_graphics->MeasureString(this->m_text.c_str(), static_cast<INT>(wcslen(this->m_text.c_str())), this->m_font.get(),
                                    Gdiplus::PointF(0.0f, 0.0f), &format, &this->m_rectangle);
}

void Text::setPosition(const Gdiplus::PointF& position) {
    this->m_position = position;
}

void Text::setFontColor(const Gdiplus::Color& color) {
    this->m_fontColor = color;
}

void Text::setFontSize(float size) {
    this->m_fontSize = size;
}

void Text::setBold(bool bold) {
    this->m_bold = bold;
}

void Text::setItalic(bool italic) {
    this->m_italic = italic;
}

void Text::visualize() {
    if (nullptr == this->m_graphics)
        return;

    if (nullptr == this->m_font)
        this->updateFont();

    if (0 != this->m_text.length()) {
        Gdiplus::SolidBrush brush(this->m_fontColor);
        this->m_graphics->DrawString(this->m_text.c_str(), static_cast<INT>(wcslen(this->m_text.c_str())),
                                     this->m_font.get(), this->m_position, &brush);
    }

    /* update the rectangle */
    this->m_rectangle.X = this->m_position.X;
    this->m_rectangle.Y = this->m_position.Y;
}
