#pragma once

#define CDEFREG_TYPE_RC01 0x31304352

enum {						// ����w��
	PRA_IG_RETURN_FALSE,	// �������� FALSE ��Ԃ�
	PRA_IG_RETURN_TRUE,		// �������� TRUE ��Ԃ�
	PRA_AU_RETURN_FALSE,	// �o�^�シ���������� FALSE ��Ԃ�
	PRA_AU_RETURN_TRUE		// �o�^�シ���������� TRUE ��Ԃ�
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

