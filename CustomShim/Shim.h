#pragma once
#include "framework.h"
#include "apphelp.h"
#include <vector>

// This file declares the abstract Shim class and useful macros for implementing a shim.

template<typename F, F>
struct HookIndexHolder {
	static int index;
};

template<typename F, F f>
int typename HookIndexHolder<F, f>::index = -1;

#define SHIM_INSTANCE(SHIM_NAME) static Shim_##SHIM_NAME* instance = NULL; \
	Shim_##SHIM_NAME::Shim_##SHIM_NAME() : Shim(L#SHIM_NAME) { \
		instance = this; \
	}

#define ADD_HOOK(DLL, FUNCTION_NAME, HOOK_FUNCTION) do { \
		HookIndexHolder<decltype(HOOK_FUNCTION), HOOK_FUNCTION>::index = GetNextHookIndex(); \
		AddHook(DLL, FUNCTION_NAME, HOOK_FUNCTION); \
	} while (0)

#define DEFINE_NEXT(HOOK_FUNCTION) auto next = \
	(decltype(&HOOK_FUNCTION)) instance->GetNextProcAddress(HookIndexHolder<decltype(HOOK_FUNCTION), HOOK_FUNCTION>::index)

#define REQUIRE_COM_NEXT(HOOK_FUNCTION) auto next = (decltype(&HOOK_FUNCTION)) SE_COM_Lookup(pThis, HOOK_FUNCTION, __FUNCTION__); \
	if (!next) { \
		ASL_PRINTF(ASL_LEVEL_ERROR, "Failed to look up function replaced by " #HOOK_FUNCTION); \
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
	// Creates a Shim with the specified name. Shim subclasses should use the SHIM_INSTANCE macro instead of calling this manually.
	Shim(LPCWSTR setName);
	// During RegisterHooks, requests the hooking of an imported function. Shim subclasses should use the ADD_HOOK macro instead of calling this manually.
	void AddHook(LPCSTR dll, LPCSTR function, LPVOID hook);
	// During RegisterHooks, gets the next available imported function hook index. Shim subclasses should not need to use this manually.
	int GetNextHookIndex();
	// Gives the Shim a chance to validate its arguments before being installed. Returning false deactivates the shim.
	virtual bool ParseCommandLine(LPCSTR args);
	// Directs the Shim to request any desired imported function hooks using the ADD_HOOK macro.
	virtual void RegisterHooks();
	// Notifies the Shim of a process/engine lifecycle event. Called only after RegisterHooks.
	virtual void HandleNotification(DWORD notification, PVOID data);
	// Gets the real function (or next hook) replaced by the given hook. Shim subclasses should use the DEFINE_NEXT macro instead of calling this manually.
	PVOID GetNextProcAddress(int hookIndex);
private:
	// The shim's name as passed to SHIM_INSTANCE and expected in the SDB file.
	LPCWSTR name;
	// During RegisterHooks, the collection of imported function hooks requested so far.
	std::vector<HOOKAPI>* pendingHooks;
	// The API hooks block filled in by Install with information from AddHook, then populated with original/next function pointers by the shim engine.
	PHOOKAPI apiHooks;
};
