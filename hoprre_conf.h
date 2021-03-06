#pragma once

#define CDEFREG_TYPE_RC01 0x31304352

enum {						// 動作指定
	PRA_IG_RETURN_FALSE,	// 無視して FALSE を返す
	PRA_IG_RETURN_TRUE,		// 無視して TRUE を返す
	PRA_AU_RETURN_FALSE,	// 登録後すぐ解除して FALSE を返す
	PRA_AU_RETURN_TRUE		// 登録後すぐ解除して TRUE を返す
};

typedef struct HOPRRE_DAT_ {
	INT id;
	UINT fsModifiers;
	UINT vk;
	DWORD dwAction;
} HOPRRE_DAT;

#define DEFMODNAMELEN 16

typedef struct CDEFREG_ {
	DWORD dwType;
	HOPRRE_DAT c;
	WCHAR szModule[DEFMODNAMELEN];
} CDEFREG, *LPCDEFREG;

#define SIZEOF_CDEFREG(s) (sizeof(CDEFREG) - sizeof(WCHAR) * DEFMODNAMELEN + ((::_tcslen(s) + 1 >= 16) ? ::_tcslen(s) + 1 : 16) * sizeof(TCHAR))

