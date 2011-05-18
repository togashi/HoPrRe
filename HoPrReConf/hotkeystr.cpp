#include "stdafx.h"
#include "hotkeystr.h"

typedef struct VKNAME_ {
	UINT vk;
	LPCTSTR name;
	LPCTSTR idstr;
} VKNAME;

#define KEY(i, n)  { i, _T(n), _T(#i) }
#define L0	NULL

static const VKNAME VKNAMES[] = {
	KEY(VK_LBUTTON, NULL),
	KEY(VK_RBUTTON, NULL),
	KEY(VK_CANCEL, NULL),
	KEY(VK_MBUTTON, NULL),
	KEY(VK_XBUTTON1, NULL),
	KEY(VK_XBUTTON2, NULL),
	
	KEY(VK_BACK, "Backspace"),
	KEY(VK_TAB, "Tab"),
	KEY(VK_CLEAR, "Clear"),
	KEY(VK_RETURN, "Enter"),
	KEY(VK_SHIFT, "SHIFT"),
	KEY(VK_CONTROL, "CONTROL"),
	KEY(VK_MENU, "ALT"),
	KEY(VK_PAUSE, "Pause"),
	KEY(VK_CAPITAL, "Caps Lock"),
	KEY(VK_KANA, "�J�i"),
	KEY(VK_HANGEUL, NULL),
	KEY(VK_JUNJA, NULL),
	KEY(VK_FINAL, "�m��"),
	KEY(VK_KANJI, "����"),
	KEY(VK_HANJA, NULL),
	KEY(VK_ESCAPE, "Esc"),
	KEY(VK_CONVERT, "�ϊ�"),
	KEY(VK_NONCONVERT, "���ϊ�"),
	KEY(VK_ACCEPT, "�m��"),
	KEY(VK_MODECHANGE, NULL),
	KEY(VK_SPACE, "Space"),
	KEY(VK_PRIOR, "Page Up"),
	KEY(VK_NEXT, "Page Down"),
	KEY(VK_END, "End"),
	KEY(VK_HOME, "Home"),
	KEY(VK_LEFT, "��"),
	KEY(VK_UP, "��"),
	KEY(VK_RIGHT, "��"),
	KEY(VK_DOWN, "��"),
	KEY(VK_SELECT, "Select"),
	KEY(VK_PRINT, "Print"),
	KEY(VK_EXECUTE, NULL),
	KEY(VK_SNAPSHOT, "Print Screen"),
	KEY(VK_INSERT, "Insert"),
	KEY(VK_DELETE, "Delete"),
	KEY(VK_HELP, "Help"),
	
	KEY(VK_LWIN, NULL),
	KEY(VK_RWIN, NULL),
	KEY(VK_APPS, NULL),
	KEY(VK_SLEEP, NULL),

	KEY(VK_NUMPAD0, "�e���L�[0"),
	KEY(VK_NUMPAD1, "�e���L�[1"),
	KEY(VK_NUMPAD2, "�e���L�[2"),
	KEY(VK_NUMPAD3, "�e���L�[3"),
	KEY(VK_NUMPAD4, "�e���L�[4"),
	KEY(VK_NUMPAD5, "�e���L�[5"),
	KEY(VK_NUMPAD6, "�e���L�[6"),
	KEY(VK_NUMPAD7, "�e���L�[7"),
	KEY(VK_NUMPAD8, "�e���L�[8"),
	KEY(VK_NUMPAD9, "�e���L�[9"),
	KEY(VK_MULTIPLY, "�e���L�[*"),
	KEY(VK_ADD, "�e���L�[+"),
	KEY(VK_SEPARATOR, "�e���L�[,"),
	KEY(VK_SUBTRACT, "�e���L�[-"),
	KEY(VK_DECIMAL, "�e���L�[."),
	KEY(VK_DIVIDE, "�e���L�[/"),
	KEY(VK_F1, "F1"),
	KEY(VK_F2, "F2"),
	KEY(VK_F3, "F3"),
	KEY(VK_F4, "F4"),
	KEY(VK_F5, "F5"),
	KEY(VK_F6, "F6"),
	KEY(VK_F7, "F7"),
	KEY(VK_F8, "F8"),
	KEY(VK_F9, "F9"),
	KEY(VK_F10, "F10"),
	KEY(VK_F11, "F11"),
	KEY(VK_F12, "F12"),
	KEY(VK_F13, "F13"),
	KEY(VK_F14, "F14"),
	KEY(VK_F15, "F15"),
	KEY(VK_F16, "F16"),
	KEY(VK_F17, "F17"),
	KEY(VK_F18, "F18"),
	KEY(VK_F19, "F19"),
	KEY(VK_F20, "F20"),
	KEY(VK_F21, "F21"),
	KEY(VK_F22, "F22"),
	KEY(VK_F23, "F23"),
	KEY(VK_F24, "F24"),

	KEY(VK_NUMLOCK, "Num Lock"),
	KEY(VK_SCROLL, "Scroll Lock"),

	KEY(VK_OEM_NEC_EQUAL, NULL),
	KEY(VK_OEM_FJ_JISHO, NULL),
	KEY(VK_OEM_FJ_MASSHOU, NULL),
	KEY(VK_OEM_FJ_TOUROKU, NULL),
	KEY(VK_OEM_FJ_LOYA, NULL),
	KEY(VK_OEM_FJ_ROYA, NULL),
	KEY(VK_LSHIFT, NULL),
	KEY(VK_RSHIFT, NULL),
	KEY(VK_LCONTROL, NULL),
	KEY(VK_RCONTROL, NULL),
	KEY(VK_LMENU, NULL),
	KEY(VK_RMENU, NULL),
	
	KEY(VK_BROWSER_BACK, "�߂�"),
	KEY(VK_BROWSER_FORWARD, "�i��"),
	KEY(VK_BROWSER_REFRESH, "�X�V"),
	KEY(VK_BROWSER_STOP, "���~"),
	KEY(VK_BROWSER_SEARCH, "����"),
	KEY(VK_BROWSER_FAVORITES, "���C�ɓ���"),
	KEY(VK_BROWSER_HOME, "�z�[��"),

	KEY(VK_VOLUME_MUTE, "�~���[�g"),
	KEY(VK_VOLUME_DOWN, "���ʁ|"),
	KEY(VK_VOLUME_UP, "���ʁ{"),
	KEY(VK_MEDIA_NEXT_TRACK, "���g���b�N"),
	KEY(VK_MEDIA_PREV_TRACK, "�O�g���b�N"),
	KEY(VK_MEDIA_STOP, "��~"),
	KEY(VK_MEDIA_PLAY_PAUSE, "�Đ�/�ꎞ��~"),
	KEY(VK_LAUNCH_MAIL, "���[��"),
	KEY(VK_LAUNCH_MEDIA_SELECT, NULL),
	KEY(VK_LAUNCH_APP1, NULL),
	KEY(VK_LAUNCH_APP2, NULL),
	
	KEY(VK_OEM_1, NULL),
	KEY(VK_OEM_PLUS, "+"),
	KEY(VK_OEM_COMMA, ","),
	KEY(VK_OEM_MINUS, "-"),
	KEY(VK_OEM_PERIOD, "."),
	KEY(VK_OEM_2, NULL),
	KEY(VK_OEM_3, NULL),
	KEY(VK_OEM_4, NULL),
	KEY(VK_OEM_5, NULL),
	KEY(VK_OEM_6, NULL),
	KEY(VK_OEM_7, NULL),
	KEY(VK_OEM_8, NULL),
	KEY(VK_OEM_AX, NULL),
	KEY(VK_OEM_102, NULL),
	
	KEY(VK_ICO_HELP, NULL),
	KEY(VK_ICO_00, NULL),
	KEY(VK_PROCESSKEY, NULL),
	KEY(VK_ICO_CLEAR, NULL),
	KEY(VK_PACKET, NULL),
	
	KEY(VK_OEM_RESET, NULL),
	KEY(VK_OEM_JUMP, NULL),
	KEY(VK_OEM_PA1, NULL),
	KEY(VK_OEM_PA2, NULL),
	KEY(VK_OEM_PA3, NULL),
	KEY(VK_OEM_WSCTRL, NULL),
	KEY(VK_OEM_CUSEL, NULL),
	KEY(VK_OEM_ATTN, NULL),
	KEY(VK_OEM_FINISH, NULL),
	KEY(VK_OEM_COPY, NULL),
	KEY(VK_OEM_AUTO, NULL),
	KEY(VK_OEM_ENLW, NULL),
	KEY(VK_OEM_BACKTAB, NULL),
	KEY(VK_ATTN, NULL),
	KEY(VK_CRSEL, NULL),
	KEY(VK_EXSEL, NULL),
	KEY(VK_EREOF, NULL),
	KEY(VK_PLAY, NULL),
	KEY(VK_ZOOM, NULL),
	KEY(VK_NONAME, NULL),
	KEY(VK_PA1, NULL),
	KEY(VK_OEM_CLEAR, NULL),
	
	{ 0, NULL, NULL }
};

