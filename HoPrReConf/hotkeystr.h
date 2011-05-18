#pragma once

typedef enum { VKT_INVALID, VKT_NUMBER, VKT_SYMBOL, VKT_NAME } VKTYPE;

extern CString GetVKDisplayName(UINT vk);
extern CString GetVKID(UINT vk);
extern CString GetHotkeyString(UINT fsModifiers, UINT vk);
extern UINT StrToVK(LPCTSTR lpszStr, VKTYPE *vkt = NULL);
extern UINT IdStrToVK(LPCTSTR lpszId);
extern UINT NameStrToVK(LPCTSTR lpszName);
extern UINT NumStrToVK(LPCTSTR lpszNum);
