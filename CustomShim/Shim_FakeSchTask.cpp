#include "Shim_FakeSchTask.h"
#include "FakeSchTask_fakes.hpp"

// This file implements the FakeSchTask shim.

SHIM_INSTANCE(FakeSchTask)

bool Shim_FakeSchTask::ParseCommandLine(LPCSTR args) {
	// The backslash character is a folder separator in scheduled task paths - reject the command line if the name includes one
	return strstr(args, "\\") == NULL;
	// Arguments (the fake task's name, in this case) are saved in the commandLine field by the base class
}

void Shim_FakeSchTask::HandleNotification(DWORD notification, PVOID data) {
	// Only interested in the "shim engine fully initialized" notification...
	if (notification != SE_NOTIFY_INIT) return;
	// ...because at that point, the shim ID is known (protected shimId field is set) and COM hooks can be installed
	// The Task Scheduler 2.0 API is implemented by taskschd.dll - this can be found by searching for the CLSID in the Registry
	SE_COM_AddServer(L"TASKSCHD.DLL", FALSE);
	// Autoruns calls ITaskService::GetFolder to get the ITaskFolder corresponding to the root, then recurses into its children
	// Ask the shim engine to hook all CoCreateInstance results for CLSID_TaskScheduler, replacing the GetFolder function
	// Vtable indexes can be found by counting to the position of the desired function in the CINTERFACE [interface]Vtbl struct in headers
	// Getting the index wrong will cause a different function to be hooked, probably leading to stack corruption and a crash
	if (!SE_COM_AddHook(shimId, CLSID_TaskScheduler, IID_ITaskService, 7, Hook_ITaskService_GetFolder)) {
		ASL_PRINTF(ASL_LEVEL_MARK, "SE_COM_AddHook failed for ITaskService::GetFolder");
	}
	// To list the scheduled tasks in each folder, Autoruns calls ITaskFolder::GetTasks, or ITaskFolder::GetTask to manipulate a specific one
	// ITaskFolder instances are created and returned by taskschd.dll, never externally, so they don't have a registered CLSID
	// These hooks will have to be activated by SE_COM_HookObject once an instance is accessible
	if (!SE_COM_AddHook(shimId, CLSID_NULL, IID_ITaskFolder, 13, Hook_ITaskFolder_GetTask)) {
		ASL_PRINTF(ASL_LEVEL_MARK, "SE_COM_AddHook failed for ITaskFolder::GetTask");
	}
	if (!SE_COM_AddHook(shimId, CLSID_NULL, IID_ITaskFolder, 14, Hook_ITaskFolder_GetTasks)) {
		ASL_PRINTF(ASL_LEVEL_MARK, "SE_COM_AddHook failed for ITaskFolder::GetTasks");
	}
}

HRESULT STDMETHODCALLTYPE Shim_FakeSchTask::Hook_ITaskService_GetFolder(ITaskService* pThis, BSTR path, ITaskFolder** ppResult) {
	if (!instance->fakeTask) {
		// ITaskDefinition has numerous properties that would be tedious to fake, so take advantage of the ITaskService to make a real definition
		ITaskDefinition* fakeTaskDefinition = NULL;
		pThis->NewTask(0, &fakeTaskDefinition);
		IActionCollection* pActions = NULL;
		fakeTaskDefinition->get_Actions(&pActions);
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
		// But don't use the definition to register a real task, just give it to the fake IRegisteredTask
		LPCSTR taskName = strlen(instance->commandLine) > 0 ? instance->commandLine : "Fake Task"; // Default task name if no command line given
		instance->fakeTask = new FakeScheduledTask(taskName, fakeTaskDefinition);
		ASL_PRINTF(ASL_LEVEL_TRACE, "Prepared fake task and definition");
	}
	// The REQUIRE_NEXT macro takes a function signature and pointer, and initializes a "next" variable pointing to the corresponding real function
	REQUIRE_NEXT(HRESULT(STDMETHODCALLTYPE*)(ITaskService*, BSTR, ITaskFolder**), Hook_ITaskService_GetFolder);
	// Call the real ITaskService::GetFolder
	HRESULT result = next(pThis, path, ppResult);
	ASL_PRINTF(ASL_LEVEL_TRACE, "ITaskService::GetFolder returned %d for %S", result, path);
	if (SUCCEEDED(result)) {
		// Install the ITaskFolder hooks on the class of the resulting object
		SE_COM_HookObject(NULL, IID_ITaskFolder, (PVOID*) ppResult, FALSE, FALSE);
	}
	return result;
}

HRESULT STDMETHODCALLTYPE Shim_FakeSchTask::Hook_ITaskFolder_GetTask(ITaskFolder* pThis, BSTR path, IRegisteredTask** ppResult) {
	// A single task is being requested by path - check to see whether it's the fake
	BSTR fakePath = NULL;
	instance->fakeTask->get_Path(&fakePath);
	BOOL isFake = _wcsicmp(fakePath, path) == 0;
	SysFreeString(fakePath);
	if (isFake && ppResult) {
		// If so, just return the fake task instance - the real ITaskFolder doesn't know about it
		instance->fakeTask->AddRef();
		*ppResult = instance->fakeTask;
		ASL_PRINTF(ASL_LEVEL_TRACE, "Returning fake task");
		return S_OK;
	} else {
		// If not, pass the request along to the real ITaskFolder
		REQUIRE_NEXT(HRESULT(STDMETHODCALLTYPE*)(ITaskFolder*, BSTR, IRegisteredTask**), Hook_ITaskFolder_GetTask);
		return next(pThis, path, ppResult);
	}
}

HRESULT STDMETHODCALLTYPE Shim_FakeSchTask::Hook_ITaskFolder_GetTasks(ITaskFolder* pThis, LONG flags, IRegisteredTaskCollection** ppResult) {
	// Even though Autoruns only uses ITaskService::GetFolder to get the root folder, SE_COM_HookObject affects all instances of this ITaskFolder implementation
	// To check whether this ITaskFolder corresponds to the root folder, get its name for comparison against "\"
	BSTR folderName = NULL;
	HRESULT result = pThis->get_Name(&folderName);
	if (FAILED(result)) return result;
	// Call the real ITaskFolder::GetTasks to obtain the real collection of registered tasks in this folder
	REQUIRE_NEXT(HRESULT(STDMETHODCALLTYPE*)(ITaskFolder*, LONG, IRegisteredTaskCollection**), Hook_ITaskFolder_GetTasks);
	result = next(pThis, flags, ppResult);
	ASL_PRINTF(ASL_LEVEL_TRACE, "ITaskFolder::GetTasks returned %d for %S; flags(%d) pResult(%p)", result, folderName, flags, (void*) *ppResult);
	if (SUCCEEDED(result) && wcscmp(folderName, L"\\") == 0) {
		// Wrap the root folder's IRegisteredTaskCollection in a FakeTaskCollection that will insert the fake task
		// instance->fakeTask was set in the ITaskService::GetFolder hook, which is the only way for the caller to get a first ITaskFolder
		// The FakeTaskCollection takes ownership of the real IRegisteredTaskCollection and will Release it when destroyed
		*ppResult = new FakeTaskCollection(*ppResult, instance->fakeTask);
		ASL_PRINTF(ASL_LEVEL_TRACE, "Wrapping in FakeTaskCollection %p", (void*) *ppResult);
	}
	SysFreeString(folderName);
	return result;
}
