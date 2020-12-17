#pragma once
#include "Shim.h"
#include <taskschd.h>

class Shim_FakeSchTask : public Shim {
public:
	Shim_FakeSchTask();
	virtual void Notify(DWORD notification, PVOID data);
protected:
	virtual std::vector<HOOKAPI> GetHooks();
private:
	IRegisteredTaskCollection* rootFolderTasks = NULL;
	IRegisteredTask* fakeTask = NULL;
	ITaskDefinition* fakeTaskDefinition = NULL;
	static HRESULT STDMETHODCALLTYPE Hook_ITaskService_GetFolder(ITaskService* pThis, BSTR path, ITaskFolder** ppResult);
	static HRESULT STDMETHODCALLTYPE Hook_ITaskFolder_GetTask(ITaskFolder* pThis, BSTR path, IRegisteredTask** ppResult);
	static HRESULT STDMETHODCALLTYPE Hook_ITaskFolder_GetTasks(ITaskFolder* pThis, LONG flags, IRegisteredTaskCollection** ppResult);
	static HRESULT STDMETHODCALLTYPE Hook_IRegisteredTaskCollection_getCount(IRegisteredTaskCollection* pThis, LONG* pCount);
	static HRESULT STDMETHODCALLTYPE Hook_IRegisteredTaskCollection_getItem(IRegisteredTaskCollection* pThis, VARIANT index, IRegisteredTask** ppResult);
};
