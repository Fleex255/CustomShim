#include "Shim_AcceptEula.h"

SHIM_INSTANCE(AcceptEula)

std::vector<HOOKAPI> Shim_AcceptEula::GetHooks() {
	std::vector<HOOKAPI> hooks;
	hooks.push_back(MakeHookInfo("ADVAPI32.DLL", "RegQueryValueExW", Hook_RegQueryValueExW));
	hooks.push_back(MakeHookInfo("ADVAPI32.DLL", "RegQueryValueExA", Hook_RegQueryValueExA));
	return hooks;
}

LSTATUS WINAPI Shim_AcceptEula::Hook_RegQueryValueExW(HKEY hkey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData) {
	auto next = (LSTATUS(WINAPI*)(HKEY, LPCWSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD)) instance->GetNextProcAddress(0);
	auto result = next(hkey, lpValueName, lpReserved, lpType, lpData, lpcbData);
	if (result == ERROR_FILE_NOT_FOUND && lpData != NULL && *lpcbData >= sizeof(DWORD) && wcscmp(lpValueName, L"EulaAccepted") == 0) {
		*((LPDWORD) lpData) = 1;
		*lpcbData = sizeof(DWORD);
		if (lpType != NULL) *lpType = REG_DWORD;
		ASL_PRINTF(ASL_LEVEL_TRACE, "Fabricated EulaAccepted");
		return ERROR_SUCCESS;
	} else {
		ASL_PRINTF(ASL_LEVEL_TRACE, "Querying %S lpType(%p) lpData(%p) lpcbData(%p) returned %d", lpValueName, lpType, lpData, lpcbData, result);
		return result;
	}
}

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
