// HoPrRe.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"
#include "resource.h"

#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

#include <tlhelp32.h>
#include <Dbghelp.h>

#pragma comment(lib, "Dbghelp.lib")

/////////////////////////////////////////////////////////////////////
// APIフックを実行するメインルーチン
/////////////////////////////////////////////////////////////////////

class CAPIHook 
{
public:

	// コンストラクタ
	CAPIHook(PSTR, PSTR, PROC);

	// デストラクタ
	~CAPIHook();

	// オリジナルアドレス取得用
	operator PROC() { return(m_pfnOrig); }

private:
	static CAPIHook *sm_pHead;  // ノードのトップのデータ
	CAPIHook *m_pNext;          // 次のノードのデータ

	// 保存領域
	PCSTR m_pszModuleName;      // 関数を持つモジュール名（ANSI）
	PCSTR m_pszFuncName;        // 関数名（ANSI）
	PROC  m_pfnOrig;            // オリジナル関数のアドレス
	PROC  m_pfnHook;            // 置き換わる関数のアドレス

	// APIフックを適応させるメイン関数
	static void WINAPI ReplaceIATEntryInAllMods(PCSTR, PROC, PROC);
	static void WINAPI ReplaceIATEntryInOneMod(PCSTR, PROC, PROC, HMODULE);

	// 新たにロードされたモジュールにAPIフックを対応させる関数
	static void WINAPI FixupNewlyLoadedModule(HMODULE, DWORD);

public:
	// それぞれのAPIに置き換わるフック関数
	static HMODULE WINAPI Hook_LoadLibraryA(PCSTR);
	static HMODULE WINAPI Hook_LoadLibraryW(PCWSTR);
	static HMODULE WINAPI Hook_LoadLibraryExA(PCSTR, HANDLE, DWORD);
	static HMODULE WINAPI Hook_LoadLibraryExW(PCWSTR, HANDLE, DWORD);
	static FARPROC WINAPI Hook_GetProcAddress(HMODULE, PCSTR);

	// それぞれのAPIをフックすることを宣言
	static CAPIHook *sm_pLoadLibraryA;
	static CAPIHook *sm_pLoadLibraryW;
	static CAPIHook *sm_pLoadLibraryExA;
	static CAPIHook *sm_pLoadLibraryExW;
	static CAPIHook *sm_pGetProcAddress;
};

// ノードのトップを初期化
CAPIHook *CAPIHook::sm_pHead = NULL;


// コンストラクタ
CAPIHook::CAPIHook(
					PSTR pszModuleName,
					PSTR pszFuncName, 
					PROC pfnHook)
{
	m_pNext  = sm_pHead; // 次のノードのアドレスを代入
	sm_pHead = this;     // このノードのアドレスを代入

	// オリジナル関数のアドレスを取得
	PROC pfnOrig = ::GetProcAddress(
		GetModuleHandleA(pszModuleName), pszFuncName);
	
	// フックに関するデータを保存
	m_pszModuleName      = pszModuleName;
	m_pszFuncName        = pszFuncName;
	m_pfnOrig            = pfnOrig;
	m_pfnHook            = pfnHook;

	// プロセスIDが0ならDLLをマッピングする最初のプロセスと判断
	// そのプロセスIDを共有メモリに保存
	//if(g_dwCurrentProcessId == 0)
	//	g_dwCurrentProcessId = GetCurrentProcessId();

	// 起動元と同じプロセスならフックを行わない
	//if(g_dwCurrentProcessId != GetCurrentProcessId())
		ReplaceIATEntryInAllMods(m_pszModuleName, m_pfnOrig, m_pfnHook);
}

// デストラクタ
CAPIHook::~CAPIHook() 
{
	// 起動元と同じプロセスなら解除の必要なし
	//if(g_dwCurrentProcessId != GetCurrentProcessId())
		ReplaceIATEntryInAllMods(m_pszModuleName, m_pfnHook, m_pfnOrig);

	// ノードのトップを取得
	CAPIHook *p = sm_pHead;

	// ノードのトップが自分ならば、次のノードをトップに据えて終了
	if(p == this) {
		sm_pHead = p->m_pNext;
		return;
	}
	// もし自分ではないならば、ノードの中から検索して
	// 自分を連結から外す
	while(p->m_pNext != NULL) {
		if (p->m_pNext == this) {
			p->m_pNext = p->m_pNext->m_pNext; 
			break; 
		}
		p = p->m_pNext;
	}
}


