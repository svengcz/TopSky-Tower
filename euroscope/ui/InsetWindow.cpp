/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the inset window of the system
 */

#include "stdafx.h"

#include <Eigen/Core>

#include "../RadarScreen.h"
#include "InsetWindow.h"

using namespace topskytower::euroscope;

#define HEADER_HEIGHT     13.0f
#define RESIZE_HEIGHT     7.0f
#define CLICK_ROI_SPACING 10.0f
#define RESIZE_BTN_SIZE   5.0f

InsetWindow::InsetWindow(const std::string& title, RadarScreen* parent, const Gdiplus::RectF& rectangle,
                         bool resizable, bool registerInManager) :
        UiElement(parent, rectangle),
        m_active(false),
        m_resizable(resizable),
        m_resizeActive(false),
        m_registerInManager(registerInManager),
        m_title(title),
        m_titleVisualization(),
        m_headlineRectangle(),
        m_crossRectangle(),
        m_resizeRectangle(),
        m_offsetToCenter(NAN, NAN),
        m_minimumSize(),
        m_contentArea(rectangle),
        m_elements() {
    this->m_titleVisualization.setBold(true);
    this->m_titleVisualization.setFontColor(UiElement::foregroundColor());

    this->m_contentArea.GetSize(&this->m_minimumSize);

    this->m_area.X = this->m_contentArea.X;
    this->m_area.Y = this->m_contentArea.Y - HEADER_HEIGHT;
    this->m_area.Width = this->m_contentArea.Width;
    this->m_area.Height = this->m_contentArea.Height + HEADER_HEIGHT;
    if (true == this->m_resizable)
        this->m_area.Height += RESIZE_HEIGHT;
}

InsetWindow::~InsetWindow() {
    for (auto& element : this->m_elements)
        delete element;
    this->m_elements.clear();
}

const std::string& InsetWindow::title() const {
    return this->m_title;
}

void InsetWindow::setContentSize(const Gdiplus::SizeF size) {
    /* update the dimension */
    this->m_contentArea.Width = size.Width;
    this->m_contentArea.Height = size.Height;

    /* calculate the overall dimension */
    this->m_area.Width = size.Width;
    this->m_area.Height = size.Height + HEADER_HEIGHT;
    if (true == this->m_resizable)
        this->m_area.Height += RESIZE_HEIGHT;
}

void InsetWindow::resize() { }

void InsetWindow::setActive(bool active) {
    this->m_active = active;

    if (true == this->m_registerInManager) {
        if (false == active) {
            this->m_parent->uiManager().removeCustomWindow(this);
            delete this;
        }
        else {
            this->m_parent->uiManager().addCustomWindow(this);
        }
    }
}

bool InsetWindow::isActive() const {
    return this->m_active;
}

void InsetWindow::move(const Gdiplus::PointF& direction) {
    (void)direction;
}

void InsetWindow::setPosition(const Gdiplus::PointF& position) {
    this->m_area.X = position.X;
    this->m_area.Y = position.Y;
    this->m_contentArea.X = position.X;
    this->m_contentArea.Y = position.Y + HEADER_HEIGHT;
}

bool InsetWindow::click(const Gdiplus::PointF& pt, UiManager::MouseButton button) {
    if (UiManager::MouseButton::Left == button) {
        /* check if the cross is clicked */
        if (true == UiElement::isInRectangle(pt, this->m_crossRectangle)) {
            this->setActive(false);
            return true;
        }
        /* check if the headline is clicked */
        else if (true == UiElement::isInRectangle(pt, this->m_headlineRectangle) || true == UiElement::isInRectangle(pt, this->m_titleVisualization.rectangle())) {
            return true;
        }
    }

    /* check the children */
    for (auto& element : this->m_elements) {
        if (true == element->click(pt, button))
            return true;
    }

    return false;
}

static __inline Eigen::Vector2f __centerOfRectangle(const Gdiplus::RectF& rectangle) {
    return Eigen::Vector2f(
        rectangle.GetLeft() + rectangle.Width * 0.5f,
        rectangle.GetTop() + rectangle.Height
    );
}

