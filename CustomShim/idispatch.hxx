private:
	volatile unsigned cRef = 1;
public:	
	IFACEMETHOD(QueryInterface)(REFIID riid, void** ppv) override {
		if (!ppv) return E_INVALIDARG;
		*ppv = NULL;
		if (riid == IID_IUnknown || riid == IID_IDispatch || riid == IID_CURRENT) {
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
#undef IID_CURRENT
