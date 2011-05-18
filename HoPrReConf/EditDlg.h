#pragma once

#include "resource.h"

class CEditDlg:
	public CDialogImpl<CEditDlg>
{
public:
	enum { IDD = IDD_EDIT };
	BEGIN_MSG_MAP_EX(CEditDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_SYSCOMMAND(OnSysCommand)
		MSG_WM_TIMER(OnTimer)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnClosingCmd)
		COMMAND_ID_HANDLER_EX(IDOK, OnClosingCmd)
		COMMAND_CODE_HANDLER_EX(BN_CLICKED, OnButtonClicked)
		COMMAND_CODE_HANDLER_EX(EN_CHANGE, OnEditChanged)
	END_MSG_MAP()
public:
	CEditDlg(void);
	CEditDlg(UINT fsModifiers, UINT vk, LPCTSTR lpszModName);
	~CEditDlg(void);

	UINT GetModifiers(void);
	UINT GetVirtualKey(void);
	LPCTSTR GetModuleFilename(void);

	void SetModifiers(UINT);
	void SetVirtualKey(UINT);
	void SetModuleFilename(LPCTSTR);
private:
	UINT m_Modifiers;
	UINT m_vk;
	CString m_ModName;

	CButton m_ChkWin;
	CButton m_ChkCtrl;
	CButton m_ChkAlt;
	CButton m_ChkShift;
	CEdit m_EdtVk;
	CHotKeyCtrl m_Hotkey;
	CEdit m_EdtShow;
	CButton m_ChkCode;

	CEdit m_EdtMod;
	CStatic m_SttEffect;

	void DataToDialog(void);
	void DialogToData(void);
	void UpdateEffect(BOOL bImmediately);
	CString GetVKDetail(UINT vk);
private:
	// MSG_WM_*
	LRESULT OnInitDialog(HWND, LPARAM);
	void OnSysCommand(UINT, CPoint &);
	void OnTimer(UINT_PTR id);
	
	// COMMAND_ID_HANDLER_EX
	void OnClosingCmd(UINT, int, HWND);
	// COMMAND_CODE_HANDLER_EX
	void OnButtonClicked(UINT, int, HWND);
	void OnEditChanged(UINT, int, HWND);
};
