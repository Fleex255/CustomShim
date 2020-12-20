#pragma once
#include <OAIdl.h>

// This file provides simple implementations of IUnknown and IDispatch, which may be useful for COM-hooking shims.
// If you are not hooking any COM functions, you do not need this file.

template<REFIID I1, REFIID I2, REFIID... IRest>
static bool IsRecognizedIID(REFIID riid) {
	return I1 == riid || IsRecognizedIID<I2, IRest...>(riid);
}

template<REFIID I>
static bool IsRecognizedIID(REFIID riid) {
	return I == riid;
}

// Base class for implementations of COM classes without branching inheritance chains.
// T is the most derived interface implemented; I is the IIDs of all implemented interfaces below IUnknown.
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
		return InterlockedIncrement(&cRef);
	}
	IFACEMETHOD_(ULONG, Release)() override {
		unsigned localRef = InterlockedDecrement(&cRef);
		if (localRef == 0) {
			ASL_PRINTF(ASL_LEVEL_TRACE, "Last reference dropped");
			delete this;
		}
		return localRef;
	}
};

// Stub IDispatch implementation, since actual dispatch functionality is not needed for this demo.
// T is again the most derived interface implemented; I is the IIDs of all implemented interfaces below IDispatch.
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
