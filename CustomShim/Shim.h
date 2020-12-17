#pragma once
#include "framework.h"
#include "apphelp.h"
#include <vector>

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
	Shim(LPCWSTR setName);
	Shim(const Shim&) = delete;
	Shim& operator=(const Shim&) = delete;
	LPCWSTR GetName();
	PHOOKAPI Install(LPCSTR pszCommandLine, PDWORD pdwNumHooks);
	virtual void Notify(DWORD notification, PVOID data);
	virtual ~Shim() = default;
protected:
	LPCSTR commandLine;
	DWORD shimId;
	static HOOKAPI MakeHookInfo(LPCSTR dll, LPCSTR function, LPVOID hook);
	virtual bool ParseCommandLine(LPCSTR args);
	PVOID GetNextProcAddress(int hookIndex);
	virtual std::vector<HOOKAPI> GetHooks() = 0;
private:
	LPCWSTR name;
	PHOOKAPI apiHooks;
};
