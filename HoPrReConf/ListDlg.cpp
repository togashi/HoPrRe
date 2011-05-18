#include "StdAfx.h"
#include "ListDlg.h"
#include "../hoprre_conf.h"
#include "hotkeystr.h"
#include "editdlg.h"

#define VNAMELEN 80

class CSetRedraw {
private:
	HWND m_hWnd;
public:
	CSetRedraw(HWND hWnd): m_hWnd(hWnd) {
		::SendMessage(m_hWnd, WM_SETREDRAW, 0, 0);
	}
	~CSetRedraw() {
		::SendMessage(m_hWnd, WM_SETREDRAW, 1, 0);
	}
};

CListDlg::CListDlg(LPCTSTR lpszRegValueNameFormat)
{
	CTempBuffer<TCHAR> buf;
	{
		size_t l = ::_tcslen(lpszRegValueNameFormat);
		buf.Allocate(l + 1);
		::_tcscpy_s(buf, l + 1, lpszRegValueNameFormat);
	}
	
	LPTSTR vname = ::_tcsrchr(buf, _T('\\'));
	if (vname != NULL) {
		m_RegValueNameFormat = vname + 1;
		*vname = _T('\0');
	}

	LPTSTR kpath = ::_tcschr(buf, _T('\\'));
	if (kpath != NULL) {
		m_RegKeyPath = kpath + 1;
		*kpath = _T('\0');
	}
	
	if (::_tcsicmp(buf, _T("HKEY_LOCAL_MACHINE")) == 0 || ::_tcsicmp(buf, _T("HKLM")) == 0) {
		m_hRootKey = HKEY_LOCAL_MACHINE;
	} else if (::_tcsicmp(buf, _T("HKEY_CURRENT_USER")) == 0 || ::_tcsicmp(buf, _T("HKCU")) == 0) {
		m_hRootKey = HKEY_CURRENT_USER;
	}
}

CListDlg::~CListDlg(void)
{
}

LRESULT CListDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	DlgResize_Init(true, true);
	
	SetIcon(::LoadIcon(_AtlBaseModule.GetModuleInstance(), MAKEINTRESOURCE(IDI_ICON1)), FALSE);

	m_List.Attach(GetDlgItem(IDC_LIST));
	m_List.InsertColumn(1, _T("ホットキー"), LVCFMT_LEFT, 200, -1);
	m_List.InsertColumn(2, _T("モジュール名"), LVCFMT_LEFT, 200, 0);
	m_List.InsertColumn(3, _T("設定名"), LVCFMT_LEFT, 200, 1);
	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);
	UpdateList();
	return 1;
}

void CListDlg::OnSysCommand(UINT sc, CPoint &pos)
{
	if (sc == SC_CLOSE) {
		PostMessage(WM_COMMAND, MAKELONG(IDCLOSE, BN_CLICKED), NULL);
	}
	SetMsgHandled(FALSE);
}

void CListDlg::OnCmdClose(UINT nCode, int nID, HWND hWndCtrl)
{
	EndDialog(IDOK);
}

