#include <vector>
#include "shimlist.h"

// This file defines and populates a vector of shims that will be accessible to the shim engine.

#include "Shim_AcceptEula.h"
#include "Shim_FakeSchTask.h"
// Include your shim declarations here...

// This DLL loads early in the process's life, even before the C++ runtime library is ready.
// Static initializers therefore must not call into the CRT, so the vector must be constructed later.
std::vector<Shim*>* knownShims = NULL;

void EnsureShimsInstantiated() {
	if (knownShims) return;
	knownShims = new std::vector<Shim*>();
	knownShims->push_back(new Shim_AcceptEula());
	knownShims->push_back(new Shim_FakeSchTask());
	// Instantiate and add your shims here...
	ASL_PRINTF(ASL_LEVEL_TRACE, "Instantiated %d shims", knownShims->size());
}
