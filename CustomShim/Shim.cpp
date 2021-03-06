#include "Shim.h"
#include "dllmain.h"
#include <strsafe.h>

// This file implements the Shim base class.

Shim::Shim(LPCWSTR setName) {
	name = setName;
	pendingHooks = NULL;
	apiHooks = NULL;
	commandLine = NULL;
	shimId = -1;
}

LPCWSTR Shim::GetName() {
	return name;
}

PHOOKAPI Shim::Install(LPCSTR pszCommandLine, PDWORD pdwNumHooks) {
	if (pszCommandLine) {
		// The shim engine will free the command line string after calling GetHookAPIs, so make a copy
		size_t cch = strlen(pszCommandLine) + 1;
		LPSTR copied = new char[cch];
		StringCchCopyA(copied, cch, pszCommandLine);
		commandLine = copied;
	} else {
		commandLine = "";
	}
	if (!ParseCommandLine(commandLine)) {
		ASL_PRINTF(ASL_LEVEL_TRACE, "%S rejected command-line arguments", name);
		return NULL;
	}
	// Give the shim a chance to call AddHook (using the ADD_HOOK macro)
	pendingHooks = new std::vector<HOOKAPI>();
	RegisterHooks();
	// Copy the hooks vector into a C array
	PHOOKAPI pHookApi = new HOOKAPI[pendingHooks->size()];
	for (unsigned i = 0; i < pendingHooks->size(); i++) {
#pragma warning(push)
#pragma warning(disable: 6386) // The size()-based allocation confuses the overrun checker
		pHookApi[i] = (*pendingHooks)[i];
#pragma warning(pop)
	}
	*pdwNumHooks = (DWORD) pendingHooks->size();
	ASL_PRINTF(ASL_LEVEL_TRACE, "%S installed %d hooks", name, *pdwNumHooks);
	delete pendingHooks;
	pendingHooks = NULL;
	// Hold on to the array so the shim can get the NextFunction addresses after the shim engine sets them
	apiHooks = pHookApi;
	return pHookApi;
}

void Shim::Notify(DWORD notification, PVOID data) {
	if (notification == SE_NOTIFY_INIT) {
		shimId = SE_GetShimId(thisModule, name);
		ASL_PRINTF(ASL_LEVEL_TRACE, "%S has ID %d", name, shimId);
	}
	HandleNotification(notification, data);
}

void Shim::AddHook(LPCSTR dll, LPCSTR function, LPVOID hook) {
	if (!pendingHooks) {
		ASL_PRINTF(ASL_LEVEL_ERROR, "AddHook can only be called during RegisterHooks");
		return;
	}
	HOOKAPI hookInfo = { 0 };
	hookInfo.DllName = dll;
	hookInfo.FunctionName = function;
	hookInfo.HookFunction = hook;
	pendingHooks->push_back(hookInfo);
}

int Shim::GetNextHookIndex() {
	if (!pendingHooks) {
		ASL_PRINTF(ASL_LEVEL_ERROR, "GetNextHookIndex can only be called during RegisterHooks");
		return -1;
	}
	return (int) pendingHooks->size();
}

PVOID Shim::GetNextProcAddress(int hookIndex) {
	if (hookIndex < 0) {
		ASL_PRINTF(ASL_LEVEL_ERROR, "Invalid hook index %d, check ADD_HOOK and DEFINE_NEXT macro invocations", hookIndex);
		return NULL;
	}
	if (apiHooks) {
		PVOID next = apiHooks[hookIndex].NextFunction;
		if (!next) {
			ASL_PRINTF(ASL_LEVEL_ERROR, "apiHooks[%d].NextFunction not set yet", apiHooks);
		}
		return next;
	} else {
		ASL_PRINTF(ASL_LEVEL_ERROR, "apiHooks not initialized yet");
		return NULL;
	}
}

bool Shim::ParseCommandLine(LPCSTR args) {
	// By default, command-line arguments are ignored and the shim can load
	return true;
}

void Shim::RegisterHooks() {
	// No imported function hooks by default
}

void Shim::HandleNotification(DWORD notification, PVOID data) {
	// No response by default
}