void CListDlg::UpdateList(void)
{
	CSetRedraw redraw(m_List);
	m_List.DeleteAllItems();
	
	HKEY hKey;
	int count = 0;
	if (::RegOpenKeyEx(m_hRootKey, m_RegKeyPath, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		TCHAR vname[VNAMELEN];
		DWORD vnlen;
		DWORD vtype;
		for (DWORD dwIndex = 0; ; dwIndex++) {
			vnlen = VNAMELEN;
			if (::RegEnumValue(hKey, dwIndex, vname, &vnlen, NULL, &vtype, NULL, NULL) != ERROR_SUCCESS) {
				break;
			}
			if (::PathMatchSpec(vname, m_RegValueNameFormat) && vtype == REG_BINARY) {
				BYTE dummy;
				DWORD vdlen = sizeof(dummy);
				::RegQueryValueEx(hKey, vname, NULL, &vtype, &dummy, &vdlen);
				
				CTempBuffer<BYTE> vdata;
				vdata.Allocate(vdlen);
				::RegQueryValueEx(hKey, vname, NULL, &vtype, vdata, &vdlen);
				LPCDEFREG pDef = (LPCDEFREG)((BYTE*)vdata);
				if (pDef->dwType == CDEFREG_TYPE_RC01) {
					CString hk = GetHotkeyString(pDef->c.fsModifiers, pDef->c.vk);
					m_List.AddItem(count, 0, hk);
					m_List.AddItem(count, 1, pDef->szModule);
					m_List.AddItem(count, 2, vname);
					count++;
				}
			}
		}
		::RegCloseKey(hKey);
	}
	UpdateButtonStates();
}

void CListDlg::UpdateButtonStates(void)
{
	int selcount = 0;
	for (int i = m_List.GetItemCount() - 1; i >= 0; i--) {
		if (m_List.GetItemState(i, LVIS_SELECTED)) {
			selcount++;
		}
	}
	GetDlgItem(IDC_DEL).EnableWindow(selcount > 0);
	GetDlgItem(IDC_EDIT).EnableWindow(selcount == 1);
}

CString CListDlg::GenerateNewValueName(void)
{
	CString res;
	HKEY hKey;
	//if (::RegOpenKeyEx(m_hRootKey, m_RegKeyPath, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
	if (::RegCreateKeyEx(m_hRootKey, m_RegKeyPath, 0, NULL, 0, KEY_READ | KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
		CString fmt = m_RegValueNameFormat;
		fmt.Replace(_T("*"), _T("%d"));
		CString vname;
		for (DWORD dwIndex = 0; dwIndex < 0xFFFF; dwIndex++) {
			vname.Format(fmt, dwIndex);
			if (::RegQueryValueEx(hKey, vname, NULL, NULL, NULL, NULL) != ERROR_SUCCESS) {
				res = vname;
				break;
			}
		}
		::RegCloseKey(hKey);
	}
	return res;
}

void CListDlg::OnCmdNew(UINT, int, HWND)
{
	CEditDlg dlg;
	if (dlg.DoModal() == IDOK) {
		// save and update list
		CString vname = GenerateNewValueName();
		if (vname.IsEmpty()) {
			::MessageBox(m_hWnd, _T("設定を保存できません"), NULL, MB_OK | MB_ICONERROR);
			return;
		}
		LPCTSTR modname = dlg.GetModuleFilename();
		CTempBuffer<BYTE> buf;
		size_t siz = SIZEOF_CDEFREG(modname);
		buf.Allocate(siz);
		LPCDEFREG pDef = reinterpret_cast<LPCDEFREG>((BYTE*)(buf));
		pDef->dwType = CDEFREG_TYPE_RC01;
		pDef->c.dwAction = PRA_IG_RETURN_FALSE;
		pDef->c.fsModifiers = dlg.GetModifiers();
		pDef->c.vk = dlg.GetVirtualKey();
		pDef->c.id = 0;
		::_tcscpy_s(pDef->szModule, ::_tcslen(modname) + 1, modname);

		HKEY hKey;
		if (::RegOpenKeyEx(m_hRootKey, m_RegKeyPath, 0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS) {
			::RegSetValueEx(hKey, vname, 0, REG_BINARY, (BYTE*)buf, siz);
			::RegCloseKey(hKey);
		}

		int c = m_List.GetItemCount();
		CString keystr = GetHotkeyString(dlg.GetModifiers(), dlg.GetVirtualKey());
		{
			CSetRedraw redraw(m_List);
			c = m_List.AddItem(c + 1, 0, keystr);
			m_List.AddItem(c, 1, modname);
			m_List.AddItem(c, 2, vname);
		}
	}
	//UpdateButtonStates();
}

void CListDlg::OnCmdDel(UINT, int, HWND)
{
	CSetRedraw redraw(m_List);
	for (int i = m_List.GetItemCount() - 1; i >= 0; i--) {
		UINT s = m_List.GetItemState(i, LVIS_SELECTED);
		if (s == LVIS_SELECTED) {
			TCHAR vname[VNAMELEN];
			m_List.GetItemText(i, 2, vname, VNAMELEN);
			
			// 設定削除
			HKEY hKey;
			if (::RegOpenKeyEx(m_hRootKey, m_RegKeyPath, 0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS) {
				::RegDeleteValue(hKey, vname);
				::RegCloseKey(hKey);
			}

			m_List.DeleteItem(i);
		}
	}
	//UpdateButtonStates();
}

void CListDlg::OnCmdEdit(UINT, int, HWND)
{
	TCHAR vname[VNAMELEN];
	vname[0] = _T('\0');
	for (int i = m_List.GetItemCount() - 1; i >= 0; i--) {
		UINT s = m_List.GetItemState(i, LVIS_SELECTED);
		if (s == LVIS_SELECTED) {
			m_List.GetItemText(i, 2, vname, VNAMELEN);
			break;
		}
	}
	if (vname[0] == _T('\0')) {
		// 対象がない
		return;
	}
			
	HKEY hKey;
	DWORD vtype;
	DWORD vdlen;
	CTempBuffer<BYTE> vdata;
	LPCDEFREG pDef;
	
	CString modname;
	UINT fsModifiers;
	UINT vk;
	
	// ロード
	if (::RegOpenKeyEx(m_hRootKey, m_RegKeyPath, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		BYTE dummy;
		vdlen = sizeof(dummy);
		::RegQueryValueEx(hKey, vname, NULL, &vtype, &dummy, &vdlen);
		vdata.Allocate(vdlen);
		::RegQueryValueEx(hKey, vname, NULL, &vtype, vdata, &vdlen);
		::RegCloseKey(hKey);
		pDef = (LPCDEFREG)((BYTE*)vdata);
		modname = pDef->szModule;
		fsModifiers = pDef->c.fsModifiers;
		vk = pDef->c.vk;
	}
	
	CEditDlg dlg;
	dlg.SetModifiers(fsModifiers);
	dlg.SetVirtualKey(vk);
	dlg.SetModuleFilename(modname);
	if (dlg.DoModal() != IDOK) {
		// キャンセル
		return;
	}
	
	// セーブ
	if (::RegOpenKeyEx(m_hRootKey, m_RegKeyPath, 0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS) {
		vdlen = SIZEOF_CDEFREG(dlg.GetModuleFilename());
		vdata.Reallocate(vdlen);
		pDef = (LPCDEFREG)((BYTE*)vdata);
		pDef->dwType = CDEFREG_TYPE_RC01;
		::_tcscpy_s(pDef->szModule, ::_tcslen(dlg.GetModuleFilename()) + 1, dlg.GetModuleFilename());
		pDef->c.fsModifiers = dlg.GetModifiers();
		pDef->c.vk = dlg.GetVirtualKey();
		pDef->c.id = -1;
		pDef->c.dwAction = PRA_IG_RETURN_FALSE;
		LRESULT lr = ::RegSetValueEx(hKey, vname, 0, REG_BINARY, vdata, vdlen);
		::RegCloseKey(hKey);
	}
	
	// リスト更新
	int i;
	TCHAR vname2[VNAMELEN];
	for (i = m_List.GetItemCount() - 1; i >= 0; i--) {
		m_List.GetItemText(i, 2, vname2, VNAMELEN);
		if (::_tcscmp(vname, vname2) == 0) {
			break;
		}
	}
	if (i >= 0) {	
		m_List.SetItemText(i, 0, GetHotkeyString(dlg.GetModifiers(), dlg.GetVirtualKey()));
		m_List.SetItemText(i, 1, dlg.GetModuleFilename());
	}
}

LRESULT CListDlg::OnListViewItemChanged(LPNMHDR pnmh)
{
	UpdateButtonStates();
	return 0;
}

LRESULT CListDlg::OnListViewDoubleClicked(LPNMHDR pnmh)
{
	OnCmdEdit(0, 0, NULL);
	return 0;
}
