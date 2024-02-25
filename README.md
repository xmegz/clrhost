# .Net Core Runtime Host Start
Simple conception howto start .Net Core Runtime from native code   
C++ part and C# part debugging is also possible.    
Just for .Net Core 3.1.32 version   
 

## APIs For Hosting CoreCLR
https://github.com/dotnet/coreclr/blob/release/3.1/src/vm/corhost.cpp
https://github.com/dotnet/coreclr/blob/release/3.1/src/binder/applicationcontext.cpp
https://github.com/dotnet/coreclr/blob/release/3.1/src/dlls/mscoree/unixinterface.cpp

## Sample Host Example Project
https://github.com/dotnet/samples/blob/main/core/hosting/src/NativeHost/nativehost.cpp

