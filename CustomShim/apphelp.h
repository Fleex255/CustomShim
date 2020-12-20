#pragma once
#include "framework.h"

// Functions defined in this file are imported from apphelp.dll using the apphelp.lib import library in the libs subfolder.
// Please see the apphelp project's README if you need to import other functions. (Alternatively, use GetProcAddress.)
#define APPHELP_API EXTERN_C DECLSPEC_IMPORT

// Log level constants for the first parameter to SE_ShimDPF.
#define ASL_LEVEL_PRINT 1
#define ASL_LEVEL_ERROR 2 // Will bring down the process
#define ASL_LEVEL_MARK 3
#define ASL_LEVEL_TRACE 4

// Notification codes for the first parameter to the NotifyShims function/callback.
#define SE_NOTIFY_INIT 1
#define SE_NOTIFY_PROCESS_DYING 2
#define SE_NOTIFY_DLL_LOAD 3
#define SE_NOTIFY_DLL_UNLOAD 4
#define SE_NOTIFY_LDR_ENTRY_REMOVED 105

// Represents a request for an imported function to be redirected to one in this DLL.
typedef struct tagHOOKAPI {
	PCSTR DllName;
	PCSTR FunctionName;
	PVOID HookFunction;
	PVOID NextFunction; // Populated by the shim engine
	PVOID Reserved[2];
} HOOKAPI, *PHOOKAPI;

// Hook a member function for all instances of a COM class that are created by a class factory.
// For instances created by other means, note the IID-plus-index to hook mapping for later application by SE_COM_HookObject.
// In the latter case, rcls can be CLSID_NULL.
APPHELP_API BOOL WINAPI SE_COM_AddHook(DWORD shimId, REFCLSID rcls, REFIID riid, DWORD vtblIndex, PVOID pfHook);
// Inform the shim engine that COM objects from the specified server DLL may be hooked.
APPHELP_API BOOL WINAPI SE_COM_AddServer(LPCWSTR pwszDll, BOOL exclude);
// Not clear - probably like SE_COM_HookObject except that the object reference need not be of the specified interface yet.
APPHELP_API BOOL WINAPI SE_COM_HookInterface(REFCLSID rcls, REFIID riid, PVOID* ppvObject, BOOL isClassFactory);
// Apply the specified interface's hooks, previously registered by SE_COM_AddHook, to all instances of the class of the given object.
// The two parameters of unknown significance can apparently be NULL and FALSE.
APPHELP_API BOOL WINAPI SE_COM_HookObject(PVOID unknown1, REFIID riid, PVOID* ppvObject, BOOL unknown2, BOOL isClassFactory);
// Retrieve the original function pointer corresponding to the given hook function pointer for the given object.
// The hook name is not actually used.
APPHELP_API PVOID WINAPI SE_COM_Lookup(PVOID pvObject, PVOID pfHook, LPCSTR pszHookName);
// Get the unique ID for an active shim belonging to the specified shim DLL.
APPHELP_API DWORD WINAPI SE_GetShimId(HMODULE hModule, LPCWSTR pwszShimName);
// Printf a message to the shims log. Commas in the format string will be mangled.
APPHELP_API VOID __cdecl SE_ShimDPF(DWORD logLevel, LPCSTR component, LPCSTR function, DWORD line, LPCSTR format, ...);

// More convenient wrapper for SE_ShimDPF.
#define ASL_PRINTF(LOG_LEVEL, FORMAT_STR, ...) SE_ShimDPF((LOG_LEVEL), __FILE__, __FUNCTION__, __LINE__, (FORMAT_STR), __VA_ARGS__)
