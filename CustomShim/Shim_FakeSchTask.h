#pragma once
#include "Shim.h"
#include <taskschd.h>

class Shim_FakeSchTask : public Shim {
public:
	Shim_FakeSchTask();
	virtual void HandleNotification(DWORD notification, PVOID data);
protected:
	virtual bool ParseCommandLine(LPCSTR args);
	virtual std::vector<HOOKAPI> GetHooks();
private:
	IRegisteredTask* fakeTask = NULL;
	ITaskDefinition* fakeTaskDefinition = NULL;
	static HRESULT STDMETHODCALLTYPE Hook_ITaskService_GetFolder(ITaskService* pThis, BSTR path, ITaskFolder** ppResult);
	static HRESULT STDMETHODCALLTYPE Hook_ITaskFolder_GetTask(ITaskFolder* pThis, BSTR path, IRegisteredTask** ppResult);
	static HRESULT STDMETHODCALLTYPE Hook_ITaskFolder_GetTasks(ITaskFolder* pThis, LONG flags, IRegisteredTaskCollection** ppResult);
};
