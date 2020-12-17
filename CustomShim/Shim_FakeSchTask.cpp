#include "Shim_FakeSchTask.h"
#include "FakeScheduledTask.hpp"
#include <string>

SHIM_INSTANCE(FakeSchTask)

void Shim_FakeSchTask::Notify(DWORD notification, PVOID data) {
	if (notification != SE_NOTIFY_INIT) return;
	SE_COM_AddServer(L"TASKSCHD.DLL", FALSE);
	if (!SE_COM_AddHook(shimId, CLSID_TaskScheduler, IID_ITaskService, 7, Hook_ITaskService_GetFolder)) {
		ASL_PRINTF(ASL_LEVEL_MARK, "SE_COM_AddHook failed for ITaskService::GetFolder");
	}
	if (!SE_COM_AddHook(shimId, CLSID_NULL, IID_ITaskFolder, 13, Hook_ITaskFolder_GetTask)) {
		ASL_PRINTF(ASL_LEVEL_MARK, "SE_COM_AddHook failed for ITaskFolder::GetTask");
	}
	if (!SE_COM_AddHook(shimId, CLSID_NULL, IID_ITaskFolder, 14, Hook_ITaskFolder_GetTasks)) {
		ASL_PRINTF(ASL_LEVEL_MARK, "SE_COM_AddHook failed for ITaskFolder::GetTasks");
	}
	if (!SE_COM_AddHook(shimId, CLSID_NULL, IID_IRegisteredTaskCollection, 7, Hook_IRegisteredTaskCollection_getCount)) {
		ASL_PRINTF(ASL_LEVEL_MARK, "SE_COM_AddHook failed for IRegisteredTaskCollection::get_Count");
	}
	if (!SE_COM_AddHook(shimId, CLSID_NULL, IID_IRegisteredTaskCollection, 8, Hook_IRegisteredTaskCollection_getItem)) {
		ASL_PRINTF(ASL_LEVEL_MARK, "SE_COM_AddHook failed for IRegisteredTaskCollection::get_Item");
	}
}

std::vector<HOOKAPI> Shim_FakeSchTask::GetHooks() {
	return std::vector<HOOKAPI>();
}

HRESULT STDMETHODCALLTYPE Shim_FakeSchTask::Hook_ITaskService_GetFolder(ITaskService* pThis, BSTR path, ITaskFolder** ppResult) {
	if (!instance->fakeTaskDefinition) {
		pThis->NewTask(0, &instance->fakeTaskDefinition);
		IActionCollection* pActions = NULL;
		instance->fakeTaskDefinition->get_Actions(&pActions);
		IAction* pAction = NULL;
		pActions->Create(TASK_ACTION_EXEC, &pAction);
		pActions->Release();
		IExecAction* pExecAction = NULL;
		pAction->QueryInterface(&pExecAction);
		pAction->Release();
		BSTR path = SysAllocString(L"C:\\fake.exe");
		pExecAction->put_Path(path);
		SysFreeString(path);
		pExecAction->Release();
		LPCSTR taskName = strlen(instance->commandLine) > 0 ? instance->commandLine : "Fake Task";
		instance->fakeTask = new FakeScheduledTask(taskName, instance->fakeTaskDefinition);
		ASL_PRINTF(ASL_LEVEL_TRACE, "Prepared fake task and definition");
	}
	REQUIRE_NEXT(HRESULT(STDMETHODCALLTYPE*)(ITaskService*, BSTR, ITaskFolder**), Hook_ITaskService_GetFolder);
	HRESULT result = next(pThis, path, ppResult);
	ASL_PRINTF(ASL_LEVEL_TRACE, "ITaskService::GetFolder returned %d for %S", result, path);
	if (SUCCEEDED(result)) {
		SE_COM_HookObject(NULL, IID_ITaskFolder, (PVOID*) ppResult, FALSE, FALSE);
	}
	return result;
}

