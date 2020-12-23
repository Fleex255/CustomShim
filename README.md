# Custom Application Compatibility Shim Development Kit

The `CustomShim` Visual Studio C++ project here builds a 32-bit Windows
[application compatibility shim](https://techcommunity.microsoft.com/t5/ask-the-performance-team/demystifying-shims-or-using-the-app-compat-toolkit-to-make-your/ba-p/374947)
DLL providing two example shims.
You can use it as a starting point to create custom shims that intercept calls to Win32 or COM functions made by 32-bit applications.
It has been tested on Windows 10 2004, but it comes with **absolutely no warranty** and is **definitely not supported by Microsoft**.

## Installing Custom Shim Modules

To make a shim DLL available to the shim engine, place it in the `C:\Windows\SysWOW64` directory.
Windows 10 will only load shims from DLLs with [certain names](https://gist.github.com/w4kfu/95a87764db7029e03f09d78f7273c4f4#file-dllinjshim-cpp-L12-L28),
so your DLL must be renamed after being compiled.
`AcRes.dll` is a good choice because that's the name of a real AppCompat-related file (but not shim module) that resides elsewhere.

## Applying Custom Shims

While the [Compatibility Administrator](https://docs.microsoft.com/en-us/windows/deployment/planning/using-the-compatibility-administrator-tool)
can see shims defined in non-system SDB files if the shims are marked as general-purpose, it does not support using them in new application fixes.
SDB files that declare and/or use custom shims must therefore be created by other means.
The most scalable way is to [patch the Compatibility Administrator into ShimDBC](https://fleexlab.blogspot.com/2020/12/turning-compatibility-administrator.html),
a Microsoft-internal utility that compiles XML into SDB.
For example, this XML produces an SDB that demonstrates the two example shims on some Sysinternals utilities:

```xml
<?xml version="1.0" encoding="utf-8"?>
<DATABASE NAME="Test Database" ID="{5FB8C914-168C-4B9B-8256-DF8A0F384E3E}">
    <LIBRARY>
        <SHIM NAME="AcceptEula" FILE="AcRes.dll" RUNTIME_PLATFORM="X86_ANY" ID="{92E61B85-313A-4880-B6E4-DEF2567413AD}"/>
        <SHIM NAME="FakeSchTask" FILE="AcRes.dll" RUNTIME_PLATFORM="X86_ANY" ID="{C1CAD7E2-ACAC-4467-8A6A-D437C51D5918}"/>
    </LIBRARY>
    <APP NAME="Winobj" ID="{CC4BFC0C-5815-4F08-99C7-4ED13E611FAB}">
        <EXE NAME="Winobj.exe" RUNTIME_PLATFORM="X86_ANY" PRODUCT_NAME="Sysinternals Winobj" ID="{B49373D9-BC1E-4941-A43B-7B5814C23D93}">
            <SHIM NAME="AcceptEula"/>
        </EXE>
    </APP>
    <APP NAME="AccessChk" ID="{D45B7077-34B6-463B-B046-38FC68B13430}">
        <EXE NAME="accesschk.exe" RUNTIME_PLATFORM="X86_ANY" PRODUCT_NAME="Sysinternals AccessChk" ID="{E79328FD-855A-4D8A-92A3-F502E831BD1B}">
            <SHIM NAME="AcceptEula"/>
        </EXE>
    </APP>
    <APP NAME="Autoruns" ID="{23897D6A-04BA-43D3-879F-C0F1E934635E}">
        <EXE NAME="autorunsc.exe" RUNTIME_PLATFORM="X86_ANY" PRODUCT_NAME="Sysinternals Autoruns" ID="{12A4EE20-93F4-4C55-B594-A0ABCD3C3283}">
            <SHIM NAME="FakeSchTask"/>
        </EXE>
        <EXE NAME="autoruns.exe" RUNTIME_PLATFORM="X86_ANY" PRODUCT_NAME="Sysinternals Autoruns" ID="{6C4947F7-A387-443D-BB1A-D5BD7C42F908}">
            <SHIM NAME="FakeSchTask" COMMAND_LINE="It's a FAAAAAKE!"/>
        </EXE>
    </APP>
</DATABASE>
```

Names of valid matching information attributes for `EXE` tags can be found by examining existing fix entries in the Compatibility Administrator.
`ID` attributes can omitted, in which case ShimDBC will update the input XML file to generate them.
If a call is not intercepted due to the DLL that makes it ("inex policy"), you can add `INCLUDE` tags inside the library `SHIM` definitions, e.g. `<INCLUDE MODULE="clr.dll"/>` for .NET applications.

To compile the XML file named `YourXml.xml` into `YourDatabase.sdb`:

    shimdbc Custom YourXml.xml YourDatabase.sdb -op X86_ANY
    
Alternatively, you can use the somewhat underdocumented [Application Compatibility Database API](https://docs.microsoft.com/en-us/windows/win32/devnotes/application-compatibility-database)
to write the SDB yourself.
The `sdb.sprint` file in this directory is a [SprintDLL](https://github.com/Fleex255/SprintDLL) script that produces an SDB with the same effect as the above XML.

Regardless of how you produce the SDB, you can install it like any other, using the
[`sdbinst`](https://docs.microsoft.com/en-us/windows/deployment/planning/using-the-sdbinstexe-command-line-tool) utility that comes with Windows:

    sdbinst YourDatabase.sdb

## Getting Shim Logs

To enable logging messages from shims (logged through `SE_ShimDPF` e.g. by the `ASL_PRINTF` macro):

* Set the `SHIM_DEBUG_LEVEL` environment variable, e.g. to `4` for the most verbose logs.
* Set the `SHIM_FILE_LOG` environment variable to the filename to write the shim logs to under the `%TEMP%` directory.
  To save the log elsewhere, use `../` sequences (forward slash, not backslash) to traverse up out of the temporary directory.
  The process hosting the shim you're debugging must have write access to the location you specify.
  
To enable logging messages and reports from the shim engine (`apphelp.dll`):

* Set the `SHIMENG_DEBUG_LEVEL` environment variable, e.g. to `4` for the most verbose logs.
* Set the `LogFlags` DWord value in the `HKLM\Software\Microsoft\Windows NT\CurrentVersion\AppCompatFlags` Registry key to 255 (all reports).

Shim engine diagnostics are always saved as `%TEMP%\AslLog_[report type]_[process].exe_[pid].txt`.
`ApphelpDebug` logs have the log messages; `shimengstate` reports may also be interesting.

**Warning:** Enabling lots of logging at once, especially with .NET processes, can manifest a deadlock bug in the shim engine that brings down the process with a breakpoint-related exit code: 0x80000003 or 0x4000001F.
