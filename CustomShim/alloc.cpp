#include "framework.h"
#include <new>

#define ALLOCATOR_OPERATOR _NODISCARD _Ret_notnull_ _Post_writable_byte_size_(_Size) _VCRT_ALLOCATOR void* __CRTDECL operator

void* AllocateOrThrow(size_t size) throw (std::bad_alloc) {
	auto hlocal = LocalAlloc(LMEM_ZEROINIT, size == 0 ? 1 : size);
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
