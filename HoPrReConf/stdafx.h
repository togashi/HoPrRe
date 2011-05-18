// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。

#pragma once

#ifndef STRICT
#define STRICT
#endif

#include "targetver.h"

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// 一部の CString コンストラクタは明示的です。

#define _WTL_NO_CSTRING

#include "resource.h"
#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>

//#include <atlwin.h>
//#include <atlhost.h>
#include <atlcrack.h>
#include <atlapp.h>
	// atlframe.h
#include <atlframe.h>
	// CDialogResize
#include <atlctrls.h>
	// CListViewCtrl
#include <atlmisc.h>
	// CPoint
#include <atlstr.h>
	// CString
#include <atlalloc.h>
	// CTempBuffer

using namespace ATL;

extern HMODULE g_hModule;