#pragma once
#include "Shim.h"
#include <taskschd.h>

// FakeSchTask is a somewhat more complex shim that makes Sysinternals Autoruns and AutorunsC see a scheduled task in the root folder that isn't there.
// The fake task's name can be specified as the shim's command line, or left as a default. This shim demonstrates COM hooks and command line validation.

class Shim_FakeSchTask : public Shim {
public:
	Shim_FakeSchTask();
protected:
	virtual bool ParseCommandLine(LPCSTR args);
	virtual std::vector<HOOKAPI> GetHooks();
	virtual void HandleNotification(DWORD notification, PVOID data);
private:
	// This shim's implementation will cache just one task, reusing it if needed.
	IRegisteredTask* fakeTask = NULL;
	// When represented as non-instance functions, COM functions take a leading "this" pointer parameter.
	static HRESULT STDMETHODCALLTYPE Hook_ITaskService_GetFolder(ITaskService* pThis, BSTR path, ITaskFolder** ppResult);
	static HRESULT STDMETHODCALLTYPE Hook_ITaskFolder_GetTask(ITaskFolder* pThis, BSTR path, IRegisteredTask** ppResult);
	static HRESULT STDMETHODCALLTYPE Hook_ITaskFolder_GetTasks(ITaskFolder* pThis, LONG flags, IRegisteredTaskCollection** ppResult);
};
