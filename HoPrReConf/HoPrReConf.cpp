// HoPrReConf.cpp : WinMain ‚ÌŽÀ‘•


#include "stdafx.h"
#include "resource.h"
#include "listdlg.h"

class CHoPrReConfModule : public CAtlExeModuleT< CHoPrReConfModule >
{
public :
	int WinMain(int nShowCmd);
};

CHoPrReConfModule _AtlModule;

int CHoPrReConfModule::WinMain(int nShowCmd)
{
	CListDlg dlg(_T("HKCU\\Software\\HoPrRe\\RC_*"));
	return dlg.DoModal();
}

//
extern "C" int WINAPI _tWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, 
                                LPTSTR /*lpCmdLine*/, int nShowCmd)
{
    return _AtlModule.WinMain(nShowCmd);
}

