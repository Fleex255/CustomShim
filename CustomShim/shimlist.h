#pragma once
#include "Shim.h"

// This file declares the vector of known shims, to be populated by shimlist.cpp and used by engineinterface.cpp.

extern std::vector<Shim*>* knownShims;
void EnsureShimsInstantiated();