bool InsetWindow::move(const Gdiplus::PointF& pt, bool released) {
    /* we do not have to move the screen */
    if (true == released) {
        this->m_offsetToCenter = { NAN, NAN };
        return false;
    }

    /* check if the move is valid in the headline */
    if (true == std::isnan(this->m_offsetToCenter.X)) {
        /* use a bigger ROI, because the function is called after the first moves */
        Gdiplus::RectF headlineMoveBox(
            this->m_headlineRectangle.X - CLICK_ROI_SPACING,
            this->m_headlineRectangle.Y - CLICK_ROI_SPACING,
            this->m_headlineRectangle.Width + CLICK_ROI_SPACING * 2.0f,
            this->m_headlineRectangle.Height + CLICK_ROI_SPACING * 2.0f
        );

        /* check if the move is called next to the headline */
        if (true == UiElement::isInRectangle(pt, headlineMoveBox)) {
            auto center = __centerOfRectangle(this->m_headlineRectangle);
            this->m_offsetToCenter = { pt.X - center[0], pt.Y - center[1] };
            this->m_resizeActive = false;
        }
        else {
            /* check if the resize-box is moved, if the move is called next to the resize button */
            if (true == this->m_resizable) {
                /* use a bigger ROI, because the function is called after the first moves */
                Gdiplus::RectF resizeMoveBox(
                    this->m_resizeRectangle.X - CLICK_ROI_SPACING,
                    this->m_resizeRectangle.Y - CLICK_ROI_SPACING,
                    this->m_resizeRectangle.Width + CLICK_ROI_SPACING * 2.0f,
                    this->m_resizeRectangle.Height + CLICK_ROI_SPACING * 2.0f
                );

                if (true == UiElement::isInRectangle(pt, resizeMoveBox)) {
                    auto center = __centerOfRectangle(this->m_resizeRectangle);
                    this->m_offsetToCenter = { pt.X - center[0], pt.Y - center[1] };
                    this->m_resizeActive = true;
                }
                else {
                    return false;
                }
            }
            else {
                return false;
            }
        }
    }

    /* calculate the correct center */
    Eigen::Vector2f boxCenter;
    if (true == this->m_resizeActive)
        boxCenter = __centerOfRectangle(this->m_resizeRectangle);
    else
        boxCenter = __centerOfRectangle(this->m_headlineRectangle);

    /* calculate the direction of the movement */
    Gdiplus::PointF direction((pt.X - boxCenter[0]) - this->m_offsetToCenter.X, (pt.Y - boxCenter[1]) - this->m_offsetToCenter.Y);

    /* check if we can move the window */
    auto area = this->m_parent->GetRadarArea();
    if (area.left > static_cast<int>(this->m_area.GetLeft() + direction.X))
        direction.X = area.left - this->m_area.GetLeft();
    else if (area.right < static_cast<int>(this->m_area.GetRight() + direction.X))
        direction.X = area.right - this->m_area.GetRight();
    if (area.top > static_cast<int>(this->m_area.GetTop() + direction.Y))
        direction.Y = area.top - this->m_area.GetTop();
    else if (area.bottom < static_cast<int>(this->m_area.GetBottom() + direction.Y))
        direction.Y = area.bottom - this->m_area.GetBottom();

    /* move the window and all elements */
    if (false == this->m_resizeActive) {
        this->m_area.X += direction.X;
        this->m_area.Y += direction.Y;
        this->m_contentArea.X += direction.X;
        this->m_contentArea.Y += direction.Y;
        for (auto& element : this->m_elements)
            element->move(direction);
    }
    /* calculate the new width and height */
    else {
        /* check for the elements if we shrinked it down to the minimum */
        if (this->m_contentArea.Width + direction.X <= this->m_minimumSize.Width)
            direction.X = this->m_minimumSize.Width - this->m_contentArea.Width;
        if (this->m_contentArea.Height + direction.Y <= this->m_minimumSize.Height)
            direction.Y = this->m_minimumSize.Height - this->m_contentArea.Height;

        this->m_area.Width += direction.X;
        this->m_area.Height += direction.Y;
        this->m_contentArea.Width += direction.X;
        this->m_contentArea.Height += direction.Y;

        this->resize();
    }

    return true;
}

