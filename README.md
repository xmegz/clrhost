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

https://github.com/dotnet/runtime/blob/main/src/libraries/System.Private.CoreLib/src/Internal/Runtime/InteropServices/ComponentActivator.cs
```
    public static unsafe int LoadAssemblyBytes(byte* assembly, nint assemblyByteLength, byte* symbols, nint symbolsByteLength, IntPtr loadContext, IntPtr reserved)
        {
            if (!IsSupported)
                return HostFeatureDisabled;

            try
            {
                ArgumentNullException.ThrowIfNull(assembly);
                ArgumentOutOfRangeException.ThrowIfNegativeOrZero(assemblyByteLength);
                ArgumentOutOfRangeException.ThrowIfGreaterThan(assemblyByteLength, int.MaxValue);
                ArgumentOutOfRangeException.ThrowIfNotEqual(loadContext, IntPtr.Zero);
                ArgumentOutOfRangeException.ThrowIfNotEqual(reserved, IntPtr.Zero);

                ReadOnlySpan<byte> assemblySpan = new ReadOnlySpan<byte>(assembly, (int)assemblyByteLength);
                ReadOnlySpan<byte> symbolsSpan = default;
                if (symbols != null && symbolsByteLength > 0)
                {
                    symbolsSpan = new ReadOnlySpan<byte>(symbols, (int)symbolsByteLength);
                }

                LoadAssemblyBytesLocal(assemblySpan, symbolsSpan);
            }
            catch (Exception e)
            {
                return e.HResult;
            }

            return 0;

            [UnconditionalSuppressMessage("ReflectionAnalysis", "IL2026:RequiresUnreferencedCode",
                Justification = "The same feature switch applies to GetFunctionPointer and this function. We rely on the warning from GetFunctionPointer.")]
            static void LoadAssemblyBytesLocal(ReadOnlySpan<byte> assemblyBytes, ReadOnlySpan<byte> symbolsBytes) => AssemblyLoadContext.Default.InternalLoad(assemblyBytes, symbolsBytes);
        }
```


runtime\src\native\corehost\hostpolicy\hostpolicy.cpp
```
  case coreclr_delegate_type::load_assembly_bytes:
            return coreclr->create_delegate(
                "System.Private.CoreLib",
                "Internal.Runtime.InteropServices.ComponentActivator",
                "LoadAssemblyBytes",
                delegate);
```

C:\Projects\Microsoft\Source\runtime\src\native\corehost\hostpolicy\coreclr.cpp
```
pal::hresult_t coreclr_t::create_delegate(
    const char* entryPointAssemblyName,
    const char* entryPointTypeName,
    const char* entryPointMethodName,
    void** delegate)
{
    assert(coreclr_contract.coreclr_execute_assembly != nullptr);

    return coreclr_contract.coreclr_create_delegate(
        _host_handle,
        _domain_id,
        entryPointAssemblyName,
        entryPointTypeName,
        entryPointMethodName,
        delegate);
}
```


dumpbin /exports coreclr.dll
```
Microsoft (R) COFF/PE Dumper Version 14.39.33520.0
Copyright (C) Microsoft Corporation.  All rights reserved.


Dump of file coreclr.dll

File Type: DLL

  Section contains the following exports for coreclr.dll

    00000000 characteristics
    FFFFFFFF time date stamp
        0.00 version
           2 ordinal base
          11 number of functions
          11 number of names

    ordinal hint RVA      name

          3    0 0049CDD0 CLRJitAttachState
          4    1 002135D0 GetCLRRuntimeHost
          5    2 0010A000 MetaDataGetDispenser
          6    3 003B6C60 coreclr_create_delegate
          7    4 00117690 coreclr_execute_assembly
          8    5 00117390 coreclr_initialize
          9    6 001572E0 coreclr_set_error_writer
         10    7 003B6EA0 coreclr_shutdown
         11    8 0014BAE0 coreclr_shutdown_2
          2    9 00488740 g_CLREngineMetrics
         12    A 004051C0 g_dacTable
```
Native entry point
https://github.com/smx-smx/EzDotnet
```
namespace ManagedSample
{
	public class EntryPoint {
		private static string[] ReadArgv(IntPtr args, int sizeBytes) {
			int nargs = sizeBytes / IntPtr.Size;
			string[] argv = new string[nargs];
			
			for(int i=0; i<nargs; i++, args += IntPtr.Size) {
				IntPtr charPtr = Marshal.ReadIntPtr(args);
				argv[i] = Marshal.PtrToStringAnsi(charPtr);
			}
			return argv;
		}
		
		private static int Entry(IntPtr args, int sizeBytes) {
			string[] argv = ReadArgv(args, sizeBytes);
			Main(argv);
			return 0;
		}

		public static void Main(string[] args){
			Console.WriteLine("Hello, World");
		}
	}
}
```