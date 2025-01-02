# How to Embed .NET Core DLL into a Custom C++ Host Loader

## Concept
Load .NET project DLLs directly from PE resources or ELF sections and initialize CoreCLR from these embedded resources.

## Why better than single file publish ?

**True Memory-Based Operation**
The resource injection technique allows embedded .NET DLLs to be loaded directly from memory without relying on the file system.

**Greater Flexibility**
Injected DLLs are placed in custom sections or embedded as resources within the host binary, enabling completely customizable loading, management, and execution.

**Enhanced Security**
The injection method allows resources to be encrypted, reducing the chances of reverse engineering and manipulation.

**Improved Startup Performance**
The native binary directly invokes CoreCLR from memory and loads the injected DLLs as resources, significantly reducing startup time.

**Easier Diagnostics and Debugging**
The host code retains complete control over .NET runtime initialization, providing more precise diagnostic tools and logging capabilities.

**Conclusion**
While Single File Publish is straightforward and suitable for many use cases, bundling .NET DLLs at the end of the native binary comes with several limitations and potential issues. The presented resource injection technique addresses these challenges by offering greater flexibility, security, and performance.

## Project Structure
* **AppHostLinux** - Linux build project for the .NET Core startup (C++)
* **AppHostWindows** - Windows build project for the .NET Core startup (C++)
* **AppHostShared** - Shared source code for the .NET Core startup (C++).
* **Hello** - A .NET Core project compiled into ''Hello.dll'', which is embedded as a resource/section into the AppHost binary (C#).
* **InjectResourceLinux** - Injects 'Hello.dll' into 'AppHostLinux.Out' as an ELF section (C#).
* **InjectResourceWindows** - Injects 'Hello.dll' into 'AppHostWindows.Exe' as a PE resource (C#).


## Build instuctions


### Prerequisites
* Operating System: Windows 11
* .NET SDK: Version 8.0 (64-bit)
* Linux Environment: WSL with Ubuntu 20.04 LTS
* IDE: Visual Studio Community 2022

### Installing Dependencies in WSL
Run the following commands in your WSL terminal to install dotnet-sdk and a build tools
```bash
wget https://packages.microsoft.com/config/ubuntu/20.04/packages-microsoft-prod.deb -O packages-microsoft-prod.deb
sudo dpkg -i packages-microsoft-prod.deb
rm packages-microsoft-prod.deb
sudo apt-get update && sudo apt-get install -y dotnet-sdk-8.0

sudo apt install -y openssh-server build-essential gdb rsync make zip
wget https://github.com/microsoft/CMake/releases/download/v3.19.4268486/cmake-3.19.4268486-MSVC_2-Linux-x64.sh
chmod +x cmake-3.19.4268486-MSVC_2-Linux-x64.sh
./cmake-3.19.4268486-MSVC_2-Linux-x64.sh
```

### Clone repository

```bash
git clone https://github.com/xmegz/clrhost.git
```

### Compile
* Open the solution in 'Visual Studio 2022'.
* Build the 'InjectResourceWindows' project in 64-bit 'Release' mode.
* Run 'InjectResourceWindows' to generate the Windows-hosted executable.
* Build the 'InjectResourceLinux' project in 64-bit 'Release' mode.
* Run 'InjectResourceLinux' to generate the Linux-hosted executable.

## Execution Instructions

**Windows**
Execute the generated binary with arguments from the 'out' folder:
```powershell
clrhost\InjectResourceWindows\bin\Release\net8.0\out>Hello.full.exe 1 2 3 4
```

Sample Output:
```powershell
[INFO] Apphost [Jan  2 2025 19:07:05]
[INFO] CoreCLR path:c:\Program Files\dotnet\shared\Microsoft.NETCore.App\8.0.3\coreclr.dll
[INFO] CoreCLR pointer:0xd6720000
[INFO] Error writer setted
[INFO] Initialize OK
[INFO] Create delegate OK
[INFO] Call assembly load addr:0x00418cf0 size:9728
[INFO] Assembly load ret - 0x00000000
[INFO] Create delegate OK
[INFO] Delegate entryPoint OK
[INFO] Call entryPoint delegate...
info: Hello.Program[0]
      Ctor Hello, Version=1.0.9133.34412, Culture=neutral, PublicKeyToken=null
info: Hello.Program[0]
      Start 2025. 01. 02. 19:07:34
info: Hello.Program[0]
      Args: 1,2,3,4
info: Hello.Program[0]
      APP_PATHS : C:\Projects\PadarCom\Source\clrhost\InjectResourceWindows\bin\Release\net8.0\out\
info: Hello.Program[0]
      APP_NAME : APPHOST
info: Hello.Program[0]
      APPBASE : C:\Projects\PadarCom\Source\clrhost\InjectResourceWindows\bin\Release\net8.0\out\
info: Hello.Program[0]
      HOST_RUNTIME_CONTRACT : 0x7ff635369078
info: Hello.Program[0]
      NATIVE_DLL_SEARCH_DIRECTORIES : C:\Projects\PadarCom\Source\clrhost\InjectResourceWindows\bin\Release\net8.0\out\runtimes\win-x64\native\;C:\Projects\PadarCom\Source\clrhost\InjectResourceWindows\bin\Release\net8.0\out\runtimes\win\lib\netcoreapp3.0\;C:\Projects\PadarCom\Source\clrhost\InjectResourceWindows\bin\Release\net8.0\out\runtimes\win\lib\netstandard2.0\;c:\Program Files\dotnet\shared\Microsoft.NETCore.App\8.0.3\;c:\Program Files\dotnet\shared\Microsoft.AspNetCore.App\8.0.3\;C:\Projects\PadarCom\Source\clrhost\InjectResourceWindows\bin\Release\net8.0\out\;
info: Hello.Data[0]
      Test Json Serializer {"Id":0,"Name":"Name"}
info: Hello.Program[0]
      End 2025. 01. 02. 19:07:34
[INFO] Shutdown exitCode:11
```

**Linux**
Run the executable from your WSL terminal from the 'out' folder:
``` bash
asus@Asus:/mnt/c/clrhost/InjectResourceLinux/bin/Release/net8.0/out$ ./Hello.full.exe 1 2 3 4
```

Sample Output:
``` bash
[INFO] Apphost [Jan  2 2025 19:07:00]
[INFO] CoreCLR path:/usr/share/dotnet/shared/Microsoft.NETCore.App/8.0.3/libcoreclr.so
[INFO] CoreCLR pointer:0xdba67680
[INFO] Error writer setted
[INFO] Initialize OK
[INFO] Create delegate OK
[INFO] Call assembly load addr:0xdbb2baa0 size:9728
[INFO] Assembly load ret - 0x00000000
[INFO] Create delegate OK
[INFO] Delegate entryPoint OK
[INFO] Call entryPoint delegate...
info: Hello.Program[0]
      Ctor Hello, Version=1.0.9133.34412, Culture=neutral, PublicKeyToken=null
info: Hello.Program[0]
      Start 01/02/2025 19:07:26
info: Hello.Program[0]
      Args: 1,2,3,4
info: Hello.Program[0]
      APP_PATHS : /mnt/c/Projects/PadarCom/Source/clrhost/InjectResourceLinux/bin/Release/net8.0/out/
info: Hello.Program[0]
      APP_NAME : APPHOST
info: Hello.Program[0]
      APPBASE : /mnt/c/Projects/PadarCom/Source/clrhost/InjectResourceLinux/bin/Release/net8.0/out/
info: Hello.Program[0]
      HOST_RUNTIME_CONTRACT : 0x5639da2a0020
info: Hello.Program[0]
      NATIVE_DLL_SEARCH_DIRECTORIES : /mnt/c/Projects/PadarCom/Source/clrhost/InjectResourceLinux/bin/Release/net8.0/out/runtimes/linux-x64/native/:/mnt/c/Projects/PadarCom/Source/clrhost/InjectResourceLinux/bin/Release/net8.0/out/runtimes/linux/lib/netstandard2.0/:/mnt/c/Projects/PadarCom/Source/clrhost/InjectResourceLinux/bin/Release/net8.0/out/runtimes/unix/lib/netcoreapp3.0/:/usr/share/dotnet/shared/Microsoft.NETCore.App/8.0.3/:/usr/share/dotnet/shared/Microsoft.AspNetCore.App/8.0.3/:/mnt/c/Projects/PadarCom/Source/clrhost/InjectResourceLinux/bin/Release/net8.0/out/:
info: Hello.Data[0]
      Test Json Serializer {"Id":0,"Name":"Name"}
info: Hello.Program[0]
      End 01/02/2025 19:07:26
[INFO] Shutdown exitCode:11
```

# How it made ?

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


## Inject Resource Windows

Inject Windows Resource to file
https://github.com/dotnet/runtime/tree/main/src/coreclr/tools/InjectResource

## Inject Resource Linux

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