// すべてのモジュールに対してAPIフックを行う関数
void CAPIHook::ReplaceIATEntryInAllMods(
										PCSTR pszModuleName, 
										PROC pfnCurrent, 
										PROC pfnNew)
{
	// 自分自身（API_Hook_Lib.dll）のモジュールハンドルを取得
	MEMORY_BASIC_INFORMATION mbi;
	if(VirtualQuery(ReplaceIATEntryInAllMods, &mbi, sizeof(mbi)) == 0)
		return;
	HMODULE hModThisMod = (HMODULE) mbi.AllocationBase;

	// モジュールリストを取得
	HANDLE hModuleSnap = CreateToolhelp32Snapshot(
		TH32CS_SNAPMODULE, GetCurrentProcessId());
	if(hModuleSnap == INVALID_HANDLE_VALUE)
		return;

	MODULEENTRY32 me;
	me.dwSize = sizeof(me);
	BOOL bModuleResult = Module32First(hModuleSnap, &me);
	// それぞれのモジュールに対してReplaceIATEntryInOneModを実行
	// ただし自分自身（API_Hook_Lib.dll）には行わない
	while(bModuleResult) {
		if(me.hModule != hModThisMod)
			ReplaceIATEntryInOneMod(pszModuleName, pfnCurrent, pfnNew, me.hModule);
		bModuleResult = Module32Next(hModuleSnap, &me);
	}
	CloseHandle(hModuleSnap);
}


// ひとつのモジュールに対してAPIフックを行う関数
void CAPIHook::ReplaceIATEntryInOneMod(
									   PCSTR pszModuleName, 
									   PROC pfnCurrent, 
									   PROC pfnNew, 
									   HMODULE hmodCaller) 
{
	// モジュールのインポートセクションのアドレスを取得
	ULONG ulSize;
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc;
	pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(
		hmodCaller, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &ulSize);

	// インポートセクションを持っていない
	if (pImportDesc == NULL)
		return;

	// インポートディスクリプタを検索
	while(pImportDesc->Name) {
		PSTR pszModName = (PSTR) ((PBYTE) hmodCaller + pImportDesc->Name);
		if (lstrcmpiA(pszModName, pszModuleName) == 0) 
			break;
		pImportDesc++;
	}

	// このモジュールは呼び出し先から関数をインポートしていない
	if (pImportDesc->Name == 0)
		return;

	// インポートアドレステーブルを取得
	PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA) 
		((PBYTE) hmodCaller + pImportDesc->FirstThunk);

	// 新しい関数アドレスに置き換える
	while(pThunk->u1.Function) {
		
		// 関数アドレスのアドレスを取得
		PROC *ppfn = (PROC*) &pThunk->u1.Function;

		// 該当関数であるならば発見！
		BOOL fFound = (*ppfn == pfnCurrent);

		if (fFound) {
			// アドレスが一致したので、インポートセクションのアドレスを書き換える
			DWORD dwDummy;
			VirtualProtect(ppfn, sizeof(ppfn), PAGE_EXECUTE_READWRITE, &dwDummy);
			WriteProcessMemory(
				GetCurrentProcess(), ppfn, &pfnNew, sizeof(pfnNew), NULL);
			return;
		}
		pThunk++;
	}
	// ここに処理が移った場合、インポートセクションに該当関数がなかったことになる
	return;
}

/////////////////////////////////////////////////////////////////////
// デフォルトでフックするAPIの処理（CAPIHookクラス内部で管理する）
/////////////////////////////////////////////////////////////////////

// デフォルトでフックする関数のプロトタイプを定義
typedef HMODULE (WINAPI *PLOADLIBRARYA)(PCSTR);
typedef HMODULE (WINAPI *PLOADLIBRARYW)(PCWSTR);
typedef HMODULE (WINAPI *PLOADLIBRARYEXA)(PCSTR, HANDLE, DWORD);
typedef HMODULE (WINAPI *PLOADLIBRARYEXW)(PCWSTR, HANDLE, DWORD);
typedef FARPROC (WINAPI *PGETPROCADDRESS)(HMODULE, PCSTR);

// モジュールを扱うAPIをあらかじめフックしておく
//CAPIHook CAPIHook::sm_LoadLibraryA  (
//	"Kernel32.dll", "LoadLibraryA", (PROC) CAPIHook::Hook_LoadLibraryA);
CAPIHook *CAPIHook::sm_pLoadLibraryA = NULL;
//CAPIHook CAPIHook::sm_LoadLibraryW  (
//	"Kernel32.dll", "LoadLibraryW", (PROC) CAPIHook::Hook_LoadLibraryW);
CAPIHook *CAPIHook::sm_pLoadLibraryW = NULL;
//CAPIHook CAPIHook::sm_LoadLibraryExA(
//	"Kernel32.dll", "LoadLibraryExA", (PROC) CAPIHook::Hook_LoadLibraryExA);
CAPIHook *CAPIHook::sm_pLoadLibraryExA = NULL;
//CAPIHook CAPIHook::sm_LoadLibraryExW(
//	"Kernel32.dll", "LoadLibraryExW", (PROC) CAPIHook::Hook_LoadLibraryExW);
CAPIHook *CAPIHook::sm_pLoadLibraryExW = NULL;
//CAPIHook CAPIHook::sm_GetProcAddress(
//	"Kernel32.dll", "GetProcAddress", (PROC) CAPIHook::Hook_GetProcAddress);
CAPIHook *CAPIHook::sm_pGetProcAddress = NULL;

