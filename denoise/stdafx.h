// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define STRICT
#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_NO_OPENGL
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _WTL_NO_CSTRING
#define _WTL_NO_WTYPES
#define _WTL_NEW_UXTHEME

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <stdint.h>

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <memory>
#include <experimental/filesystem>

namespace Gdiplus
{
	using std::min;
	using std::max;
}

#include <atlbase.h>
#include <atlstr.h>
#include <atlimage.h>

#include <wincodec.h>