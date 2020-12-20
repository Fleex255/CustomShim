#pragma once
#include <taskschd.h>
#include "comstubs.hpp"

class FakeTaskCollection : public DispatchStub<IRegisteredTaskCollection, IID_IRegisteredTaskCollection> {
private:
	IRegisteredTaskCollection* realCollection;
	IRegisteredTask* fakeTask;
public:
	FakeTaskCollection(IRegisteredTaskCollection* setNext, IRegisteredTask* setFakeTask) {
		realCollection = setNext;
		fakeTask = setFakeTask;
	}
	~FakeTaskCollection() override {
		realCollection->Release();
	}
	IFACEMETHOD(get_Count)(LONG* pResult) override {
		HRESULT result = realCollection->get_Count(pResult);
		if (SUCCEEDED(result)) {
			ASL_PRINTF(ASL_LEVEL_TRACE, "Real IRegisteredTaskCollection::get_Count returned %d; will increment count(%d)", result, *pResult);
			*pResult += 1;
		}
		return result;
	}
	IFACEMETHOD(get_Item)(VARIANT index, IRegisteredTask** ppResult) override {
		if (!ppResult) return E_INVALIDARG;
		if (FAILED(VariantChangeType(&index, &index, VARIANT_NOVALUEPROP, VT_I4))) {
			ASL_PRINTF(ASL_LEVEL_MARK, "Could not coerce index of type %d to I4", index.vt);
			*ppResult = NULL;
			return E_INVALIDARG;
		}
		if (index.lVal == 1) {
			fakeTask->AddRef();
			*ppResult = fakeTask;
			ASL_PRINTF(ASL_LEVEL_TRACE, "Returning fake task for item #1");
			return S_OK;
		} else {
			ASL_PRINTF(ASL_LEVEL_TRACE, "Forwarding request for #%d to real #%d", index.lVal, index.lVal - 1);
			index.lVal--;
			return realCollection->get_Item(index, ppResult);
		}
	}
	IFACEMETHOD(get__NewEnum)(IUnknown** ppResult) override {
		if (ppResult) *ppResult = NULL;
		return ppResult ? E_NOTIMPL : E_INVALIDARG;
	}
};

class FakeScheduledTask : public DispatchStub<IRegisteredTask, IID_IRegisteredTask> {
private:
	LPWSTR fakeTaskPath;
	ITaskDefinition* fakeTaskDefinition;
public: 
	FakeScheduledTask(LPCSTR setName, ITaskDefinition* setTaskDefinition) {
		fakeTaskDefinition = setTaskDefinition;
		int neededChars = MultiByteToWideChar(CP_UTF8, 0, setName, -1, NULL, 0);
		fakeTaskPath = new wchar_t[neededChars + 1];
		fakeTaskPath[0] = '\\';
		MultiByteToWideChar(CP_UTF8, 0, setName, -1, &fakeTaskPath[1], neededChars);
	}
	~FakeScheduledTask() {
		delete[] fakeTaskPath;
	}
	IFACEMETHOD(get_Name)(BSTR* pResult) override {
		if (pResult) *pResult = SysAllocString(&fakeTaskPath[1]);
		return pResult ? S_OK : E_INVALIDARG;
	}
	IFACEMETHOD(get_Path)(BSTR* pResult) override {
		if (pResult) *pResult = SysAllocString(fakeTaskPath);
		return pResult ? S_OK : E_INVALIDARG;
	}
	IFACEMETHOD(get_State)(TASK_STATE* pResult) override {
		if (pResult) *pResult = TASK_STATE_READY;
		return pResult ? S_OK : E_INVALIDARG;
	}
	IFACEMETHOD(get_Enabled)(VARIANT_BOOL* pResult) override {
		if (pResult) *pResult = VARIANT_TRUE;
		return pResult ? S_OK : E_INVALIDARG;
	}
	IFACEMETHOD(put_Enabled)(VARIANT_BOOL) override {
		return E_ACCESSDENIED;
	}
	IFACEMETHOD(Run)(VARIANT, IRunningTask** ppResult) override {
		if (ppResult) *ppResult = NULL;
		return ppResult ? E_ACCESSDENIED : E_INVALIDARG;
	}
	IFACEMETHOD(RunEx)(VARIANT params, LONG, LONG, BSTR, IRunningTask** ppResult) override {
		return Run(params, ppResult);
	}
	IFACEMETHOD(GetInstances)(LONG, IRunningTaskCollection** ppResult) override {
		if (ppResult) *ppResult = NULL;
		return ppResult ? E_ACCESSDENIED : E_INVALIDARG;
	}
	IFACEMETHOD(get_LastRunTime)(DATE* pResult) override {
		if (pResult) *pResult = 0;
		return pResult ? S_OK : E_INVALIDARG;
	}
	IFACEMETHOD(get_LastTaskResult)(LONG* pResult) override {
		if (pResult) *pResult = 0;
		return pResult ? S_OK : E_INVALIDARG;
	}
	IFACEMETHOD(get_NumberOfMissedRuns)(LONG* pResult) override {
		return get_LastTaskResult(pResult);
	}
	IFACEMETHOD(get_NextRunTime)(DATE* pResult) override {
		return get_LastRunTime(pResult);
	}
	IFACEMETHOD(get_Definition)(ITaskDefinition** ppResult) override {
		ASL_PRINTF(ASL_LEVEL_TRACE, "Task definition requested");
		if (ppResult) {
			*ppResult = fakeTaskDefinition;
			fakeTaskDefinition->AddRef();
		}
		return ppResult ? S_OK : E_INVALIDARG;
	}
	IFACEMETHOD(get_Xml)(BSTR* pResult) override {
		if (pResult) *pResult = NULL;
		return pResult ? E_ACCESSDENIED : E_INVALIDARG;
	}
	IFACEMETHOD(GetSecurityDescriptor)(LONG, BSTR* pResult) override {
		return get_Xml(pResult);
	}
	IFACEMETHOD(SetSecurityDescriptor)(BSTR, LONG) override {
		return E_ACCESSDENIED;
	}
	IFACEMETHOD(Stop)(LONG) override {
		return E_ACCESSDENIED;
	}
	IFACEMETHOD(GetRunTimes)(LPSYSTEMTIME, LPSYSTEMTIME, DWORD* pCount, LPSYSTEMTIME* pResult) override {
		if (pCount) *pCount = 0;
		if (pResult) *pResult = NULL;
		return (pCount && pResult) ? E_ACCESSDENIED : E_INVALIDARG;
	}
};