CString GetVKDisplayName(UINT vk)
{
	for (int i = 0; VKNAMES[i].idstr != NULL; i++) {
		if (vk == VKNAMES[i].vk) {
			return (VKNAMES[i].name != NULL) ? VKNAMES[i].name : VKNAMES[i].idstr;
		}
	}
	
	CString result;
	if ((vk >= 0x30 && vk <= 0x39) || (vk >= 0x41 && vk <= 0x5A)) {
		result.Format(_T("%c"), vk);
	} else if (vk != 0) {
		result.Format(_T("0x%x"), vk);
	} else {
		result.Empty();
	}
	return result;
}

CString GetVKID(UINT vk)
{
	for (int i = 0; VKNAMES[i].idstr != NULL; i++) {
		if (vk == VKNAMES[i].vk) {
			return VKNAMES[i].idstr;
		}
	}

	if ((vk >= _T('A') && vk <= _T('Z')) || (vk >= _T('0') && vk <= _T('9'))) {
		CString tmp;
		tmp.Format(_T("VK_%c"), vk);
		return tmp;
	}

	return NULL;
}

CString GetHotkeyString(UINT fsModifiers, UINT vk)
{
	CString res;
	if (fsModifiers & MOD_WIN) {
		if (!res.IsEmpty()) {
			res += _T(" + ");
		}
		res += _T("Win");
	}
	if (fsModifiers & MOD_CONTROL) {
		if (!res.IsEmpty()) {
			res += _T(" + ");
		}
		res += _T("Ctrl");
	}
	if (fsModifiers & MOD_ALT) {
		if (!res.IsEmpty()) {
			res += _T(" + ");
		}
		res += _T("Alt");
	}
	if (fsModifiers & MOD_SHIFT) {
		if (!res.IsEmpty()) {
			res += _T(" + ");
		}
		res += _T("Shift");
	}

	CString vkn = GetVKDisplayName(vk);
	if (!vkn.IsEmpty()) {
		if (!res.IsEmpty()) {
			res += _T(" + ");
		}
		res += vkn;
	} else {
		res.Empty();
	}
	
	return res;
}

