# .Net Core Runtime Host Start
- Simple conception howto start .Net Core Runtime from native code
- C++ part and C# part debugging is also possible.
- Just for .Net Core 8.0.2 version

## Main repository
https://github.com/dotnet/runtime

## Tutorials
https://learn.microsoft.com/en-us/dotnet/core/tutorials/netcore-hosting

## Design
https://github.com/dotnet/runtime/blob/main/docs/design/features/native-hosting.md

## APIs for hosting and resolve net runtime
https://github.com/dotnet/runtime/blob/main/src/native/corehost/coreclr_delegates.h
https://github.com/dotnet/runtime/blob/main/src/native/corehost/coreclr_resolver.h
https://github.com/dotnet/runtime/blob/main/src/native/corehost/nethost/nethost.h
https://github.com/dotnet/runtime/blob/main/src/native/corehost/hostfxr.h

## Sample Host Example Project
https://github.com/dotnet/samples/blob/main/core/hosting/src/NativeHost/nativehost.cpp
