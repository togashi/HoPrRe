// HoPrRe.cpp : DLL �A�v���P�[�V�����p�ɃG�N�X�|�[�g�����֐����`���܂��B
//

#include "stdafx.h"
#include "resource.h"

#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

#include <tlhelp32.h>
#include <Dbghelp.h>

#pragma comment(lib, "Dbghelp.lib")

/////////////////////////////////////////////////////////////////////
// API�t�b�N�����s���郁�C�����[�`��
/////////////////////////////////////////////////////////////////////

class CAPIHook 
{
public:

	// �R���X�g���N�^
	CAPIHook(PSTR, PSTR, PROC);

	// �f�X�g���N�^
	~CAPIHook();

	// �I���W�i���A�h���X�擾�p
	operator PROC() { return(m_pfnOrig); }

private:
	static CAPIHook *sm_pHead;  // �m�[�h�̃g�b�v�̃f�[�^
	CAPIHook *m_pNext;          // ���̃m�[�h�̃f�[�^

	// �ۑ��̈�
	PCSTR m_pszModuleName;      // �֐��������W���[�����iANSI�j
	PCSTR m_pszFuncName;        // �֐����iANSI�j
	PROC  m_pfnOrig;            // �I���W�i���֐��̃A�h���X
	PROC  m_pfnHook;            // �u�������֐��̃A�h���X

	// API�t�b�N��K�������郁�C���֐�
	static void WINAPI ReplaceIATEntryInAllMods(PCSTR, PROC, PROC);
	static void WINAPI ReplaceIATEntryInOneMod(PCSTR, PROC, PROC, HMODULE);

	// �V���Ƀ��[�h���ꂽ���W���[����API�t�b�N��Ή�������֐�
	static void WINAPI FixupNewlyLoadedModule(HMODULE, DWORD);

public:
	// ���ꂼ���API�ɒu�������t�b�N�֐�
	static HMODULE WINAPI Hook_LoadLibraryA(PCSTR);
	static HMODULE WINAPI Hook_LoadLibraryW(PCWSTR);
	static HMODULE WINAPI Hook_LoadLibraryExA(PCSTR, HANDLE, DWORD);
	static HMODULE WINAPI Hook_LoadLibraryExW(PCWSTR, HANDLE, DWORD);
	static FARPROC WINAPI Hook_GetProcAddress(HMODULE, PCSTR);

	// ���ꂼ���API���t�b�N���邱�Ƃ�錾
	static CAPIHook *sm_pLoadLibraryA;
	static CAPIHook *sm_pLoadLibraryW;
	static CAPIHook *sm_pLoadLibraryExA;
	static CAPIHook *sm_pLoadLibraryExW;
	static CAPIHook *sm_pGetProcAddress;
};

// �m�[�h�̃g�b�v��������
CAPIHook *CAPIHook::sm_pHead = NULL;


// �R���X�g���N�^
CAPIHook::CAPIHook(
					PSTR pszModuleName,
					PSTR pszFuncName, 
					PROC pfnHook)
{
	m_pNext  = sm_pHead; // ���̃m�[�h�̃A�h���X����
	sm_pHead = this;     // ���̃m�[�h�̃A�h���X����

	// �I���W�i���֐��̃A�h���X���擾
	PROC pfnOrig = ::GetProcAddress(
		GetModuleHandleA(pszModuleName), pszFuncName);
	
	// �t�b�N�Ɋւ���f�[�^��ۑ�
	m_pszModuleName      = pszModuleName;
	m_pszFuncName        = pszFuncName;
	m_pfnOrig            = pfnOrig;
	m_pfnHook            = pfnHook;

	// �v���Z�XID��0�Ȃ�DLL���}�b�s���O����ŏ��̃v���Z�X�Ɣ��f
	// ���̃v���Z�XID�����L�������ɕۑ�
	//if(g_dwCurrentProcessId == 0)
	//	g_dwCurrentProcessId = GetCurrentProcessId();

	// �N�����Ɠ����v���Z�X�Ȃ�t�b�N���s��Ȃ�
	//if(g_dwCurrentProcessId != GetCurrentProcessId())
		ReplaceIATEntryInAllMods(m_pszModuleName, m_pfnOrig, m_pfnHook);
}

// �f�X�g���N�^
CAPIHook::~CAPIHook() 
{
	// �N�����Ɠ����v���Z�X�Ȃ�����̕K�v�Ȃ�
	//if(g_dwCurrentProcessId != GetCurrentProcessId())
		ReplaceIATEntryInAllMods(m_pszModuleName, m_pfnHook, m_pfnOrig);

	// �m�[�h�̃g�b�v���擾
	CAPIHook *p = sm_pHead;

	// �m�[�h�̃g�b�v�������Ȃ�΁A���̃m�[�h���g�b�v�ɐ����ďI��
	if(p == this) {
		sm_pHead = p->m_pNext;
		return;
	}
	// ���������ł͂Ȃ��Ȃ�΁A�m�[�h�̒����猟������
	// ������A������O��
	while(p->m_pNext != NULL) {
		if (p->m_pNext == this) {
			p->m_pNext = p->m_pNext->m_pNext; 
			break; 
		}
		p = p->m_pNext;
	}
}


