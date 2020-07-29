// Include
#include "clrhost.h"

// Using namespace
using namespace std;

// Define
#define APPDLL_FILE_NAME "Hello.dll"

// Typedef
typedef char* (*managed_ptr)(void);

//-----------------------------------------------------------------------------
string get_application_path()
//-----------------------------------------------------------------------------
{
	HRESULT hr = S_OK;

	char buffer[MAX_PATH] = { 0 };

	hr = GetModuleFileNameA(NULL, buffer, MAX_PATH);

	if (FAILED(hr))
		return string();

	string::size_type pos = string(buffer).find_last_of("\\/");
	return std::string(buffer).substr(0, pos + 1);
}

//-----------------------------------------------------------------------------
string get_runtime_path()
//-----------------------------------------------------------------------------
{
	return string(CORECLR_PATH);
}

//-----------------------------------------------------------------------------
vector<string> get_dll_files_from_dll(string dirname)
//-----------------------------------------------------------------------------
{
	vector<string> result;
	dirname.append("*.dll");

	WIN32_FIND_DATAA ffd;
	HANDLE h = ::FindFirstFileA(dirname.c_str(), &ffd);

	result.push_back(string(ffd.cFileName));

	while (FindNextFileA(h, &ffd))
		result.push_back(string(ffd.cFileName));

	return result;
}

//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
//-----------------------------------------------------------------------------
{
	//
	// Initialize Paths
	//
	string app_path = get_application_path();
	string runtime_path = get_runtime_path();

	string coreclr_path = string(runtime_path);
	coreclr_path.append(CORECLR_FILE_NAME);

	string appdll_path = string(app_path);
	appdll_path.append(APPDLL_FILE_NAME);


	//
	// Load coreclr dll
	//
	HMODULE hm_coreclr = LoadLibraryExA(coreclr_path.c_str(), NULL, 0);
	if (hm_coreclr == 0)
	{
		printf("ERROR: Load from %s\n", coreclr_path.c_str());
		return -1;
	}
	else
	{
		printf("INFO: Load from %s\n", coreclr_path.c_str());
	}

	//
	// Set coreclr API host function pointers
	//
	coreclr_initialize_ptr p_coreclr_initialize = (coreclr_initialize_ptr)GetProcAddress(hm_coreclr, "coreclr_initialize");
	coreclr_create_delegate_ptr p_create_managed_delegate = (coreclr_create_delegate_ptr)GetProcAddress(hm_coreclr, "coreclr_create_delegate");
	coreclr_shutdown_ptr p_shutdown_coreclr = (coreclr_shutdown_ptr)GetProcAddress(hm_coreclr, "coreclr_shutdown");


	if (p_coreclr_initialize == NULL)
	{
		printf("ERROR: coreclr_initialize not found");
		return -1;
	}

	if (p_create_managed_delegate == NULL)
	{
		printf("ERROR: coreclr_create_delegate not found");
		return -1;
	}

	if (p_shutdown_coreclr == NULL)
	{
		printf("ERROR: coreclr_shutdown not found");
		return -1;
	}

	//
	// Find and set trusted platform assemblies
	//
	vector<string> files = get_dll_files_from_dll(runtime_path.c_str());
	string tpa_list;

	for (unsigned int i = 0; i < files.size(); i++)
	{
		tpa_list.append(runtime_path);
		tpa_list.append(FS_SEPARATOR);
		tpa_list.append(files[i]);
		tpa_list.append(PATH_DELIMITER);
	}

	//
	// Set app domain properties
	//
	const char* property_keys[] = {
				"APPBASE",
				"TRUSTED_PLATFORM_ASSEMBLIES",
				"APP_PATHS"
	};
	const char* property_values[] = {
		// APPBASE
		app_path.c_str(),
		// TRUSTED_PLATFORM_ASSEMBLIES
		tpa_list.c_str(),
		// APP_PATHS
		app_path.c_str()
	};

	//
	//Initialize the CoreCLR.Creates and starts CoreCLR hostand creates an app domain
	//
	void* host_handle;
	unsigned int domain_id;

	int hr = p_coreclr_initialize(
		coreclr_path.c_str(),        // App base path
		"CoreClrHost",       // AppDomain friendly name
		sizeof(property_keys) / sizeof(char*),   // Property count
		property_keys,       // Property names
		property_values,     // Property values
		&host_handle,        // Host handle
		&domain_id);         // AppDomain ID


	if (FAILED(hr))
	{
		printf("ERROR: Initialize - 0x%08x\n", hr);
		return -1;
	}
	else
	{
		printf("INFO: Initialize\n");
	}

	//
	//Create a native callable function pointer for a managed method.
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
		printf("ERROR: Create delegate - 0x%08x\n", hr);		
		return -1;
	}
	else
	{
		printf("INFO: Create delegate\n");
	}

	//
	// Call managed method
	//
	if (p_managed == 0)
	{
		printf("ERROR: Delegate invalid\n");
		return -1;
	}
	else
	{
		printf("INFO: Delegate valid\n");
	}

	printf("INFO: Call delegate...\n");

	p_managed();

	//
	// Shutdown CoreCLR. It unloads the app domain and stops the CoreCLR host.
	//
	hr = p_shutdown_coreclr(host_handle, domain_id);
	if (FAILED(hr))
	{
		printf("ERROR: Shutdown failed - 0x%08x\n", hr);
	}
	else
	{
		printf("INFO: Shutdown\n");
		return -1;
	}

	return 0;
}

