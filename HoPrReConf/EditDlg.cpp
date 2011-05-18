#include "StdAfx.h"
#include "EditDlg.h"
#include "hotkeystr.h"

CEditDlg::CEditDlg(void):
	m_Modifiers(0),
	m_vk(0),
	m_ModName(_T(""))
{
}

CEditDlg::CEditDlg(UINT fsModifiers, UINT vk, LPCTSTR lpszModName):
	m_Modifiers(fsModifiers),
	m_vk(vk),
	m_ModName(lpszModName)
{
}

CEditDlg::~CEditDlg(void)
{
}

UINT CEditDlg::GetModifiers(void)
{
	return m_Modifiers;
}

UINT CEditDlg::GetVirtualKey(void)
{
	return m_vk;
}

LPCTSTR CEditDlg::GetModuleFilename(void) {
	return m_ModName;
}

LRESULT CEditDlg::OnInitDialog(HWND, LPARAM)
{
	m_ChkWin.Attach(GetDlgItem(IDC_CHECK_MOD_W));
	m_ChkCtrl.Attach(GetDlgItem(IDC_CHECK_MOD_C));
	m_ChkAlt.Attach(GetDlgItem(IDC_CHECK_MOD_A));
	m_ChkShift.Attach(GetDlgItem(IDC_CHECK_MOD_S));
	m_EdtVk.Attach(GetDlgItem(IDC_EDIT_VK));
	m_Hotkey.Attach(GetDlgItem(IDC_HOTKEY));
	m_ChkCode.Attach(GetDlgItem(IDC_CHECK_VKCODE));
	m_EdtShow.Attach(GetDlgItem(IDC_EDITSHOW));
	m_EdtMod.Attach(GetDlgItem(IDC_EDIT_MOD));
	m_SttEffect.Attach(GetDlgItem(IDC_STATIC_EFFECT));
	m_ChkCode.SetCheck(BST_CHECKED);
	{
		RECT rc;
		m_EdtVk.GetWindowRect(&rc);
		::MapWindowPoints(HWND_DESKTOP, *this, reinterpret_cast<LPPOINT>(&rc) , 2);
		m_Hotkey.SetWindowPos(HWND_TOP, &rc, SWP_NOZORDER | SWP_NOACTIVATE);
		m_Hotkey.ShowWindow(SW_HIDE);
		m_Hotkey.EnableWindow(FALSE);
		m_Hotkey.SetRules(HKCOMB_S | HKCOMB_C | HKCOMB_A | HKCOMB_SC | HKCOMB_SA | HKCOMB_CA | HKCOMB_SCA, 0);
	}
	DataToDialog();
	return 1;
}

void CEditDlg::OnSysCommand(UINT sc, CPoint &)
{
	if (sc == SC_CLOSE) {
		PostMessage(WM_COMMAND, MAKELONG(IDCANCEL, BN_CLICKED), NULL);
	}
	SetMsgHandled(FALSE);
}

void CEditDlg::OnClosingCmd(UINT, int id, HWND)
{
	EndDialog(id);
}

void CEditDlg::DataToDialog(void)
{
	m_ChkWin.SetCheck((m_Modifiers & MOD_WIN) ? BST_CHECKED: BST_UNCHECKED);
	m_ChkCtrl.SetCheck((m_Modifiers & MOD_CONTROL) ? BST_CHECKED: BST_UNCHECKED);
	m_ChkAlt.SetCheck((m_Modifiers & MOD_ALT) ? BST_CHECKED: BST_UNCHECKED);
	m_ChkShift.SetCheck((m_Modifiers & MOD_SHIFT) ? BST_CHECKED: BST_UNCHECKED);
	CString tmp;
	tmp = GetVKID(m_vk);
	m_EdtVk.SetWindowText(tmp);
	m_EdtMod.SetWindowText(m_ModName);
}

void CEditDlg::DialogToData(void)
{
}

