#pragma once
#include "framework.h"
#include <tchar.h>

#define APPHELP_API EXTERN_C DECLSPEC_IMPORT

#define ASL_LEVEL_PRINT 1
#define ASL_LEVEL_ERROR 2
#define ASL_LEVEL_MARK 3
#define ASL_LEVEL_TRACE 4

#define SE_NOTIFY_INIT 1
#define SE_NOTIFY_PROCESS_DYING 2
#define SE_NOTIFY_DLL_LOAD 3
#define SE_NOTIFY_DLL_UNLOAD 4
#define SE_NOTIFY_LDR_ENTRY_REMOVED 105

typedef struct tagHOOKAPI {
	PCSTR DllName;
	PCSTR FunctionName;
	PVOID HookFunction;
	PVOID NextFunction;
	PVOID Reserved[2];
} HOOKAPI, *PHOOKAPI;

APPHELP_API BOOL WINAPI SE_COM_AddHook(DWORD shimId, REFCLSID rcls, REFIID riid, DWORD vtblIndex, PVOID pfHook);
APPHELP_API BOOL WINAPI SE_COM_AddServer(LPCWSTR pwszDll, BOOL exclude);
APPHELP_API BOOL WINAPI SE_COM_HookInterface(REFCLSID rcls, REFIID riid, PVOID* ppvObject, BOOL isClassFactory);
APPHELP_API BOOL WINAPI SE_COM_HookObject(PVOID unknown1, REFIID riid, PVOID* ppvObject, BOOL unknown2, BOOL isClassFactory);
APPHELP_API PVOID WINAPI SE_COM_Lookup(PVOID pvObject, PVOID pfHook, LPCSTR pszHookName);
APPHELP_API DWORD WINAPI SE_GetShimId(HMODULE hModule, LPCWSTR pwszShimName);
APPHELP_API VOID __cdecl SE_ShimDPF(DWORD logLevel, LPCSTR component, LPCSTR function, DWORD line, LPCSTR format, ...);

#define ASL_PRINTF(LOG_LEVEL, FORMAT_STR, ...) SE_ShimDPF((LOG_LEVEL), __FILE__, __FUNCTION__, __LINE__, (FORMAT_STR), __VA_ARGS__)