// 新たにロードされた関数をフックする関数
void CAPIHook::FixupNewlyLoadedModule(HMODULE hMod, DWORD dwFlags)
{
	if ((hMod != NULL) && ((dwFlags & LOAD_LIBRARY_AS_DATAFILE) == 0)) {
		for (CAPIHook *p = sm_pHead; p != NULL; p = p->m_pNext) {
			ReplaceIATEntryInOneMod(
				p->m_pszModuleName, p->m_pfnOrig, p->m_pfnHook, hMod);
		}
	}
}

// 置き換わったLoadLibraryA関数
HMODULE WINAPI CAPIHook::Hook_LoadLibraryA(PCSTR pszModulePath) 
{
	HMODULE hMod = (
		(PLOADLIBRARYA)(PROC) *(CAPIHook::sm_pLoadLibraryA))(pszModulePath);
	FixupNewlyLoadedModule(hMod, 0);
	return hMod;
}


// 置き換わったLoadLibraryW関数
HMODULE WINAPI CAPIHook::Hook_LoadLibraryW(PCWSTR pszModulePath) 
{
	HMODULE hMod = (
		(PLOADLIBRARYW)(PROC) *(CAPIHook::sm_pLoadLibraryW))(pszModulePath);
	FixupNewlyLoadedModule(hMod, 0);
	return hMod;
}

// 置き換わったLoadLibraryExA関数
HMODULE WINAPI CAPIHook::Hook_LoadLibraryExA(
										PCSTR pszModulePath, 
										HANDLE hFile, 
										DWORD dwFlags)
{
	HMODULE hMod = (
		(PLOADLIBRARYEXA)(PROC) *(CAPIHook::sm_pLoadLibraryExA))
		(pszModulePath, hFile, dwFlags);
	FixupNewlyLoadedModule(hMod, dwFlags);
	return hMod;
}

// 置き換わったLoadLibraryExW関数
HMODULE WINAPI CAPIHook::Hook_LoadLibraryExW(
										PCWSTR pszModulePath, 
										HANDLE hFile, 
										DWORD dwFlags) 
{
	HMODULE hMod = (
		(PLOADLIBRARYEXW)(PROC) *(CAPIHook::sm_pLoadLibraryExW))
		(pszModulePath, hFile, dwFlags);
	FixupNewlyLoadedModule(hMod, dwFlags);
	return hMod;
}


// 置き換わったGetProcAddress関数
FARPROC WINAPI CAPIHook::Hook_GetProcAddress(HMODULE hMod, PCSTR pszProcName)
{
	// 本当の関数アドレスを取得
	FARPROC pfn = (
		(PGETPROCADDRESS)(PROC) *(CAPIHook::sm_pGetProcAddress))
		(hMod, pszProcName);

	// もしフックすべき関数であったならば、置き換わった関数のアドレスを渡す
	for (CAPIHook *p = sm_pHead; (pfn != NULL) && (p != NULL); p = p->m_pNext) {
		if (pfn == p->m_pfnOrig)
			return (p->m_pfnHook);
	}
   return pfn;
}

/////////////////////////////////////////////////////////////////////
// ホットキー確保設定データ関連
/////////////////////////////////////////////////////////////////////

#include "../hoprre_conf.h"

// 現在のプロセスで適用されるエントリのリンクリスト項目
typedef struct HOPRRE_ENTRY_ {
	struct HOPRRE_ENTRY_ *pNext;
	HOPRRE_DAT c;
} HOPRRE_ENTRY;

// 現在のプロセスで適用されるエントリのリンクリスト
HOPRRE_ENTRY *g_Applied = NULL;


/////////////////////////////////////////////////////////////////////
// この行以降に新たにフックしたいAPIを宣言し、置き換える関数を加える
/////////////////////////////////////////////////////////////////////

// 変数定義
CAPIHook *g_pRegisterHotKey = NULL;

// フックする関数のプロトタイプを定義
typedef BOOL (WINAPI *PFNREGISTERHOTKEY)(HWND, INT, UINT, UINT);

