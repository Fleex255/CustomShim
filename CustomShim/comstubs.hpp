#pragma once
#include <OAIdl.h>

template<REFIID I1, REFIID I2, REFIID... IRest>
static bool IsRecognizedIID(REFIID riid) {
	return I1 == riid || IsRecognizedIID<I2, IRest...>(riid);
}

template<REFIID I>
static bool IsRecognizedIID(REFIID riid) {
	return I == riid;
}

template<typename T, REFIID... I>
class SimpleComObject : public T {
	static_assert(std::is_base_of<IUnknown, T>::value, "All COM objects must derive from IUnknown");
private:
	volatile unsigned cRef = 1;
public:
	SimpleComObject() = default;
	virtual ~SimpleComObject() = default;
	SimpleComObject(const SimpleComObject&) = delete;
	SimpleComObject& operator=(const SimpleComObject&) = delete;
	IFACEMETHOD(QueryInterface)(REFIID riid, void** ppv) override {
		if (!ppv) return E_INVALIDARG;
		*ppv = NULL;
		if (IsRecognizedIID<IID_IUnknown, I...>(riid)) {
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
		if (cRef == 0) {
			ASL_PRINTF(ASL_LEVEL_TRACE, "Last reference dropped");
			delete this;
		}
		return localRef;
	}
};

template<typename T, REFIID... I>
class DispatchStub : public SimpleComObject<T, IID_IDispatch, I...> {
public:
	IFACEMETHOD(GetTypeInfoCount)(UINT* pCount) override {
		if (pCount) *pCount = 0;
		return pCount ? S_OK : E_INVALIDARG;
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
};
