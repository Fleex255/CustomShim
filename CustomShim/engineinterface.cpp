#include "apphelp.h"
#include <winternl.h>
#include "shimlist.h"

std::vector<Shim*>* activeShims = NULL;

EXTERN_C PHOOKAPI WINAPI GetHookAPIs(LPSTR pszArgs, LPWSTR pwszShim, PDWORD pdwHookCount) {
	if (!pwszShim) {
		ASL_PRINTF(ASL_LEVEL_MARK, "pwszShim is NULL");
		return NULL;
	}
	ASL_PRINTF(ASL_LEVEL_TRACE, "%S; args = %s", pwszShim, pszArgs);
	EnsureShimsInstantiated();
	if (!activeShims) {
		activeShims = new std::vector<Shim*>();
	}
	for (Shim* shim : *knownShims) {
		if (_wcsicmp(pwszShim, shim->GetName()) == 0) {
			PHOOKAPI apiHooks = shim->Install(pszArgs, pdwHookCount);
			if (apiHooks == NULL && *pdwHookCount == FAIL_SHIM_LOAD) {
				*pdwHookCount = 0;
				return NULL;
			}
			activeShims->push_back(shim);
			return apiHooks;
		}
	}
	ASL_PRINTF(ASL_LEVEL_TRACE, "Requested shim is not known");
	return NULL;
}

EXTERN_C VOID WINAPI NotifyShims(DWORD notification, PVOID data) {
	switch (notification) {
		case SE_NOTIFY_INIT:
			ASL_PRINTF(ASL_LEVEL_TRACE, "Shim engine initialized");
			break;
		case SE_NOTIFY_DLL_LOAD:
			ASL_PRINTF(ASL_LEVEL_TRACE, "DLL loaded: %S", ((PLDR_DATA_TABLE_ENTRY) data)->FullDllName.Buffer);
			break;
		default:
			ASL_PRINTF(ASL_LEVEL_TRACE, "notification(%d) data(%p)", notification, data);
	}
	if (activeShims) {
		for (Shim* shim : *activeShims) {
			shim->Notify(notification, data);
		}
	}
}
