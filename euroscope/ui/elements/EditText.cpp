/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the edit text
 * Copyright:
 *   2020-2021 Sven Czarnian
 * License:
 *   GNU General Public License v3 (GPLv3)
 */

#include "stdafx.h"

#include "../../PlugIn.h"
#include "../../RadarScreen.h"
#include "EditText.h"

using namespace topskytower::euroscope;

EditText::EditText(RadarScreen* parent, const std::string& headline, const Gdiplus::RectF& dimension) :
        UiElement(parent, dimension),
        m_headline(headline),
        m_content(),
        m_editable(true),
        m_passwordField(false),
        m_headlineVisualization(),
        m_contentVisualization(),
        m_editFieldRectangle(dimension) {
    /* define the headline */
    this->m_headlineVisualization.setFontColor(UiElement::foregroundColor());

    /* define the content */
    this->m_contentVisualization.setFontColor(UiElement::foregroundColor());
}

void EditText::setPasswordField(bool passwordField) {
    this->m_passwordField = passwordField;
}

void EditText::setContent(const std::string& content) {
    this->m_content = content;
}

const std::string& EditText::content() const {
    return this->m_content;
}

void EditText::setEditable(bool editable) {
    this->m_editable = editable;
}

void EditText::setPosition(const Gdiplus::PointF& position) {
    UiElement::setPosition(position);

    /* set the new overall position */
    this->m_editFieldRectangle.X = position.X;
    this->m_editFieldRectangle.Y = position.Y;
}

void EditText::move(const Gdiplus::PointF& direction) {
    UiElement::move(direction);

    /* move the complete dimension */
    this->m_editFieldRectangle.X += direction.X;
    this->m_editFieldRectangle.Y += direction.Y;
}

void EditText::uiCallback(const std::string& string) {
    this->m_content = string;
}

bool EditText::click(const Gdiplus::PointF& pt, UiManager::MouseButton button) {
    /* ignore non-left button clicks */
    if (UiManager::MouseButton::Left != button || false == this->m_editable)
        return false;

    /* check if we are clicked */
    if (true == UiElement::isInRectangle(pt, this->m_editFieldRectangle)) {
        auto plugin = static_cast<PlugIn*>(this->m_parent->GetPlugIn());
        plugin->registerUiCallback(this, &EditText::uiCallback);

        /* calculate the box of the edit text */
        POINT point{ static_cast<int>(pt.X), static_cast<int>(pt.Y) };
        RECT rect{
            point.x, point.y,
            static_cast<int>(this->m_editFieldRectangle.GetRight() + 0.5f),
            static_cast<int>(this->m_editFieldRectangle.GetBottom() + 0.5f)
        };

        plugin->OnFunctionCall(static_cast<int>(PlugIn::TagItemFunction::UiEditTextRequest),
                               this->m_content.c_str(), point, rect);
    }

    return false;
}

bool EditText::visualize(Gdiplus::Graphics* graphics) {
    /* prepare the headline */
    this->m_headlineVisualization.setGraphics(graphics);
    this->m_headlineVisualization.setText(this->m_headline);

    /* prepare the content */
    this->m_contentVisualization.setGraphics(graphics);
    if (0 != this->m_content.length() && true == this->m_passwordField) {
        std::string hidden(this->m_content.length(), '*');
        this->m_contentVisualization.setText(hidden);
    }
    else {
        this->m_contentVisualization.setText(this->m_content);
    }

    /* draw the box */
    Gdiplus::Pen pen(UiElement::foregroundColor(), 1.0f);
    graphics->DrawRectangle(&pen, this->m_editFieldRectangle);

    /* draw the headline */
    Gdiplus::PointF position(this->m_editFieldRectangle.X + 2.0f, this->m_editFieldRectangle.Y - 2.0f - this->m_headlineVisualization.rectangle().Height * 0.8f);
    this->m_headlineVisualization.setPosition(position);
    this->m_headlineVisualization.visualize();

    /* initialize the overall area */
    this->m_area = this->m_headlineVisualization.rectangle();

    /* draw the content */
    position = Gdiplus::PointF(this->m_editFieldRectangle.X + 2.0f, this->m_editFieldRectangle.Y + 2.0f);
    this->m_contentVisualization.setPosition(position);
    this->m_contentVisualization.visualize();

    /* finalize the overall area */
    this->m_area.X = min(this->m_area.X, this->m_contentVisualization.rectangle().X);
    this->m_area.Y = min(this->m_area.Y, this->m_contentVisualization.rectangle().Y);
    this->m_area.Width = max(this->m_area.Width, this->m_contentVisualization.rectangle().Width);
    this->m_area.Height += this->m_contentVisualization.rectangle().Y - this->m_area.Y - this->m_area.Height + this->m_contentVisualization.rectangle().Height;

    return true;
}
