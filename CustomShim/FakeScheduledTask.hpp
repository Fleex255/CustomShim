#pragma once
#include <taskschd.h>

class FakeScheduledTask : public IRegisteredTask {
private:
	volatile unsigned cRef = 1;
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
	FakeScheduledTask(const FakeScheduledTask&) = delete;
	FakeScheduledTask& operator=(const FakeScheduledTask&) = delete;
	IFACEMETHOD(QueryInterface)(REFIID riid, void** ppv) override {
		if (!ppv) return E_INVALIDARG;
		*ppv = NULL;
		if (riid == IID_IUnknown || riid == IID_IDispatch || riid == IID_IRegisteredTask) {
			*ppv = (PVOID) this;
			AddRef();
			return S_OK;
		} else {
			return E_NOINTERFACE;
		}
	}
	IFACEMETHOD_(ULONG, AddRef)() override {
		InterlockedIncrement(&cRef);
		return cRef;
	}
	IFACEMETHOD_(ULONG, Release)() override {
		InterlockedDecrement(&cRef);
		unsigned localRef = cRef;
		if (cRef == 0) delete this;
		return localRef;
	}
	IFACEMETHOD(GetTypeInfoCount)(UINT* pCount) override {
		if (pCount) *pCount = 0;
		return S_OK;
	}
	IFACEMETHOD(GetTypeInfo)(UINT iTInfo, LCID, ITypeInfo** ppResult) override {
		if (ppResult) *ppResult = NULL;
		return iTInfo == 0 ? E_NOTIMPL : DISP_E_BADINDEX;
	}
	IFACEMETHOD(GetIDsOfNames)(REFIID, LPOLESTR*, UINT, LCID, DISPID*) override {
		return E_NOTIMPL;
	}
	IFACEMETHOD(Invoke)(DISPID, REFIID, LCID, WORD, DISPPARAMS*, VARIANT*, EXCEPINFO*, UINT*) override {
		return DISP_E_MEMBERNOTFOUND;
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