// ���ׂẴ��W���[���ɑ΂���API�t�b�N���s���֐�
void CAPIHook::ReplaceIATEntryInAllMods(
										PCSTR pszModuleName, 
										PROC pfnCurrent, 
										PROC pfnNew)
{
	// �������g�iAPI_Hook_Lib.dll�j�̃��W���[���n���h�����擾
	MEMORY_BASIC_INFORMATION mbi;
	if(VirtualQuery(ReplaceIATEntryInAllMods, &mbi, sizeof(mbi)) == 0)
		return;
	HMODULE hModThisMod = (HMODULE) mbi.AllocationBase;

	// ���W���[�����X�g���擾
	HANDLE hModuleSnap = CreateToolhelp32Snapshot(
		TH32CS_SNAPMODULE, GetCurrentProcessId());
	if(hModuleSnap == INVALID_HANDLE_VALUE)
		return;

	MODULEENTRY32 me;
	me.dwSize = sizeof(me);
	BOOL bModuleResult = Module32First(hModuleSnap, &me);
	// ���ꂼ��̃��W���[���ɑ΂���ReplaceIATEntryInOneMod�����s
	// �������������g�iAPI_Hook_Lib.dll�j�ɂ͍s��Ȃ�
	while(bModuleResult) {
		if(me.hModule != hModThisMod)
			ReplaceIATEntryInOneMod(pszModuleName, pfnCurrent, pfnNew, me.hModule);
		bModuleResult = Module32Next(hModuleSnap, &me);
	}
	CloseHandle(hModuleSnap);
}


// �ЂƂ̃��W���[���ɑ΂���API�t�b�N���s���֐�
void CAPIHook::ReplaceIATEntryInOneMod(
									   PCSTR pszModuleName, 
									   PROC pfnCurrent, 
									   PROC pfnNew, 
									   HMODULE hmodCaller) 
{
	// ���W���[���̃C���|�[�g�Z�N�V�����̃A�h���X���擾
	ULONG ulSize;
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc;
	pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(
		hmodCaller, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &ulSize);

	// �C���|�[�g�Z�N�V�����������Ă��Ȃ�
	if (pImportDesc == NULL)
		return;

	// �C���|�[�g�f�B�X�N���v�^������
	while(pImportDesc->Name) {
		PSTR pszModName = (PSTR) ((PBYTE) hmodCaller + pImportDesc->Name);
		if (lstrcmpiA(pszModName, pszModuleName) == 0) 
			break;
		pImportDesc++;
	}

	// ���̃��W���[���͌Ăяo���悩��֐����C���|�[�g���Ă��Ȃ�
	if (pImportDesc->Name == 0)
		return;

	// �C���|�[�g�A�h���X�e�[�u�����擾
	PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA) 
		((PBYTE) hmodCaller + pImportDesc->FirstThunk);

	// �V�����֐��A�h���X�ɒu��������
	while(pThunk->u1.Function) {
		
		// �֐��A�h���X�̃A�h���X���擾
		PROC *ppfn = (PROC*) &pThunk->u1.Function;

		// �Y���֐��ł���Ȃ�Δ����I
		BOOL fFound = (*ppfn == pfnCurrent);

		if (fFound) {
			// �A�h���X����v�����̂ŁA�C���|�[�g�Z�N�V�����̃A�h���X������������
			DWORD dwDummy;
			VirtualProtect(ppfn, sizeof(ppfn), PAGE_EXECUTE_READWRITE, &dwDummy);
			WriteProcessMemory(
				GetCurrentProcess(), ppfn, &pfnNew, sizeof(pfnNew), NULL);
			return;
		}
		pThunk++;
	}
	// �����ɏ������ڂ����ꍇ�A�C���|�[�g�Z�N�V�����ɊY���֐����Ȃ��������ƂɂȂ�
	return;
}

/////////////////////////////////////////////////////////////////////
// �f�t�H���g�Ńt�b�N����API�̏����iCAPIHook�N���X�����ŊǗ�����j
/////////////////////////////////////////////////////////////////////

// �f�t�H���g�Ńt�b�N����֐��̃v���g�^�C�v���`
typedef HMODULE (WINAPI *PLOADLIBRARYA)(PCSTR);
typedef HMODULE (WINAPI *PLOADLIBRARYW)(PCWSTR);
typedef HMODULE (WINAPI *PLOADLIBRARYEXA)(PCSTR, HANDLE, DWORD);
typedef HMODULE (WINAPI *PLOADLIBRARYEXW)(PCWSTR, HANDLE, DWORD);
typedef FARPROC (WINAPI *PGETPROCADDRESS)(HMODULE, PCSTR);

// ���W���[��������API�����炩���߃t�b�N���Ă���
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

// �V���Ƀ��[�h���ꂽ�֐����t�b�N����֐�
void CAPIHook::FixupNewlyLoadedModule(HMODULE hMod, DWORD dwFlags)
{
	if ((hMod != NULL) && ((dwFlags & LOAD_LIBRARY_AS_DATAFILE) == 0)) {
		for (CAPIHook *p = sm_pHead; p != NULL; p = p->m_pNext) {
			ReplaceIATEntryInOneMod(
				p->m_pszModuleName, p->m_pfnOrig, p->m_pfnHook, hMod);
		}
	}
}

