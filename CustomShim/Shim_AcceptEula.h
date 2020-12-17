#pragma once
#include "Shim.h"

class Shim_AcceptEula : public Shim {
public:
	Shim_AcceptEula();
protected:
	virtual std::vector<HOOKAPI> GetHooks();
private:
	static LSTATUS WINAPI Hook_RegQueryValueExW(HKEY, LPCWSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD);
	static LSTATUS WINAPI Hook_RegQueryValueExA(HKEY, LPCSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD);
};
