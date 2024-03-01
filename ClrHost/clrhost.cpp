// Include
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <string>
#include <windows.h>
#include <vector>

#include <ios>
#include <iosfwd>
#include <iostream>
#include <strstream>

// Using namespace
using namespace std;

#include "clrhost.h"
#include "pal.h"


// Define
#define PATH_DELIMITER ";"
#define APPDLL_FILE_NAME "Hello.dll"
#define DOTNET_VERSION 8

// Typedef
typedef char* (*managed_ptr)(void);


//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
//-----------------------------------------------------------------------------
{
	PalPaths Paths;
	
	//
	// Initialize paths
	//
	printf("[INFO] Initialize paths\n");

	pal_get_pal_paths(&Paths, DOTNET_VERSION, APPDLL_FILE_NAME);


	//
	// Load coreclr dll
	//
	void* hm_coreclr = pal_load_library(Paths.CoreCrlFileFullPath.c_str());
	if (hm_coreclr == NULL)
	{
		printf("[ERROR] Load coreclr from %s\n", Paths.CoreCrlFileFullPath.c_str());
		return -1;
	}
	else
	{
		printf("[INFO] Load coreclr from %s\n", Paths.CoreCrlFileFullPath.c_str());
	}

	//
	// Set coreclr API host function pointers
	//
	printf("[INFO] Set coreclr API host function pointers\n");

	coreclr_initialize_ptr p_coreclr_initialize = (coreclr_initialize_ptr)pal_get_export(hm_coreclr, "coreclr_initialize");
	coreclr_create_delegate_ptr p_create_managed_delegate = (coreclr_create_delegate_ptr)pal_get_export(hm_coreclr, "coreclr_create_delegate");
	coreclr_shutdown_ptr p_shutdown_coreclr = (coreclr_shutdown_ptr)pal_get_export(hm_coreclr, "coreclr_shutdown");


	if (p_coreclr_initialize == NULL)
	{
		printf("[ERROR] coreclr_initialize not found\n");
		return -1;
	}

	if (p_create_managed_delegate == NULL)
	{
		printf("[ERROR] coreclr_create_delegate not found\n");
		return -1;
	}

	if (p_shutdown_coreclr == NULL)
	{
		printf("[ERROR] coreclr_shutdown not found\n");
		return -1;
	}

	//
	// Find and set trusted platform assemblies
	//
	printf("[INFO] Find and set trusted platform assemblies\n");

	vector<string> files;
	pal_get_dll_list_for_tpa(&files, Paths.RuntimeDirPath.c_str());
	pal_get_dll_list_for_tpa(&files, Paths.AspNetDirPath.c_str());
	string tpa_list;

	for (unsigned int i = 0; i < files.size(); i++)
	{		
		tpa_list.append(files[i]);
		tpa_list.append(PATH_DELIMITER);
		//printf("%s\r\n",files[i].c_str());
	}

	printf("[INFO] Found %zd tpa assemblies\n", files.size());


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
		
		Paths.AppDirPath.c_str(),// APPBASE		
		"CLRHOST",// APP_NAME		
		tpa_list.c_str(),// TRUSTED_PLATFORM_ASSEMBLIES		
		Paths.AppDirPath.c_str()// APP_PATHS
	};

	//
	//Initialize the CoreCLR. Creates and starts CoreCLR hostand creates an app domain
	//
	void* host_handle;
	unsigned int domain_id;

	int hr = p_coreclr_initialize(
		Paths.AppDirPath.c_str(),        // App base path
		"DefaultDomain",       // AppDomain friendly name
		sizeof(property_keys) / sizeof(char*),   // Property count
		property_keys,       // Property names
		property_values,     // Property values
		&host_handle,        // Host handle
		&domain_id);         // AppDomain ID


	if (FAILED(hr))
	{
		printf("[ERROR] Initialize - 0x%08x\n", hr);
		return -1;
	}
	else
	{
		printf("[INFO] Initialize\n");
	}

	//
	// Create a native callable function pointer for a managed method.
	//
	managed_ptr p_managed;

	hr = p_create_managed_delegate(
		host_handle, // Host handle
		domain_id,  // AppDomain ID
		"Hello", // Assembly Name 
		"Hello.Program", // Namespace.Class
		"Main", // Static Method
		(void**)&p_managed); // Pointer to managed method


	if (FAILED(hr))
	{
		printf("[ERROR] Create delegate - 0x%08x\n", hr);		
		return -1;
	}
	else
	{
		printf("[INFO] Create delegate\n");
	}

	//
	// Call managed method
	//
	if (p_managed == NULL)
	{
		printf("[ERROR] Delegate invalid\n");
		return -1;
	}
	else
	{
		printf("[INFO] Delegate valid\n");
	}

	printf("[INFO] Call delegate...\n");

	
	p_managed();

	//
	// Shutdown CoreCLR. It unloads the app domain and stops the CoreCLR host.
	//
	hr = p_shutdown_coreclr(host_handle, domain_id);
	if (FAILED(hr))
	{
		printf("[ERROR] Shutdown failed - 0x%08x\n", hr);
	}
	else
	{
		printf("[INFO] Shutdown\n");
		return -1;
	}

	return 0;
}