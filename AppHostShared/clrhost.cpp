// Include
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <string>
#ifdef WINDOWS
#include <windows.h>
#else
#define FAILED(hr) (((int)(hr)) < 0)
#include <cstdarg>
#endif
#include <vector>

#include <ios>
#include <iosfwd>
#include <iostream>
#include <strstream>

#include "pal.h"


// Using namespace
using namespace std;

// Define
#define APPDLL_FILE_NAME "Hello.dll"
#define DOTNET_VERSION 8

/*
AssemblyName("System.Private.CoreLib");
TypeName("Internal.Runtime.InteropServices.ComponentActivator");
MethodName("LoadAssemblyBytes");
*/

typedef int (CORECLR_CALLING_CONVENTION* AssemblyLoadEnrtyPoint)(
	const void* assembly_bytes      /* Bytes of the assembly to load */,
	size_t     assembly_bytes_len   /* Byte length of the assembly to load */,
	const void* symbols_bytes       /* Optional. Bytes of the symbols for the assembly */,
	size_t     symbols_bytes_len    /* Optional. Byte length of the symbols for the assembly */,
	void* load_context        /* Extensibility parameter (currently unused and must be 0) */,
	void* reserved            /* Extensibility parameter (currently unused and must be 0) */);

typedef int (CORECLR_CALLING_CONVENTION* NativeEntryPoint)(
	int argc, /* Argument count */
	const char* argv[] /*Argument value */
	);

// Enum
enum error
{
	init = -1,
	create_delegate = -2,
	delegate = -3,
	shutdown_crl = -4
};

//-----------------------------------------------------------------------------
static inline void info(const char* format, ...)
//-----------------------------------------------------------------------------
{
	printf("[INFO] ");

	va_list args;
	va_start(args, format);
	vfprintf(stdout, format, args);
	va_end(args);
}

//-----------------------------------------------------------------------------
static inline void error(int code, const char* format, ...)
//-----------------------------------------------------------------------------
{
	printf("[ERROR] Code:%d ", code);

	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);

	exit(code);
}

//-----------------------------------------------------------------------------
static void trace(const char* message)
//-----------------------------------------------------------------------------
{
	printf("[TRACE] %s", message);
}

//-----------------------------------------------------------------------------
int main(int argc, const char* argv[])
//-----------------------------------------------------------------------------
{
	PalPaths Paths;
	PalPointers Pointers;
	PalAssembly Assembly;

	info("Initialize paths\n");
	pal_get_paths(&Paths, DOTNET_VERSION, APPDLL_FILE_NAME);

	info("CoreCrl path: %s\n", Paths.CoreCrlFileFullPath.c_str());

	info("Initialize pointers\n");
	pal_get_pointers(&Pointers, Paths.CoreCrlFileFullPath.c_str());

	info("CoreCrl pointer: %08x\n", Pointers.PtrCoreCrl);

	info("Register error writer\n");
	Pointers.PtrErrorWriter(trace);


	//
	// Set app domain properties
	//
	const char* property_keys[] = {
				"APPBASE",
				"APP_NAME",
				"TRUSTED_PLATFORM_ASSEMBLIES",
				"APP_PATHS"
	};
	const char* property_values[] = {

		Paths.AppDirPath.c_str(),	// APPBASE		
		"CLRHOST",					// APP_NAME		
		Paths.TpaList.c_str(),		// TRUSTED_PLATFORM_ASSEMBLIES		
		Paths.AppDirPath.c_str()	// APP_PATHS
	};

	//
	// Initialize the CoreCLR. Creates and starts CoreCLR hostand creates an app domain
	//
	void* host_handle;
	unsigned int domain_id;

	int hr = Pointers.PtrInitialize(
		Paths.AppDirPath.c_str(),        // App base path
		"DefaultDomain",       // AppDomain friendly name
		sizeof(property_keys) / sizeof(char*),   // Property count
		property_keys,       // Property names
		property_values,     // Property values
		&host_handle,        // Host handle
		&domain_id);         // AppDomain ID


	if (FAILED(hr))
		error(error::init, "Initialize - 0x%08x\n", hr);
	else
		info("Initialize OK\n");

	//
	// Create a native callable function pointer for a managed method.
	//
	AssemblyLoadEnrtyPoint PtrAssemblyLoadEnrtyPoint = NULL;

	hr = Pointers.PtrCreateDelegate(
		host_handle,			// Host handle
		domain_id,				// AppDomain ID
		"System.Private.CoreLib",				// Assembly Name 
		"Internal.Runtime.InteropServices.ComponentActivator",		// Namespace.Class
		"LoadAssemblyBytes",		// Static Method
		(void**)&PtrAssemblyLoadEnrtyPoint);	// Pointer to managed method


	if (FAILED(hr))
		error(error::create_delegate, "Create delegate - 0x%08x\n", hr);
	else
		info("Create delegate OK\n");

#ifdef WINDOWS	

	pal_load_assembly(&Assembly);

	info("Call assembly load.size: %ld\n", Assembly.Size);

	int loadCode = PtrAssemblyLoadEnrtyPoint(Assembly.Bytes, Assembly.Size, NULL, 0, NULL, NULL);

	info("Assembly load  ret code: %08x\n", loadCode);

#endif


	//
	// Create a native callable function pointer for a managed method.
	//
	NativeEntryPoint PtrNativeEntryPoint = NULL;

	hr = Pointers.PtrCreateDelegate(
		host_handle,			// Host handle
		domain_id,				// AppDomain ID
		"Hello",				// Assembly Name 
		"Hello.Program",		// Namespace.Class
		"NativeEntryPoint",		// Static Method
		(void**)&PtrNativeEntryPoint);	// Pointer to managed method


	if (FAILED(hr))
		error(error::create_delegate, "Create delegate - 0x%08x\n", hr);
	else
		info("Create delegate OK\n");


	//
	// Call managed method
	//
	if (PtrNativeEntryPoint == NULL)
		error(error::delegate, "Delegate invalid\n");
	else
		info("Delegate OK\n");

	info("Call delegate...\n");

	PtrNativeEntryPoint(argc, argv);

	int exitCode;

	//
	// Shutdown CoreCLR. It unloads the app domain and stops the CoreCLR host.
	//
	hr = Pointers.PtrShutdown(host_handle, domain_id, &exitCode);
	if (FAILED(hr))
		error(error::shutdown_crl, "Shutdown failed - 0x%08x\n", hr);
	else
		info("Shutdown exitCode:%d\n", exitCode);

	return exitCode;
}