bool InsetWindow::visualize(Gdiplus::Graphics* graphics) {
    /* no need to render the window */
    if (false == this->m_active)
        return false;

    /* get the relevant colors */
    auto bgColor = UiElement::backgroundColor();
    auto fgColor = UiElement::foregroundColor();

    /* draw the background */
    Gdiplus::SolidBrush bgGrush(bgColor);
    graphics->FillRectangle(&bgGrush, this->m_area);

    /* draw the border */
    Gdiplus::Pen pen(fgColor, 1.0f);
    graphics->DrawRectangle(&pen, this->m_area);

    /* calculate the rectangle of the headline */
    this->m_headlineRectangle = this->m_area;
    this->m_headlineRectangle.Y += 3.0f;
    this->m_headlineRectangle.Width -= HEADER_HEIGHT;
    this->m_headlineRectangle.Height = HEADER_HEIGHT - 6.0f;

    /* calculate the rectangle of the cross */
    this->m_crossRectangle.X = this->m_headlineRectangle.X + this->m_headlineRectangle.Width + 3.0f;
    this->m_crossRectangle.Y = this->m_headlineRectangle.Y;
    this->m_crossRectangle.Width = this->m_crossRectangle.Height = HEADER_HEIGHT - 6.0f;

    /* calculate the rectangle of the resize-button */
    if (true == this->m_resizable) {
        this->m_resizeRectangle.X = this->m_area.GetRight() - (RESIZE_BTN_SIZE);
        this->m_resizeRectangle.Y = this->m_area.GetBottom() - (RESIZE_BTN_SIZE);
        this->m_resizeRectangle.Width = RESIZE_BTN_SIZE;
        this->m_resizeRectangle.Height = RESIZE_BTN_SIZE;
    }

    /* draw the title */
    this->m_titleVisualization.setGraphics(graphics);
    this->m_titleVisualization.setText(this->m_title);
    this->m_titleVisualization.setPosition(Gdiplus::PointF(this->m_headlineRectangle.X, this->m_headlineRectangle.Y - 3.0f));
    this->m_titleVisualization.visualize();

    /* draw the headline */
    Gdiplus::SolidBrush fgBrush(fgColor);
    Gdiplus::RectF headlineRectangle(this->m_headlineRectangle);
    headlineRectangle.X += this->m_titleVisualization.rectangle().Width;
    headlineRectangle.Width -= this->m_titleVisualization.rectangle().Width;
    graphics->FillRectangle(&fgBrush, headlineRectangle);

    /* draw the closing circle */
    graphics->FillEllipse(&fgBrush, this->m_crossRectangle);

    /* draw the resize edge */
    if (true == this->m_resizable) {
        graphics->FillRectangle(&fgBrush, this->m_resizeRectangle);
        Gdiplus::RectF bottom(this->m_area.GetLeft(), this->m_area.GetBottom() - RESIZE_BTN_SIZE, this->m_area.Width - 14.0f, RESIZE_BTN_SIZE);
        graphics->FillRectangle(&fgBrush, bottom);
    }

    /* register the area in the RADAR screen */
    RECT rect = {
        static_cast<int>(this->m_area.GetLeft()),
        static_cast<int>(this->m_area.GetTop()),
        static_cast<int>(this->m_area.GetRight()),
        static_cast<int>(this->m_area.GetBottom())
    };
    this->m_parent->AddScreenObject(static_cast<int>(RadarScreen::ClickId::UserWindow), this->m_title.c_str(), rect, true, "");

    /* draw the elements */
    for (auto& element : this->m_elements)
        element->visualize(graphics);

    return true;
}