UINT IdStrToVK(LPCTSTR lpszId)
{
	for (int i = 0; VKNAMES[i].idstr != NULL; i++) {
		if (::_tcscmp(VKNAMES[i].idstr, lpszId) == 0) {
			return VKNAMES[i].vk;
		}
	}
	
	if (::_tcslen(lpszId) == 4 && ::_tcsncmp(lpszId, _T("VK_"), 3) == 0) {
		TCHAR vkc = lpszId[3];
		if ((vkc >= _T('0') && vkc <= _T('9')) ||
			(vkc >= _T('A') && vkc <= _T('Z'))) {
			return static_cast<UINT>(vkc);
		}
	}

	return 0;
}

UINT NameStrToVK(LPCTSTR lpszName)
{
	for (int i = 0; VKNAMES[i].idstr != NULL; i++) {
		if (VKNAMES[i].name != NULL && ::_tcsicmp(VKNAMES[i].name, lpszName) == 0) {
			return VKNAMES[i].vk;
		}
	}
	
	return 0;
}

UINT NumStrToVK(LPCTSTR lpszNum)
{
	if (lpszNum != NULL && ::_tcslen(lpszNum) > 0) {
		int i;
		if (StrToIntEx(lpszNum, STIF_SUPPORT_HEX, &i)) {
			return i;
		}
	}
	
	return 0;
}

UINT StrToVK(LPCTSTR lpszStr, VKTYPE *vkt)
{
	UINT vk;

	vk = NumStrToVK(lpszStr);
	if (vk != 0) {
		if (vkt != NULL) {
			*vkt = VKT_NUMBER;
		}
		return vk;
	}

	vk = IdStrToVK(lpszStr);
	if (vk != 0) {
		if (vkt != NULL) {
			*vkt = VKT_SYMBOL;
		}
		return vk;
	}

	vk = NameStrToVK(lpszStr);
	if (vk != 0) {
		if (vkt != NULL) {
			*vkt = VKT_NAME;
		}
		return vk;
	}

	if (vkt != NULL) {
		*vkt = VKT_INVALID;
	}
	return 0;
}

