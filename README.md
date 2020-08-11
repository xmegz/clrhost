# .Net Core Runtime Host Start
Simple conception howto start .Net core runtime from native code
C++ part and C# part debugging is also possible.
.Net Core 3.1.6
Plain Concept

## APIs for hosting CoreCLR
https://github.com/dotnet/coreclr/blob/release/3.1/src/vm/corhost.cpp
https://github.com/dotnet/coreclr/blob/release/3.1/src/binder/applicationcontext.cpp
https://github.com/dotnet/coreclr/blob/release/3.1/src/dlls/mscoree/unixinterface.cpp

## Sample Host Examples
https://github.com/dotnet/samples/tree/master/core/hosting/HostWithCoreClrHost

