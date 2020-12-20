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
	// RegQueryValueEx[A/W] are imported Win32 functions, so they need to be hooked by giving the shim engine HOOKAPI structures.
	virtual void RegisterHooks();
private:
	// Hook functions must be static. The calling conventions and parameter sizes must match the corresponding real functions exactly.
	static LSTATUS WINAPI Hook_RegQueryValueExW(HKEY, LPCWSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD);
	static LSTATUS WINAPI Hook_RegQueryValueExA(HKEY, LPCSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD);
};
