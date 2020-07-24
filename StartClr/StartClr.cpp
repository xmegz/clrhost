// StartClr.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "windows.h"
#include "mscoree.h"
#include "coreclrhost.h"
using namespace std;

#define CORECLR_FILE "C:\\Program Files\\dotnet\\shared\\Microsoft.NETCore.App\\3.1.6\\coreclr.dll"
#define CORECLR_DIR "C:\\Program Files\\dotnet\\shared\\Microsoft.NETCore.App\\3.1.6"
#define APP_FILE "Hello.dll"

wchar_t* GetWC(const char* c)
{
    size_t newsize = strlen(c) + 1;
    wchar_t* wcstring = new wchar_t[newsize];
    
    size_t convertedChars = 0;
    mbstowcs_s(&convertedChars, wcstring, newsize, c, _TRUNCATE);

    return wcstring;
}

int main(const int argc, const wchar_t* argv[])
{
    cout << "Start\n";
    
    char targetApp[MAX_PATH];
    string appName(APP_FILE);        
    cout << "GetFullPathNameA:"<< appName <<"\n";
    DWORD pathLength = GetFullPathNameA(appName.c_str(), MAX_PATH, targetApp, NULL);
    if (pathLength == 0)
    {
        cout << "GetFullPathNameA Error! Last Error:" << GetLastError() << "\n";
        return 0;
    }

    

    string coreDllPath(CORECLR_FILE);    
    cout << "LoadLibraryExA:" << coreDllPath << "\n";
    HMODULE coreCLRModule = LoadLibraryExA(coreDllPath.c_str(), NULL, 0);
    if (coreCLRModule == NULL)
    {        
        cout << "LoadLibraryExA Error! Last Error:"<< GetLastError()<< "\n";
        return 0;
    }


    
    cout << "GetProcAddress:" << coreCLRModule << "\n";
    FnGetCLRRuntimeHost pfnGetCLRRuntimeHost = (FnGetCLRRuntimeHost)::GetProcAddress(coreCLRModule, "GetCLRRuntimeHost");
    if (pfnGetCLRRuntimeHost == NULL)
    {
        cout << "GetProcAddress Error! Last Error:" << GetLastError() << "\n";
        return 0;
    }

    ICLRRuntimeHost4* runtimeHost;    
    HRESULT hr = pfnGetCLRRuntimeHost(IID_ICLRRuntimeHost4, (IUnknown**)&runtimeHost);
  


    hr = runtimeHost->SetStartupFlags(
        // These startup flags control runtime-wide behaviors.
        // A complete list of STARTUP_FLAGS can be found in mscoree.h,
        // but some of the more common ones are listed below.
        static_cast<STARTUP_FLAGS>(
            // STARTUP_FLAGS::STARTUP_SERVER_GC |								// Use server GC
            // STARTUP_FLAGS::STARTUP_LOADER_OPTIMIZATION_MULTI_DOMAIN |		// Maximize domain-neutral loading
            // STARTUP_FLAGS::STARTUP_LOADER_OPTIMIZATION_MULTI_DOMAIN_HOST |	// Domain-neutral loading for strongly-named assemblies
            STARTUP_FLAGS::STARTUP_CONCURRENT_GC |						// Use concurrent GC
            STARTUP_FLAGS::STARTUP_SINGLE_APPDOMAIN |					// All code executes in the default AppDomain
                                                                        // (required to use the runtimeHost->ExecuteAssembly helper function)
            STARTUP_FLAGS::STARTUP_LOADER_OPTIMIZATION_SINGLE_DOMAIN	// Prevents domain-neutral loading
            )
    );

   

    hr = runtimeHost->Start();

    wchar_t* coreRoot = GetWC(CORECLR_DIR);
    wchar_t* targetAppPath = GetWC(targetApp);

    int appDomainFlags =
        // APPDOMAIN_FORCE_TRIVIAL_WAIT_OPERATIONS |		// Do not pump messages during wait
        // APPDOMAIN_SECURITY_SANDBOXED |					// Causes assemblies not from the TPA list to be loaded as partially trusted
        APPDOMAIN_ENABLE_PLATFORM_SPECIFIC_APPS |			// Enable platform-specific assemblies to run
        APPDOMAIN_ENABLE_PINVOKE_AND_CLASSIC_COMINTEROP |	// Allow PInvoking from non-TPA assemblies
        APPDOMAIN_DISABLE_TRANSPARENCY_ENFORCEMENT;			// Entirely disables transparency checks

    size_t tpaSize = 100 * MAX_PATH; // Starting size for our TPA (Trusted Platform Assemblies) list
    wchar_t* trustedPlatformAssemblies = new wchar_t[tpaSize];
    trustedPlatformAssemblies[0] = L'\0';

    // Extensions to probe for when finding TPA list files
    const wchar_t* tpaExtensions[] = {
        L"*.dll",
        L"*.exe",
        L"*.winmd"
    };

    // Probe next to CoreCLR.dll for any files matching the extensions from tpaExtensions and
    // add them to the TPA list. In a real host, this would likely be extracted into a separate function
   // and perhaps also run on other directories of interest.
    for (int i = 0; i < _countof(tpaExtensions); i++)
    {
        // Construct the file name search pattern
        wchar_t searchPath[MAX_PATH];
        wcscpy_s(searchPath, MAX_PATH, coreRoot);
        wcscat_s(searchPath, MAX_PATH, L"\\");
        wcscat_s(searchPath, MAX_PATH, tpaExtensions[i]);

        // Find files matching the search pattern
        WIN32_FIND_DATAW findData;
        HANDLE fileHandle = FindFirstFileW(searchPath, &findData);

        if (fileHandle != INVALID_HANDLE_VALUE)
        {
            do
            {
                // Construct the full path of the trusted assembly
                wchar_t pathToAdd[MAX_PATH];
                wcscpy_s(pathToAdd, MAX_PATH, coreRoot);
                wcscat_s(pathToAdd, MAX_PATH, L"\\");
                wcscat_s(pathToAdd, MAX_PATH, findData.cFileName);

                // Check to see if TPA list needs expanded
                if (wcsnlen(pathToAdd, MAX_PATH) + (3) + wcsnlen(trustedPlatformAssemblies, tpaSize) >= tpaSize)
                {
                    // Expand, if needed
                    tpaSize *= 2;
                    wchar_t* newTPAList = new wchar_t[tpaSize];
                    wcscpy_s(newTPAList, tpaSize, trustedPlatformAssemblies);
                    trustedPlatformAssemblies = newTPAList;
                }

                // Add the assembly to the list and delimited with a semi-colon
                wcscat_s(trustedPlatformAssemblies, tpaSize, pathToAdd);
                wcscat_s(trustedPlatformAssemblies, tpaSize, L";");

                // Note that the CLR does not guarantee which assembly will be loaded if an assembly
                // is in the TPA list multiple times (perhaps from different paths or perhaps with different NI/NI.dll
                // extensions. Therefore, a real host should probably add items to the list in priority order and only
                // add a file if it's not already present on the list.
                //
                // For this simple sample, though, and because we're only loading TPA assemblies from a single path,
                // we can ignore that complication.
            } while (FindNextFileW(fileHandle, &findData));
            FindClose(fileHandle);
        }
    }


    // APP_PATHS
    // App paths are directories to probe in for assemblies which are not one of the well-known Framework assemblies
    // included in the TPA list.
    //
    // For this simple sample, we just include the directory the target application is in.
    // More complex hosts may want to also check the current working directory or other
    // locations known to contain application assets.
    wchar_t appPaths[MAX_PATH * 50];

    // Just use the targetApp provided by the user and remove the file name
    wcscpy_s(appPaths, targetAppPath);


    // APP_NI_PATHS
    // App (NI) paths are the paths that will be probed for native images not found on the TPA list.
    // It will typically be similar to the app paths.
    // For this sample, we probe next to the app and in a hypothetical directory of the same name with 'NI' suffixed to the end.
    wchar_t appNiPaths[MAX_PATH * 50];
    wcscpy_s(appNiPaths, targetAppPath);
    wcscat_s(appNiPaths, MAX_PATH * 50, L";");
    wcscat_s(appNiPaths, MAX_PATH * 50, targetAppPath);
    wcscat_s(appNiPaths, MAX_PATH * 50, L"NI");


    // NATIVE_DLL_SEARCH_DIRECTORIES
    // Native dll search directories are paths that the runtime will probe for native DLLs called via PInvoke
    wchar_t nativeDllSearchDirectories[MAX_PATH * 50];
    wcscpy_s(nativeDllSearchDirectories, appPaths);
    wcscat_s(nativeDllSearchDirectories, MAX_PATH * 50, L";");
    wcscat_s(nativeDllSearchDirectories, MAX_PATH * 50, coreRoot);


    // PLATFORM_RESOURCE_ROOTS
    // Platform resource roots are paths to probe in for resource assemblies (in culture-specific sub-directories)
    wchar_t platformResourceRoots[MAX_PATH * 50];
    wcscpy_s(platformResourceRoots, appPaths);


    DWORD domainId;

    // Setup key/value pairs for AppDomain  properties
    const wchar_t* propertyKeys[] = {
        L"TRUSTED_PLATFORM_ASSEMBLIES",
        L"APP_PATHS",
        L"APP_NI_PATHS",
        L"NATIVE_DLL_SEARCH_DIRECTORIES",
        L"PLATFORM_RESOURCE_ROOTS"
    };

    // Property values which were constructed in step 5
    const wchar_t* propertyValues[] = {
        trustedPlatformAssemblies,
        appPaths,
        appNiPaths,
        nativeDllSearchDirectories,
        platformResourceRoots
    };

    // Create the AppDomain
    hr = runtimeHost->CreateAppDomainWithManager(
        L"Sample Host AppDomain",		// Friendly AD name
        appDomainFlags,
        NULL,							// Optional AppDomain manager assembly name
        NULL,							// Optional AppDomain manager type (including namespace)
        sizeof(propertyKeys) / sizeof(wchar_t*),
        propertyKeys,
        propertyValues,
        &domainId);


   


    //DWORD exitCode = -1;
   // hr = runtimeHost->ExecuteAssembly(domainId, targetAppPath, argc - 1, (LPCWSTR*)(argc > 1 ? &argv[1] : NULL), &exitCode);

     /*
    typedef void (*bootstrap_ptr)();

   
    bootstrap_ptr managedDelegate;

    // The assembly name passed in the third parameter is a managed assembly name
    // as described at https://docs.microsoft.com/dotnet/framework/app-domains/assembly-names
    hr = coreclr_create_delegate(
        runtimeHost,
        domainId,
        "Hello, Version=1.0.0.0",
        "Hello.Program",
        "Main",
        (void**)&managedDelegate);

    managedDelegate();
    */
    
    typedef void (STDMETHODCALLTYPE MainMethodFp)(void);

    void* pfnDelegate = NULL;
    hr = runtimeHost->CreateDelegate(
        domainId,
        L"Hello, Version=1.0.0, Culture=neutral", // Target managed assembly
        L"Hello.Program",           // Target managed type
        L"Main",                                 // Target entry point (static method)
        (INT_PTR*)(&pfnDelegate));

    ((MainMethodFp*)pfnDelegate)();
   

    runtimeHost->UnloadAppDomain(domainId, true /* Wait until unload complete */);
    runtimeHost->Stop();
    runtimeHost->Release();

}

/*
void LoadDll()
{
    FILE* pFile;
    long lSize;
    char* buffer;
    size_t result;

    pFile = fopen("hello.bin", "rb");
    if (pFile == NULL) { fputs("File error", stderr); exit(1); }

    // obtain file size:
    fseek(pFile, 0, SEEK_END);
    lSize = ftell(pFile);
    rewind(pFile);

    // allocate memory to contain the whole file:
    buffer = (char*)malloc(sizeof(char) * lSize);
    if (buffer == NULL) { fputs("Memory error", stderr); exit(2); }

    // copy the file into the buffer:
    result = fread(buffer, 1, lSize, pFile);

}*/

