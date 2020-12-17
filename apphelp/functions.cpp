#include "framework.h"

#define APPHELP_API EXTERN_C __declspec(dllexport)

APPHELP_API BOOL WINAPI SE_COM_AddHook(DWORD hookId, REFCLSID rcls, REFIID riid, DWORD vtblIndex, PVOID pfHook) { return 0; }
APPHELP_API BOOL WINAPI SE_COM_AddServer(LPCWSTR pwszDll, BOOL exclude) { return 0; }
APPHELP_API BOOL WINAPI SE_COM_HookInterface(REFCLSID rcls, REFIID riid, PVOID* ppvObject, BOOL isClassFactory) { return 0; }
APPHELP_API BOOL WINAPI SE_COM_HookObject(PVOID unknown1, REFIID riid, PVOID* ppvObject, BOOL unknown2, BOOL isClassFactory) { return 0; }
APPHELP_API PVOID WINAPI SE_COM_Lookup(PVOID pvObject, PVOID pfHook, LPCSTR pszHookName) { return 0; }
APPHELP_API DWORD WINAPI SE_GetShimId(HMODULE hModule, LPCWSTR pwszShimName) { return 0; }
APPHELP_API VOID __cdecl SE_ShimDPF(DWORD logLevel, LPCSTR component, LPCSTR function, DWORD line, LPCSTR format, ...) {}
