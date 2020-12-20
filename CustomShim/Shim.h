#pragma once
#include "framework.h"
#include "apphelp.h"
#include <vector>

// This file declares the abstract Shim class and useful macros for implementing a shim.

#define SHIM_INSTANCE(SHIM_NAME) static Shim_##SHIM_NAME* instance = NULL; \
	Shim_##SHIM_NAME::Shim_##SHIM_NAME() : Shim(L#SHIM_NAME) { \
		instance = this; \
	}

#define REQUIRE_NEXT(FPTR_TYPE, CURRENT_FUNCTION) auto next = (FPTR_TYPE) SE_COM_Lookup(pThis, CURRENT_FUNCTION, __FUNCTION__); \
	if (!next) { \
		ASL_PRINTF(ASL_LEVEL_ERROR, "Failed to look up original " #FPTR_TYPE); \
		return E_NOTIMPL; \
	}

class Shim {
public:
	Shim(const Shim&) = delete;
	Shim& operator=(const Shim&) = delete;
	virtual ~Shim() = default;
	// Public methods are used by the engine interface. These call protected virtual methods as appropriate.
	LPCWSTR GetName();
	PHOOKAPI Install(LPCSTR pszCommandLine, PDWORD pdwNumHooks);
	void Notify(DWORD notification, PVOID data);
protected:
	// The command line given to the shim in the SDB, or empty (not null) if none was given. Initialized before any protected methods are called.
	LPCSTR commandLine;
	// The shim's unique ID, needed for COM hooks. Initialized immediately before HandleNotification is called for SE_NOTIFY_INIT, -1 before then.
	DWORD shimId;
	// Helpful function to create a HOOKAPI instance for registering an import hook.
	static HOOKAPI MakeHookInfo(LPCSTR dll, LPCSTR function, LPVOID hook);
	// Creates a Shim with the specified name. Shim subclasses should use the SHIM_INSTANCE macro instead of calling this manually.
	Shim(LPCWSTR setName);
	// Gives the Shim a chance to validate its arguments before being installed. Returning false deactivates the shim.
	virtual bool ParseCommandLine(LPCSTR args);
	// Asks the Shim which imported functions it would like to hook.
	virtual std::vector<HOOKAPI> GetHooks() = 0;
	// Notifies the Shim of a process/engine lifecycle event. Called only after GetHooks.
	virtual void HandleNotification(DWORD notification, PVOID data);
	// Gets the real function (or next hook) replaced by the given hook. The index must match the position used in GetHooks.
	PVOID GetNextProcAddress(int hookIndex);
private:
	// The shim's name as passed to SHIM_INSTANCE and expected in the SDB file.
	LPCWSTR name;
	// The API hooks block filled in by Install with information from GetHooks, then populated with original function pointers by the shim engine.
	PHOOKAPI apiHooks;
};
