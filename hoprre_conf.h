#pragma once

#define CDEFREG_TYPE_RC01 0x31304352

enum {						// “®ìŽw’è
	PRA_IG_RETURN_FALSE,	// –³Ž‹‚µ‚Ä FALSE ‚ð•Ô‚·
	PRA_IG_RETURN_TRUE,		// –³Ž‹‚µ‚Ä TRUE ‚ð•Ô‚·
	PRA_AU_RETURN_FALSE,	// “o˜^Œã‚·‚®‰ðœ‚µ‚Ä FALSE ‚ð•Ô‚·
	PRA_AU_RETURN_TRUE		// “o˜^Œã‚·‚®‰ðœ‚µ‚Ä TRUE ‚ð•Ô‚·
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

