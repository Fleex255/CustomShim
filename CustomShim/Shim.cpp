#include "Shim.h"
#include "dllmain.h"
#include <strsafe.h>

// This file implements the Shim base class.

Shim::Shim(LPCWSTR setName) {
	name = setName;
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
		int cch = strlen(pszCommandLine) + 1;
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
	// Copy the hooks vector into a C array
	auto hooks = GetHooks();
	PHOOKAPI pHookApi = new HOOKAPI[hooks.size()];
	for (unsigned i = 0; i < hooks.size(); i++) {
#pragma warning(push)
#pragma warning(disable: 6386) // The size()-based allocation confuses the overrun checker
		pHookApi[i] = hooks[i];
#pragma warning(pop)
	}
	*pdwNumHooks = hooks.size();
	ASL_PRINTF(ASL_LEVEL_TRACE, "%S installed %d hooks", name, *pdwNumHooks);
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

HOOKAPI Shim::MakeHookInfo(LPCSTR dll, LPCSTR function, LPVOID hook) {
	HOOKAPI hookInfo = { 0 };
	hookInfo.DllName = dll;
	hookInfo.FunctionName = function;
	hookInfo.HookFunction = hook;
	return hookInfo;
}

PVOID Shim::GetNextProcAddress(int hookIndex) {
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

std::vector<HOOKAPI> Shim::GetHooks() {
	// No imported function hooks by default
	return std::vector<HOOKAPI>();
}

void Shim::HandleNotification(DWORD notification, PVOID data) {
	// No response by default
}
