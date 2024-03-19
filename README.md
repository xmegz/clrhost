# .Net Core Runtime Host Start
- Simple conception howto start .Net Core Runtime from native code
- C++ part and C# part debugging is also possible.

- Simple conception for windows to inject dotnet assembly to native exe as resource
- Just for dotnet 8

## Main repository
https://github.com/dotnet/runtime

https://mattwarren.org/2017/03/23/Hitchhikers-Guide-to-the-CoreCLR-Source-Code/

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

## AssemblyLoadBytes trace

https://github.com/dotnet/runtime/blob/main/src/libraries/System.Private.CoreLib/src/Internal/Runtime/InteropServices/ComponentActivator.cs

```dotnet
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


https://github.com/dotnet/runtime/blob/main/src/native/corehost/hostpolicy/hostpolicy.cpp

```cpp
  case coreclr_delegate_type::load_assembly_bytes:
            return coreclr->create_delegate(
                "System.Private.CoreLib",
                "Internal.Runtime.InteropServices.ComponentActivator",
                "LoadAssemblyBytes",
                delegate);
```

https://github.com/dotnet/runtime/blob/main/src/native/corehost/hostpolicy/coreclr.cpp

```cpp
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

## Corecrl dll exports interface

```dos
dumpbin /exports coreclr.dll
```

```console
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

## Native entry point construct

https://github.com/smx-smx/EzDotnet

```csharp
namespace ManagedSample
{
	public class Program {
	  private static void NativeEntryPoint(int argc, IntPtr argv)
        {
            static string[] MarshalArgv(int argc, IntPtr argv)
            {
                string[] args = new string[argc];

                for (int i = 0; i < argc; i++, argv += IntPtr.Size)
                    args[i] = Marshal.PtrToStringAnsi(Marshal.ReadIntPtr(argv));

                return args;
            }

            string[] args = MarshalArgv(argc, argv);

            Main(args);
        }

		public static void Main(string[] args){
			Console.WriteLine("Hello, World");
		}
	}
}
```

## Hosting links

Host Runtime information
https://github.com/dotnet/runtime/blob/main/docs/design/features/host-runtime-information.md

Host Traceing
https://github.com/dotnet/runtime/blob/main/docs/design/features/host-tracing.md

Hosting Layer Apis (hostpolicy, hostfxr, coreclr)
https://github.com/dotnet/runtime/blob/main/docs/design/features/hosting-layer-apis.md

Host Error codes
https://github.com/dotnet/runtime/blob/main/docs/design/features/host-error-codes.md

c:\Projects\Microsoft\Source\runtime\src\installer\managed\Microsoft.NET.HostModel\
c:\Projects\Microsoft\Source\runtime\src\native\corehost\

## Inject Resource Windows

Inject Windows Resource to file
https://github.com/dotnet/runtime/tree/main/src/coreclr/tools/InjectResource

## Inject Resource Linux

Install build tools
```bash
sudo apt install -y openssh-server build-essential gdb rsync make zip
wget https://github.com/microsoft/CMake/releases/download/v3.19.4268486/cmake-3.19.4268486-MSVC_2-Linux-x64.sh
chmod +x cmake-3.19.4268486-MSVC_2-Linux-x64.sh
./cmake-3.19.4268486-MSVC_2-Linux-x64.sh
```

Objcopy test
```bash
objcopy --add-section .sname=Hello.dll AppHostLinux.out New.out
objcopy --add-section .sname=Hello.dll --set-section-flags .sname=noload,readonly AppHostLinux.out New.out
```

Usefull links
https://stackoverflow.com/questions/7370407/get-the-start-and-end-address-of-text-section-in-an-executable

https://github.com/mattst88/build-id

https://stackoverflow.com/questions/10863510/getting-the-sh-name-member-in-a-section-header-elf-file

Test obj scan
```bash
mcedit dump_shdr.c
```

```c
#include <sys/stat.h>
#include <sys/mman.h>
#include <elf.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>


int print_shdr(const char *const fname, size_t size) {
  int fd = open(fname, O_RDONLY);
  char *p = mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);

  Elf64_Ehdr *ehdr = (Elf64_Ehdr*)p;
  Elf64_Shdr *shdr = (Elf64_Shdr *)(p + ehdr->e_shoff);
  int shnum = ehdr->e_shnum;

  Elf64_Shdr *sh_strtab = &shdr[ehdr->e_shstrndx];
  const char *const sh_strtab_p = p + sh_strtab->sh_offset;

  for (int i = 0; i < shnum; ++i) {
    const char* sname = sh_strtab_p + shdr[i].sh_name;

    if (!strcmp(sname,".sname"))
    {
        printf("%2d: %4d '%s' %4lu %4lu\n", i, shdr[i].sh_name,
               sname, shdr[i].sh_offset,shdr[i].sh_size);

        printf("dump\n");
        const char* data = p + shdr[i].sh_offset;
        long size = shdr[i].sh_size;
        while (size--)
        {
            putchar(*data);
            data++;
        }
        printf("\ndump\n");
    }
  }

  return 0;
}

int main(int argc, char *argv[])
{
  struct stat st;
  const char *fname = "/proc/self/exe";

  if (argc > 1)
    fname = argv[1];

  if (stat(fname, &st) != 0) {
    perror("stat");
    return 1;
  }
  return print_shdr(fname, st.st_size);
}
```

```
gcc dump_shdr.c
objcopy --add-section .sname=dump_shdr.c --set-section-flags .sname=noload,readonly a.out new.out
./new.out
```

```
 objcopy --add-section .idr_rcdata1=Hello.dll --set-section-flags .idr_rcdata1=noload,readonly AppHostLinux.out new.out
```

Windows crosstool for Linux X64
https://docs.unrealengine.com/4.26/en-US/SharingAndReleasing/Linux/GettingStarted/

## Bundle makeing

Single File App
https://learn.microsoft.com/en-us/dotnet/core/deploying/single-file/overview?tabs=cli

Single File App Host Design
https://github.com/dotnet/designs/blob/main/accepted/2020/single-file/design.md

Single File Extract
https://github.com/dotnet/designs/blob/main/accepted/2020/single-file/extract.md

Bundler
https://github.com/dotnet/designs/blob/main/accepted/2020/single-file/bundler.md

Boundle marker.
c:\Projects\Microsoft\Source\runtime\src\native\corehost\apphost\

