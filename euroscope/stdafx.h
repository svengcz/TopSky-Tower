// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once

#include <algorithm>

#include <targetver.h>

#define NOMINMAX
#include <Windows.h>
using std::min;
using std::max;

#include <version.h>

#pragma warning(push, 0)
#include <gdiplus.h>
#pragma warning(pop)
