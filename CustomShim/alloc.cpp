#include "framework.h"
#include <new>

// Some of the code in this DLL runs and needs to allocate memory before the C++ runtime is initialized.
// This file replaces the new and delete operators' implementations with ones that use the standard Windows process heap instead.

#define ALLOCATOR_OPERATOR _NODISCARD _Ret_notnull_ _Post_writable_byte_size_(_Size) _VCRT_ALLOCATOR void* __CRTDECL operator

void* AllocateOrThrow(size_t size) {
	auto hlocal = LocalAlloc(LMEM_ZEROINIT, size == 0 ? 1 : size); // Must return a distinct pointer even for zero-size allocations
	if (hlocal) {
		return (void*) hlocal;
	} else {
		throw std::bad_alloc();
	}
}

ALLOCATOR_OPERATOR new(size_t _Size) {
	return AllocateOrThrow(_Size);
}

ALLOCATOR_OPERATOR new[](size_t _Size) {
	return AllocateOrThrow(_Size);
}

void operator delete(void* ptr) {
	if (ptr) LocalFree(ptr);
}

void operator delete[](void* ptr) {
	if (ptr) LocalFree(ptr);
}