#define FLAGSTR(a) if (fsModifiers & (a)) { ::_ftprintf(f, _T(" %s"), _T(#a)); }
#define MOD_KEYS (MOD_ALT | MOD_CONTROL | MOD_SHIFT | MOD_WIN)

struct LOGFILE {
	TCHAR path[MAX_PATH + 1];
	LOGFILE() {
		::ExpandEnvironmentStrings(_T("%USERPROFILE%\\hoprre_log.txt"), path, MAX_PATH);
	}
} g_LogFile;

BOOL WINAPI Hook_RegisterHotKey(HWND hWnd, INT id, UINT fsModifiers, UINT vk)
{
	#ifdef _DEBUG
	FILE *f;
	::_tfopen_s(&f, g_LogFile.path, _T("a"));
	if (f != NULL) {
		::_ftprintf(f, _T("%d:"), id);
		FLAGSTR(MOD_ALT);
		FLAGSTR(MOD_CONTROL);
		FLAGSTR(MOD_SHIFT);
		FLAGSTR(MOD_WIN);
		if (vk >= 'A' && vk <= 'Z') {
			::_ftprintf(f, _T(" %c\n"), vk);
		} else {
			::_ftprintf(f, _T(" 0x%X\n"), vk);
		}
		::fclose(f);
	}
	#endif
	
	UINT mods = fsModifiers & MOD_KEYS;
	for (HOPRRE_ENTRY *cur = g_Applied; cur != NULL; cur = cur->pNext) {
		BOOL match_id = (cur->c.id == -1) || (cur->c.id == id);
		BOOL match_mod = (cur->c.fsModifiers == 0xFFFFFFFF) || (cur->c.fsModifiers == mods);
		BOOL match_vk = (cur->c.vk == 0xFFFFFFFF) || (cur->c.vk == vk);
		
		#ifdef _DEBUG
		FILE *f;
		::_tfopen_s(&f, g_LogFile.path, _T("a"));
		if (f != NULL) {
			::_ftprintf(
				f,
				_T("%s:%d/%d; %s:%d/%d; %s:%d/%d;\r\n"),
				(match_id ? _T("ID") : _T("id")) ,cur->c.id, id,
				(match_mod ? _T("MOD") : _T("mod")), cur->c.fsModifiers, mods,
				(match_vk ? _T("VK") : _T("vk")), cur->c.vk, vk
			);
			::fclose(f);
		}
		#endif
		
		if (match_id && match_mod && match_vk) {
			switch (cur->c.dwAction) {
				case PRA_AU_RETURN_FALSE:
					((PFNREGISTERHOTKEY)(PROC) *(g_pRegisterHotKey))(hWnd, id, fsModifiers, vk);
					::UnregisterHotKey(hWnd, id);
				case PRA_IG_RETURN_FALSE:
					return FALSE;
				case PRA_AU_RETURN_TRUE:
					((PFNREGISTERHOTKEY)(PROC) *(g_pRegisterHotKey))(hWnd, id, fsModifiers, vk);
					::UnregisterHotKey(hWnd, id);
				case PRA_IG_RETURN_TRUE:
					return TRUE;
				default:
					return FALSE;
			}
		}
	}
	return ((PFNREGISTERHOTKEY)(PROC) *(g_pRegisterHotKey))(hWnd, id, fsModifiers, vk);
}