// �u���������LoadLibraryA�֐�
HMODULE WINAPI CAPIHook::Hook_LoadLibraryA(PCSTR pszModulePath) 
{
	HMODULE hMod = (
		(PLOADLIBRARYA)(PROC) *(CAPIHook::sm_pLoadLibraryA))(pszModulePath);
	FixupNewlyLoadedModule(hMod, 0);
	return hMod;
}


// �u���������LoadLibraryW�֐�
HMODULE WINAPI CAPIHook::Hook_LoadLibraryW(PCWSTR pszModulePath) 
{
	HMODULE hMod = (
		(PLOADLIBRARYW)(PROC) *(CAPIHook::sm_pLoadLibraryW))(pszModulePath);
	FixupNewlyLoadedModule(hMod, 0);
	return hMod;
}

// �u���������LoadLibraryExA�֐�
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

// �u���������LoadLibraryExW�֐�
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


// �u���������GetProcAddress�֐�
FARPROC WINAPI CAPIHook::Hook_GetProcAddress(HMODULE hMod, PCSTR pszProcName)
{
	// �{���̊֐��A�h���X���擾
	FARPROC pfn = (
		(PGETPROCADDRESS)(PROC) *(CAPIHook::sm_pGetProcAddress))
		(hMod, pszProcName);

	// �����t�b�N���ׂ��֐��ł������Ȃ�΁A�u����������֐��̃A�h���X��n��
	for (CAPIHook *p = sm_pHead; (pfn != NULL) && (p != NULL); p = p->m_pNext) {
		if (pfn == p->m_pfnOrig)
			return (p->m_pfnHook);
	}
   return pfn;
}

/////////////////////////////////////////////////////////////////////
// �z�b�g�L�[�m�ېݒ�f�[�^�֘A
/////////////////////////////////////////////////////////////////////

#include "../hoprre_conf.h"

// ���݂̃v���Z�X�œK�p�����G���g���̃����N���X�g����
typedef struct HOPRRE_ENTRY_ {
	struct HOPRRE_ENTRY_ *pNext;
	HOPRRE_DAT c;
} HOPRRE_ENTRY;

// ���݂̃v���Z�X�œK�p�����G���g���̃����N���X�g
HOPRRE_ENTRY *g_Applied = NULL;


/////////////////////////////////////////////////////////////////////
// ���̍s�ȍ~�ɐV���Ƀt�b�N������API��錾���A�u��������֐���������
/////////////////////////////////////////////////////////////////////

// �ϐ���`
CAPIHook *g_pRegisterHotKey = NULL;

// �t�b�N����֐��̃v���g�^�C�v���`
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

// �K�p����郊�X�g��������
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

// �K�p����郊�X�g��j��
void UninitializeAppliedList(void) {
	while (g_Applied != NULL) {
		HOPRRE_ENTRY *tmp = g_Applied->pNext;
		delete g_Applied;
		g_Applied = tmp;
	}
}

// �t�b�N���d����
void DoSetAPIHook(void) {
		CAPIHook::sm_pLoadLibraryA = new CAPIHook("kernel32.dll", "LoadLibraryA", (PROC)CAPIHook::Hook_LoadLibraryA);
		CAPIHook::sm_pLoadLibraryW = new CAPIHook("kernel32.dll", "LoadLibraryW", (PROC)CAPIHook::Hook_LoadLibraryW);
		CAPIHook::sm_pLoadLibraryExA = new CAPIHook("kernel32.dll", "LoadLibraryExA", (PROC)CAPIHook::Hook_LoadLibraryExA);
		CAPIHook::sm_pLoadLibraryExW = new CAPIHook("kernel32.dll", "LoadLibraryExW", (PROC)CAPIHook::Hook_LoadLibraryExW);
		CAPIHook::sm_pGetProcAddress = new CAPIHook("kernel32.dll", "GetProcAddress", (PROC)CAPIHook::Hook_GetProcAddress);
		g_pRegisterHotKey = new CAPIHook("user32.dll", "RegisterHotKey", (PROC)Hook_RegisterHotKey);
}

// �t�b�N������
void DoUnsetAPIHook(void) {
	delete g_pRegisterHotKey;

	delete CAPIHook::sm_pGetProcAddress;
	delete CAPIHook::sm_pLoadLibraryExW;
	delete CAPIHook::sm_pLoadLibraryExA;
	delete CAPIHook::sm_pLoadLibraryW;
	delete CAPIHook::sm_pLoadLibraryA;
}

// �t�b�N�J�n
void SetAPIHook(HMODULE hModule) {
	InitializeAppliedList(hModule);
	if (g_Applied != NULL) {
		DoSetAPIHook();
	}
}

// �t�b�N�I��
void UnsetAPIHook(void) {
	if (g_Applied != NULL) {
		DoUnsetAPIHook();
		UninitializeAppliedList();
	}
}