CString CEditDlg::GetVKDetail(UINT vk)
{
	CString ret;
	if (vk == 0) {
		return ret;
	}
	ret.Format(_T("0x%2.2X(%d)"), vk, vk);
	CString id, dn;
	id = GetVKID(vk);
	dn = GetVKDisplayName(vk);
	if (!id.IsEmpty()) {
		ret.AppendFormat(_T(", %s"), id);
	}
	if (!dn.IsEmpty() && id != dn) {
		ret.AppendFormat(_T(", %s"), dn);
	}
	return ret;
}

void CEditDlg::UpdateEffect(BOOL bImmediately)
{
	if (!bImmediately) {
		KillTimer(0);
		SetTimer(0, 750, NULL);
		return;
	}
	
	CString tmp;
	
	CString key;
	if (m_Modifiers != 0) {
		key = GetHotkeyString(m_Modifiers, m_vk);
	}
	if (!key.IsEmpty()) {
		CString mod;	
		m_EdtMod.GetWindowText(mod);
		if (mod.IsEmpty() || mod == _T("*")) {
			tmp.Format(_T("全プロセスによるホットキー \"%s\" の登録を制限する"), key);
		} else {
			tmp.Format(_T("モジュールファイル名が \"%s\" にマッチするプロセスによるホットキー \"%s\" の登録を制限する"), mod, key);
		}
	}
	if (!tmp.IsEmpty()) {
		tmp.Insert(0, _T("得られる効果:\r\n"));
	}
	m_SttEffect.SetWindowText(tmp);
}

void CEditDlg::OnButtonClicked(UINT, int id, HWND)
{
	if (id == IDC_CHECK_VKCODE) {
		BOOL checked = IsDlgButtonChecked(IDC_CHECK_VKCODE) == BST_CHECKED;
		
		m_EdtVk.ShowWindow(checked ? SW_SHOWNORMAL : SW_HIDE);
		m_EdtVk.EnableWindow(checked);
		
		m_Hotkey.ShowWindow(checked ? SW_HIDE : SW_SHOWNORMAL);
		m_Hotkey.EnableWindow(!checked);
		
		if (checked) {
			CString vkstr(GetVKID(m_vk));
			m_EdtVk.SetWindowText(vkstr);
		} else {
			m_Hotkey.SetHotKey(m_vk, 0);
		}
		return;
	}

	#define CHKMAP(o, m) if (o.GetCheck() == BST_CHECKED) { m_Modifiers |= (m); } else { m_Modifiers &= ~(m); }
	switch (id) {
		case IDC_CHECK_MOD_W:
			CHKMAP(m_ChkWin, MOD_WIN);
			break;
		case IDC_CHECK_MOD_C:
			CHKMAP(m_ChkCtrl, MOD_CONTROL);
			break;
		case IDC_CHECK_MOD_A:
			CHKMAP(m_ChkAlt, MOD_ALT);
			break;
		case IDC_CHECK_MOD_S:
			CHKMAP(m_ChkShift, MOD_SHIFT);
			break;
	}
	UpdateEffect(FALSE);
}

void CEditDlg::OnEditChanged(UINT, int id, HWND)
{
	if (id == IDC_EDIT_VK) {
		CString tmp;
		m_EdtVk.GetWindowText(tmp);
		m_vk = StrToVK(tmp);
		m_EdtShow.SetWindowText(GetVKDetail(m_vk));
	} else if (id == IDC_HOTKEY) {
		WORD wVk;
		WORD wMod;
		m_Hotkey.GetHotKey(wVk, wMod);
		m_vk = wVk;
		m_EdtShow.SetWindowText(GetVKDetail(m_vk));
	} else if (id == IDC_EDIT_MOD) {
		m_EdtMod.GetWindowText(m_ModName);
	}
	UpdateEffect(FALSE);
}

void CEditDlg::SetModifiers(UINT fsModifiers)
{
	m_Modifiers = fsModifiers;
}

void CEditDlg::SetVirtualKey(UINT vk)
{
	m_vk = vk;
}

void CEditDlg::SetModuleFilename(LPCTSTR modname)
{
	m_ModName = modname;
}

void CEditDlg::OnTimer(UINT_PTR id)
{
	KillTimer(id);
	UpdateEffect(TRUE);
}
