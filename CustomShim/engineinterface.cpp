#include "apphelp.h"
#include <winternl.h>
#include "shimlist.h"

// This file is the main point of contact between the shim engine and this shim module.

// The vector of shims that were requested by the shim engine and loaded successfully.
// It must be a pointer instead of constructed immediately due to the concerns noted in shimlist.cpp.
std::vector<Shim*>* activeShims = NULL;

// Called by the shim engine to request the installation of a shim.
// Returns an array of requested import hooks and provides its length in the pdwHookCount output parameter.
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
	// Find the requested shim in the vector of known shims (shimlist.cpp)
	for (Shim* shim : *knownShims) {
		if (_wcsicmp(pwszShim, shim->GetName()) == 0) {
			// Install can fail if the shim doesn't like the command line it was given
			PHOOKAPI apiHooks = shim->Install(pszArgs, pdwHookCount);
			if (apiHooks == NULL) {
				*pdwHookCount = 0;
				return NULL;
			}
			// If the shim was found and approved of its command line, mark it as active and return its requested hooks
			activeShims->push_back(shim);
			return apiHooks;
		}
	}
	ASL_PRINTF(ASL_LEVEL_TRACE, "Requested shim is not known");
	return NULL;
}

// Called by the shim engine at various points in the process's life cycle.
EXTERN_C VOID WINAPI NotifyShims(DWORD notification, PVOID data) {
	switch (notification) {
		case SE_NOTIFY_INIT:
			// All hooks have been installed and all statically imported DLLs have been loaded
			ASL_PRINTF(ASL_LEVEL_TRACE, "Shim engine initialized");
			break;
		case SE_NOTIFY_DLL_LOAD:
			ASL_PRINTF(ASL_LEVEL_TRACE, "DLL loaded: %S", ((PLDR_DATA_TABLE_ENTRY) data)->FullDllName.Buffer);
			break;
		default:
			ASL_PRINTF(ASL_LEVEL_TRACE, "notification(%d) data(%p)", notification, data);
	}
	// Pass the notification on to all active shims
	if (activeShims) {
		for (Shim* shim : *activeShims) {
			shim->Notify(notification, data);
		}
	}
}
