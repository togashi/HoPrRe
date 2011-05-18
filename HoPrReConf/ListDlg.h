#pragma once

#include "resource.h"

class CListDlg:
	public CDialogImpl<CListDlg>,
	public CDialogResize<CListDlg>
{
public:
	enum { IDD = IDD_LIST };

	BEGIN_MSG_MAP_EX(CListDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_SYSCOMMAND(OnSysCommand)
		COMMAND_ID_HANDLER_EX(IDCLOSE, OnCmdClose)
		COMMAND_ID_HANDLER_EX(IDC_NEW, OnCmdNew)
		COMMAND_ID_HANDLER_EX(IDC_DEL, OnCmdDel)
		COMMAND_ID_HANDLER_EX(IDC_EDIT, OnCmdEdit)
		NOTIFY_CODE_HANDLER_EX(LVN_ITEMCHANGED, OnListViewItemChanged)
		NOTIFY_CODE_HANDLER_EX(NM_DBLCLK, OnListViewDoubleClicked)
		CHAIN_MSG_MAP(CDialogResize<CListDlg>)
		CHAIN_MSG_MAP(__super);
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CListDlg)
		DLGRESIZE_CONTROL(IDC_LIST, DLSZ_SIZE_X | DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_NEW, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_DEL, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_EDIT, DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()
public:
	CListDlg(LPCTSTR lpszRegValueNameFormat);
	~CListDlg(void);

private:
	HKEY m_hRootKey;
	CString m_RegKeyPath;
	CString m_RegValueNameFormat;
	
	CListViewCtrl m_List;

	void UpdateList(void);
	void UpdateButtonStates(void);
	CString GenerateNewValueName(void);
private:
	// MSG_WM_*
	LRESULT OnInitDialog(HWND, LPARAM);
	void OnSysCommand(UINT, CPoint &);
	
	// COMMAND_ID_HANDLER_EX
	void OnCmdClose(UINT, int, HWND);
	void OnCmdNew(UINT, int, HWND);
	void OnCmdDel(UINT, int, HWND);
	void OnCmdEdit(UINT, int, HWND);

	// NOTIFY_CODE_HANDLER_EX
	LRESULT OnListViewItemChanged(LPNMHDR pnmh);
	LRESULT OnListViewDoubleClicked(LPNMHDR pnmh);
};
