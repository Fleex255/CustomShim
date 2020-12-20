#pragma once
#include "Shim.h"

// AcceptEula is a simple shim that makes Sysinternals utilities always see the Registry value that indicates their EULA has been accepted.
// It demonstrates hooking imported functions.

// Shim classes should be named Shim_[shim name as expected in SDB] to work with the SHIM_INSTANCE macro.
class Shim_AcceptEula : public Shim {
public:
	// Shims should declare a public no-argument constructor, to be defined by the SHIM_INSTANCE macro.
	Shim_AcceptEula();
protected:
	// All shims must implement GetHooks so they can be asked what imported functions they would like to hook.
	virtual std::vector<HOOKAPI> GetHooks();
private:
	// Hook functions must be static. The calling conventions and parameter sizes must match the corresponding real functions exactly.
	static LSTATUS WINAPI Hook_RegQueryValueExW(HKEY, LPCWSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD);
	static LSTATUS WINAPI Hook_RegQueryValueExA(HKEY, LPCSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD);
};