// 適用されるリストを初期化
void InitializeAppliedList(HMODULE hModule) {
	g_Applied = NULL;

	TCHAR fn[MAX_PATH + 1];
	::GetModuleFileName(NULL, fn, MAX_PATH);
	PTCHAR fs = ::_tcsrchr(fn, _T('\\'));
	fs = (fs == NULL) ? fn : fs + 1;

	TCHAR szRegPath[MAX_PATH + 1];
	if (::LoadString(hModule, IDS_REG_CONFIG, szRegPath, MAX_PATH) == 0) {
		::_tcscpy_s(szRegPath, MAX_PATH, _T("HKCU\\Software\\HoPrRe"));
	}
	TCHAR szVNPat[80];
	if (::LoadString(hModule, IDS_REG_CONFIG_VFMT, szVNPat, 80) == 0) {
		::_tcscpy_s(szVNPat, 80, _T("RC_*"));
	}

	HKEY hRoot = HKEY_CURRENT_USER;
	LPTSTR lpszRegSubKey = ::_tcschr(szRegPath, _T('\\'));
	if (lpszRegSubKey != NULL) {
		*lpszRegSubKey = _T('\0');
		lpszRegSubKey++;
		if (::_tcsicmp(szRegPath, _T("HKEY_LOCAL_MACHINE")) == 0 || ::_tcsicmp(szRegPath, _T("HKLM")) == 0) {
			hRoot = HKEY_LOCAL_MACHINE;
		} else if (::_tcsicmp(szRegPath, _T("HKEY_CURRENT_USER")) == 0 || ::_tcsicmp(szRegPath, _T("HKCU")) == 0) {
			hRoot = HKEY_CURRENT_USER;
		}
	} else {
		lpszRegSubKey = szRegPath;
	}
	
	HKEY hKey = NULL;
	if (::RegOpenKeyEx(hRoot, lpszRegSubKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
		return;
	}
	
	#define VNAMELEN 80
	TCHAR vname[VNAMELEN];
	DWORD vnlen;
	DWORD vtype;
	for (DWORD dwIndex = 0; ; dwIndex++) {
		vnlen = VNAMELEN;
		if (::RegEnumValue(hKey, dwIndex, vname, &vnlen, NULL, &vtype, NULL, NULL) != ERROR_SUCCESS) {
			break;
		}
		if (::PathMatchSpec(vname, szVNPat) && vtype == REG_BINARY) {
			BYTE dummy;
			DWORD vdlen = sizeof(dummy);
			::RegQueryValueEx(hKey, vname, NULL, &vtype, &dummy, &vdlen);
			LPBYTE vdata = new BYTE[vdlen];
			::RegQueryValueEx(hKey, vname, NULL, &vtype, vdata, &vdlen);
			LPCDEFREG pDef = (LPCDEFREG)vdata;
			if (pDef->dwType == CDEFREG_TYPE_RC01) {
				if (pDef->szModule[0] == _T('\0') || ::PathMatchSpec(fs, pDef->szModule)) {
					HOPRRE_ENTRY *newone = new HOPRRE_ENTRY;
					newone->pNext = g_Applied;
					newone->c = pDef->c;
					g_Applied = newone;
				}
			}
			delete [] vdata;
		}
	}

	#ifdef _DEBUG
	FILE *f;
	::_tfopen_s(&f, g_LogFile.path, _T("a"));
	if (f != NULL) {
		int count = 0;
		HOPRRE_ENTRY *cur = g_Applied;
		while (cur != NULL) {
			count++;
			cur = cur->pNext;
		}
		if (count > 0) {
			::_ftprintf(f, _T("%s: %d rules applied.\r\n"), fs, count);
		}
		::fclose(f);
	}
	#endif
}

// 適用されるリストを破棄
void UninitializeAppliedList(void) {
	while (g_Applied != NULL) {
		HOPRRE_ENTRY *tmp = g_Applied->pNext;
		delete g_Applied;
		g_Applied = tmp;
	}
}

// フックを仕込む
void DoSetAPIHook(void) {
		CAPIHook::sm_pLoadLibraryA = new CAPIHook("kernel32.dll", "LoadLibraryA", (PROC)CAPIHook::Hook_LoadLibraryA);
		CAPIHook::sm_pLoadLibraryW = new CAPIHook("kernel32.dll", "LoadLibraryW", (PROC)CAPIHook::Hook_LoadLibraryW);
		CAPIHook::sm_pLoadLibraryExA = new CAPIHook("kernel32.dll", "LoadLibraryExA", (PROC)CAPIHook::Hook_LoadLibraryExA);
		CAPIHook::sm_pLoadLibraryExW = new CAPIHook("kernel32.dll", "LoadLibraryExW", (PROC)CAPIHook::Hook_LoadLibraryExW);
		CAPIHook::sm_pGetProcAddress = new CAPIHook("kernel32.dll", "GetProcAddress", (PROC)CAPIHook::Hook_GetProcAddress);
		g_pRegisterHotKey = new CAPIHook("user32.dll", "RegisterHotKey", (PROC)Hook_RegisterHotKey);
}

// フックを解除
void DoUnsetAPIHook(void) {
	delete g_pRegisterHotKey;

	delete CAPIHook::sm_pGetProcAddress;
	delete CAPIHook::sm_pLoadLibraryExW;
	delete CAPIHook::sm_pLoadLibraryExA;
	delete CAPIHook::sm_pLoadLibraryW;
	delete CAPIHook::sm_pLoadLibraryA;
}

// フック開始
void SetAPIHook(HMODULE hModule) {
	InitializeAppliedList(hModule);
	if (g_Applied != NULL) {
		DoSetAPIHook();
	}
}

// フック終了
void UnsetAPIHook(void) {
	if (g_Applied != NULL) {
		DoUnsetAPIHook();
		UninitializeAppliedList();
	}
}
