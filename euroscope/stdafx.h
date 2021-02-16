/*
 * @brief Defines the include header for all euroscope files
 * @file euroscope/stdafx.h
 * @author Sven Czarnian <devel@svcz.de>
 * @copyright Copyright 2020-2021 Sven Czarnian
 * @license This project is published under the GNU General Public License v3 (GPLv3)
 */

#pragma once

#include <algorithm>

#include "res/targetver.h"

#define NOMINMAX
#include <Windows.h>
using std::min;
using std::max;

#include <version.h>

#pragma warning(push, 0)
#include <gdiplus.h>
#pragma warning(pop)
