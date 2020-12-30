/*
 * Author:
 *   Sven Czarnian <devel@svcz.de>
 * Brief:
 *   Implements the UI manager
 */

#include "stdafx.h"

#include "../RadarScreen.h"
#include "HoppiesLogonWindow.h"
#include "Toolbar.h"
#include "UiManager.h"

using namespace topskytower::euroscope;

UiManager::UiManager(RadarScreen* parent) :
        m_parent(parent),
        m_toolbar(nullptr),
        m_hoppies(nullptr),
        m_customWindows(),
        m_renderQueue() { }

UiManager::~UiManager() {
    this->m_renderQueue.clear();

    if (nullptr != this->m_toolbar) {
        delete this->m_toolbar;
        this->m_toolbar = nullptr;
    }

    if (nullptr != this->m_hoppies) {
        delete this->m_hoppies;
        this->m_hoppies = nullptr;
    }
}

void UiManager::activateUi(UiManager::WindowId id) {
    if (nullptr == this->m_parent)
        return;

    switch (id) {
    case UiManager::WindowId::PdcLogon:
        if (nullptr == this->m_hoppies)
            this->m_hoppies = new HoppiesLogonWindow(this->m_parent);
        this->m_hoppies->setActive(true);
        this->m_renderQueue.push_back(this->m_hoppies);
        break;
    default:
        break;
    }
}

void UiManager::updateRenderQueue(InsetWindow* element) {
    auto it = std::find(this->m_renderQueue.cbegin(), this->m_renderQueue.cend(), element);
    if (this->m_renderQueue.cend() != it) {
        this->m_renderQueue.erase(it);
        this->m_renderQueue.push_back(element);
    }
}

bool UiManager::click(InsetWindow* element, const Gdiplus::PointF& pt, UiManager::MouseButton button) {
    bool retval = element->click(pt, button);
    if (true == retval)
        this->updateRenderQueue(element);
    return retval;
}

bool UiManager::click(const std::string_view& objectName, const Gdiplus::PointF& pt, UiManager::MouseButton button) {
    if (nullptr == this->m_parent)
        return false;

    if ("Toolbar" == objectName) {
        return this->m_toolbar->click(pt, button);
    }
    else if ("PDC" == objectName) {
        return this->click(this->m_hoppies, pt, button);
    }
    else {
        auto it = this->m_customWindows.find(std::string(objectName));
        if (this->m_customWindows.end() != it)
            return this->click(it->second, pt, button);
    }

    return false;
}

bool UiManager::move(InsetWindow* element, const Gdiplus::PointF& pt, bool released) {
    bool retval = element->move(pt, released);
    if (true == retval)
        this->updateRenderQueue(element);
    return retval;
}

bool UiManager::move(const std::string_view& objectName, const Gdiplus::PointF& pt, bool released) {
    if (nullptr == this->m_parent)
        return false;

    if ("PDC" == objectName) {
        return this->move(this->m_hoppies, pt, released);
    }
    else {
        auto it = this->m_customWindows.find(std::string(objectName));
        if (this->m_customWindows.end() != it)
            return this->move(it->second, pt, released);
    }

    return false;
}

void UiManager::addCustomWindow(InsetWindow* window) {
    auto customIt = this->m_customWindows.find(window->title());
    if (this->m_customWindows.end() != customIt)
        this->removeCustomWindow(window);

    this->m_renderQueue.push_back(window);
    this->m_customWindows[window->title()] = window;
}

void UiManager::removeCustomWindow(InsetWindow* window) {
    auto renderIt = std::find(this->m_renderQueue.begin(), this->m_renderQueue.end(), window);
    if (this->m_renderQueue.end() != renderIt)
        this->m_renderQueue.erase(renderIt);

    auto customIt = this->m_customWindows.find(window->title());
    if (this->m_customWindows.end() != customIt)
        this->m_customWindows.erase(customIt);
}

void UiManager::resetClickStates() {
    this->m_toolbar->resetClickStates();
}

void UiManager::visualize(Gdiplus::Graphics* graphics) {
    if (nullptr == this->m_parent)
        return;

    if (nullptr == this->m_toolbar)
        this->m_toolbar = new Toolbar(this->m_parent, this);
    this->m_toolbar->visualize(graphics);

    for (auto& window : this->m_renderQueue)
        window->visualize(graphics);
}
