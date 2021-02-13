/*
 * @brief Defines the hidden window callback function
 * @file euroscope/HiddenWindow.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <Windows.h>

LRESULT CALLBACK HiddenWindow(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
