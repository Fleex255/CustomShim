# `apphelp.dll` Import Library

This little project builds a LIB file for `apphelp.dll` so the main project can import functions from it.
(For cdecl functions, one can simply write a DEF and compile it with the `lib` utility, but stdcall functions get their names mangled in a way that requires some knowledge of the parameter list.)

If you need more `apphelp.dll` functions than the seven already imported:

1. Add function stubs with the correct names, calling conventions, and parameter sizes to `functions.cpp`.
2. Add corresponding lines to `module.def`.
3. Compile in the Debug x86 (or x64 for 64-bit) configuration.
4. Copy `apphelp.lib` from the `Debug` folder (`x64\Debug` for 64-bit) here to the `libs` (`libs64` for 64-bit) folder of the main project.