HRESULT STDMETHODCALLTYPE Shim_FakeSchTask::Hook_ITaskFolder_GetTask(ITaskFolder* pThis, BSTR path, IRegisteredTask** ppResult) {
	BSTR fakePath = NULL;
	instance->fakeTask->get_Path(&fakePath);
	BOOL isFake = _wcsicmp(fakePath, path) == 0;
	SysFreeString(fakePath);
	if (isFake && ppResult) {
		instance->fakeTask->AddRef();
		*ppResult = instance->fakeTask;
		ASL_PRINTF(ASL_LEVEL_TRACE, "Returning fake task");
		return S_OK;
	} else {
		REQUIRE_NEXT(HRESULT(STDMETHODCALLTYPE*)(ITaskFolder*, BSTR, IRegisteredTask**), Hook_ITaskFolder_GetTask);
		return next(pThis, path, ppResult);
	}
}

HRESULT STDMETHODCALLTYPE Shim_FakeSchTask::Hook_ITaskFolder_GetTasks(ITaskFolder* pThis, LONG flags, IRegisteredTaskCollection** ppResult) {
	REQUIRE_NEXT(HRESULT(STDMETHODCALLTYPE*)(ITaskFolder*, LONG, IRegisteredTaskCollection**), Hook_ITaskFolder_GetTasks);
	HRESULT result = next(pThis, flags, ppResult);
	BSTR folderName = NULL;
	pThis->get_Name(&folderName);
	ASL_PRINTF(ASL_LEVEL_TRACE, "ITaskFolder::GetTasks returned %d for %S; pThis(%p) flags(%d) pResult(%p)", result, folderName, (void*) pThis, flags, (void*) *ppResult);
	if (SUCCEEDED(result)) {
		if (wcscmp(folderName, L"\\") == 0) {
			instance->rootFolderTasks = *ppResult;
			SE_COM_HookObject(NULL, IID_IRegisteredTaskCollection, (PVOID*) ppResult, FALSE, FALSE);
		} else if (*ppResult == instance->rootFolderTasks) {
			instance->rootFolderTasks = NULL;
		}
	}
	SysFreeString(folderName);
	return result;
}

HRESULT STDMETHODCALLTYPE Shim_FakeSchTask::Hook_IRegisteredTaskCollection_getCount(IRegisteredTaskCollection* pThis, LONG* pCount) {
	REQUIRE_NEXT(HRESULT(STDMETHODCALLTYPE*)(IRegisteredTaskCollection*, LONG*), Hook_IRegisteredTaskCollection_getCount);
	HRESULT result = next(pThis, pCount);
	if (SUCCEEDED(result) && pThis == instance->rootFolderTasks) {
		ASL_PRINTF(ASL_LEVEL_TRACE, "Root IRegisteredTaskCollection::get_Count returned %d; count(%d) pThis(%p); will increment", result, *pCount, (void*) pThis);
		(*pCount)++;
	}
	return result;
}

HRESULT STDMETHODCALLTYPE Shim_FakeSchTask::Hook_IRegisteredTaskCollection_getItem(IRegisteredTaskCollection* pThis, VARIANT index, IRegisteredTask** ppResult) {
	REQUIRE_NEXT(HRESULT(STDMETHODCALLTYPE*)(IRegisteredTaskCollection*, VARIANT, IRegisteredTask**), Hook_IRegisteredTaskCollection_getItem);
	if (pThis == instance->rootFolderTasks) {
		if (FAILED(VariantChangeType(&index, &index, VARIANT_NOVALUEPROP, VT_I4))) {
			ASL_PRINTF(ASL_LEVEL_MARK, "Could not coerce index of type %d to I4", index.vt);
			return E_INVALIDARG;
		}
		if (index.lVal == 1) {
			instance->fakeTask->AddRef();
			*ppResult = instance->fakeTask;
			ASL_PRINTF(ASL_LEVEL_TRACE, "Returning fake task for item #1");
			return S_OK;
		} else {
			ASL_PRINTF(ASL_LEVEL_TRACE, "Forwarding request for #%d to real #%d", index.lVal, index.lVal - 1);
			index.lVal--;
			return next(pThis, index, ppResult);
		}
	} else {
		return next(pThis, index, ppResult);
	}
}
