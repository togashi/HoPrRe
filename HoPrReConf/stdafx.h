// stdafx.h : �W���̃V�X�e�� �C���N���[�h �t�@�C���̃C���N���[�h �t�@�C���A�܂���
// �Q�Ɖ񐔂������A�����܂�ύX����Ȃ��A�v���W�F�N�g��p�̃C���N���[�h �t�@�C��
// ���L�q���܂��B

#pragma once

#ifndef STRICT
#define STRICT
#endif

#include "targetver.h"

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// �ꕔ�� CString �R���X�g���N�^�͖����I�ł��B

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