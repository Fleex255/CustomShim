# Example Shim Module

This project produces an AppCompat shim DLL with two example shims:

* `AcceptEula` can be applied to Sysinternals utilities to make them think their EULA has already been accepted (even without the `/accepteula` switch).
  It demonstrates how to write simple shims that hook imported Win32 functions.
* `FakeSchTask` can be applied to [Autoruns(c)](https://docs.microsoft.com/en-us/sysinternals/downloads/autoruns) to make them see a fake scheduled task with a name specified as the shim command line.
  It demonstrates how to write shims that hook COM member functions or use/validate the shim command line.
  
To create a new shim:

1. Create a `Shim_[name].h` file declaring a subclass of `Shim` with a name starting with `Shim_`.
    * `#include "Shim.h"`.
    * Declare a public zero-argument constructor.
	* Declare protected overrides of `GetHooks` and the two other shim lifecycle callbacks if needed.
	* Declare private static functions for your hook implementations.
2. Create a `Shim_[name].cpp` file to implement your shim class.
    * `#include` your shim header file.
    * Use the `SHIM_INSTANCE(name)` macro (no quotes around the name) to define the constructor and `instance` variable.
	* When populating the hooks vector in `GetHooks`, use `MakeHookInfo` to produce a `HOOKAPI` structure for each imported function you would like to hook.
	* If hooking COM functions, use `SE_COM_AddServer` and `SE_COM_AddHook` in `HandleNotification` in response to `SE_NOTIFY_INIT`.
	* In hook implementations, use the `instance->GetNextProcAddress` function (for imported function hooks) or the `REQUIRE_NEXT` macro (for COM hooks) to get the original function.
3. Register your shim in `shimlist.cpp`.
    * `#include` your shim header file.
	* In `EnsureShimsInstantiated`, add an instance of your shim class to the `knownShims` vector.
	
You can delete the example `Shim_*.*` and `FakeSchTask_fakes.hpp` files and remove the example shims from `shimlist.cpp` if making your own.