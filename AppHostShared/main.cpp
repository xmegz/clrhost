/*-----------------------------------------------------------------------------
 * Project:    CrlHost
 * Repository: https://github.com/xmegz/clrhost
 * Author:     Pádár Tamás
 -----------------------------------------------------------------------------*/

//
// Include
//
#ifdef WINDOWS
	#include <windows.h>
#else
	#define FAILED(hr) (((int)(hr)) < 0)	
#endif

#include "pal.h"

//
// Using namespace
//
using namespace std;

//
// Defines
//
#define DOTNET_VERSION 8

//
// Typedefs
//
typedef int (CORECLR_CALLING_CONVENTION* AssemblyLoadEnrtyPoint)(
	const void*	assembly_bytes,		/* Bytes of the assembly to load */
	size_t		assembly_bytes_len,	/* Byte length of the assembly to load */
	const void*	symbols_bytes,		/* Optional. Bytes of the symbols for the assembly */
	size_t		symbols_bytes_len,	/* Optional. Byte length of the symbols for the assembly */
	void*		load_context,		/* Extensibility parameter (currently unused and must be 0) */
	void*		reserved);			/* Extensibility parameter (currently unused and must be 0) */

typedef int (CORECLR_CALLING_CONVENTION* NativeEntryPoint)(
	int			argc,				/* Argument count */
	const char* argv[]);			/*Argument value */


//-----------------------------------------------------------------------------
int main(int argc, const char* argv[])
//-----------------------------------------------------------------------------
{
	PalPaths Paths;
	PalPointers Pointers;
	PalAssembly Assembly;

	pal_info("Apphost [%s %s]\n",__DATE__, __TIME__);
	
	pal_get_paths(&Paths, DOTNET_VERSION);
	pal_info("CoreCLR path:%s\n", Paths.CoreCrlFileFullPath.c_str());

	pal_get_pointers(&Pointers, Paths.CoreCrlFileFullPath.c_str());
	pal_info("CoreCLR pointer:0x%08x\n", Pointers.PtrCoreCrl);

	Pointers.PtrSetErrorWriter(pal_trace);
	pal_info("Error writer setted\n");


	//
	// Setup AppDomain properties
	//
	const char* property_keys[] = {
				"APPBASE",
				"APP_NAME",
				"TRUSTED_PLATFORM_ASSEMBLIES",
				"APP_PATHS",
				"NATIVE_DLL_SEARCH_DIRECTORIES",
				"HOST_RUNTIME_CONTRACT"
	};
	
	const char* property_values[] = {
		Paths.AppDirPath.c_str(),	   // APPBASE
		"APPHOST",					   // APP_NAME
		Paths.TpaList.c_str(),		   // TRUSTED_PLATFORM_ASSEMBLIES
		Paths.AppDirPath.c_str(),	   // APP_PATHS
		Paths.ProbeList.c_str(),	   // NATIVE_DLL_SEARCH_DIRECTORIES
		Paths.RuntimeContract.c_str(), // HOST_RUNTIME_CONTRACT
	};

	int property_count = sizeof(property_keys) / sizeof(char*);

	pal_debug("Paths.ProbeList: %s\n\n\n", Paths.ProbeList.c_str());
	pal_debug("Paths.TpaList: %s\n\n\n", Paths.TpaList.c_str());
	
	//
	// Initialize the CoreCLR. Creates and starts CoreCLR host and creates an AppDomain
	//
	void* host_handle;
	unsigned int domain_id;

	int hr = Pointers.PtrInitialize(
		Paths.AppDirPath.c_str(),	// App base path (exe path)
		"DefaultDomain",			// AppDomain friendly name
		property_count,				// Property count
		property_keys,				// Property names
		property_values,			// Property values
		&host_handle,				// Host handle
		&domain_id);				// AppDomain ID


	if (FAILED(hr))
		pal_error(pal_error::init, "Initialize - 0x%08x\n", hr);
	else
		pal_info("Initialize OK\n");

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
		pal_error(pal_error::create_delegate, "Create delegate - 0x%08x\n", hr);
	else
		pal_info("Create delegate OK\n");


	//
	// Load .net assembly from resource to memory
	//	
	pal_load_assembly(&Assembly);

	pal_info("Call assembly load addr:0x%08x size:%ld\n",Assembly.Bytes, Assembly.Size);

	hr = PtrAssemblyLoadEnrtyPoint(Assembly.Bytes, Assembly.Size, NULL, 0, NULL, NULL);

	pal_info("Assembly load ret - 0x%08x\n", hr);
	

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
		pal_error(pal_error::create_delegate, "Create delegate - 0x%08x\n", hr);
	else
		pal_info("Create delegate OK\n");


	//
	// Call managed method
	//
	if (PtrNativeEntryPoint == NULL)
		pal_error(pal_error::delegate, "Delegate entryPoint invalid\n");
	else
		pal_info("Delegate entryPoint OK\n");

	pal_info("Call entryPoint delegate...\n");
	
	++argv; // Skip first argument
	argc--;

	PtrNativeEntryPoint(argc, argv);

	//
	// Shutdown CoreCLR. It unloads the AppDomain and stops the CoreCLR host.
	//
	int exitCode;

	hr = Pointers.PtrShutdown(host_handle, domain_id, &exitCode);
	if (FAILED(hr))
		pal_error(pal_error::shutdown_crl, "Shutdown failed - 0x%08x\n", hr);
	else
		pal_info("Shutdown exitCode:%d\n", exitCode);

	//
	// Free resource memory
	//
	Assembly.Free();

	return exitCode;
}