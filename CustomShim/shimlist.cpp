#include <vector>
#include "shimlist.h"
#include "Shim_AcceptEula.h"
#include "Shim_FakeSchTask.h"

std::vector<Shim*>* knownShims = NULL;

void EnsureShimsInstantiated() {
	if (knownShims) return;
	knownShims = new std::vector<Shim*>();
	knownShims->push_back(new Shim_AcceptEula());
	knownShims->push_back(new Shim_FakeSchTask());
	ASL_PRINTF(ASL_LEVEL_TRACE, "Instantiated %d shims", knownShims->size());
}
