#include "Shim_AcceptEula.h"

// This file implements the AcceptEula shim.

// This macro defines the public zero-argument constructor and a static "instance" variable, for use by hook implementations.
SHIM_INSTANCE(AcceptEula)

void Shim_AcceptEula::RegisterHooks() {
	// Register hooks on both RegQueryValueExW and RegQueryValueExA
	// The macro keeps track of the index into the HOOKAPI array for later lookup of the original function by hook function instead of index
	ADD_HOOK("ADVAPI32.DLL", "RegQueryValueExW", Hook_RegQueryValueExW);
	ADD_HOOK("ADVAPI32.DLL", "RegQueryValueExA", Hook_RegQueryValueExA);
}

// Will be called when the program tries to call RegQueryValueExW.
LSTATUS WINAPI Shim_AcceptEula::Hook_RegQueryValueExW(HKEY hkey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData) {
	// This macro defines a "next" function pointer variable that can be used to call the original function or next hook
	DEFINE_NEXT(Hook_RegQueryValueExW);
	// Call the real RegQueryValueExW
	auto result = next(hkey, lpValueName, lpReserved, lpType, lpData, lpcbData);
	// Examine the results before allowing them to return to the program
	if (result == ERROR_FILE_NOT_FOUND && lpData != NULL && *lpcbData >= sizeof(DWORD) && wcscmp(lpValueName, L"EulaAccepted") == 0) {
		// If a value named "EulaAccepted" was looked up but not found, alter the results to indicate a DWORD value of 1
		*((LPDWORD) lpData) = 1;
		*lpcbData = sizeof(DWORD);
		if (lpType != NULL) *lpType = REG_DWORD;
		ASL_PRINTF(ASL_LEVEL_TRACE, "Fabricated EulaAccepted");
		return ERROR_SUCCESS;
	} else {
		// Pass the real result through
		ASL_PRINTF(ASL_LEVEL_TRACE, "Querying %S lpType(%p) lpData(%p) lpcbData(%p) returned %d", lpValueName, lpType, lpData, lpcbData, result);
		return result;
	}
	// Hooks need not call the original function; they could do something else entirely
}

// Likewise for RegQueryValueExA - the same as above, but with non-wide-string functions.
LSTATUS WINAPI Shim_AcceptEula::Hook_RegQueryValueExA(HKEY hkey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData) {
	DEFINE_NEXT(Hook_RegQueryValueExA);
	auto result = next(hkey, lpValueName, lpReserved, lpType, lpData, lpcbData);
	if (result == ERROR_FILE_NOT_FOUND && lpData != NULL && *lpcbData >= sizeof(DWORD) && strcmp(lpValueName, "EulaAccepted") == 0) {
		*((LPDWORD) lpData) = 1;
		*lpcbData = sizeof(DWORD);
		if (lpType != NULL) *lpType = REG_DWORD;
		ASL_PRINTF(ASL_LEVEL_TRACE, "Fabricated EulaAccepted");
		return ERROR_SUCCESS;
	} else {
		ASL_PRINTF(ASL_LEVEL_TRACE, "Querying %s lpType(%p) lpData(%p) lpcbData(%p) returned %d", lpValueName, lpType, lpData, lpcbData, result);
		return result;
	}
}
