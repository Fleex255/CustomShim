#include "Shim_AcceptEula.h"

// This file implements the AcceptEula shim.

// This macro defines the public zero-argument constructor and a static "instance" variable, for use by hook implementations.
SHIM_INSTANCE(AcceptEula)

std::vector<HOOKAPI> Shim_AcceptEula::GetHooks() {
	std::vector<HOOKAPI> hooks;
	// Register hooks on both RegQueryValueExW and RegQueryValueExA
	// Order matters for looking up the original function with GetNextProcAddress
	hooks.push_back(MakeHookInfo("ADVAPI32.DLL", "RegQueryValueExW", Hook_RegQueryValueExW)); // Hook 0
	hooks.push_back(MakeHookInfo("ADVAPI32.DLL", "RegQueryValueExA", Hook_RegQueryValueExA)); // Hook 1
	return hooks;
}

// Will be called when the program tries to call RegQueryValueExW.
LSTATUS WINAPI Shim_AcceptEula::Hook_RegQueryValueExW(HKEY hkey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData) {
	// Get and call the real RegQueryValueExW
	auto next = (LSTATUS(WINAPI*)(HKEY, LPCWSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD)) instance->GetNextProcAddress(0);
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
	auto next = (LSTATUS(WINAPI*)(HKEY, LPCSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD)) instance->GetNextProcAddress(1);
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
