
// Include
#include "coreclrhost.h"

using namespace std;


#define APPDLL_FILE_NAME "Hello.dll"

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
	return string("c:\\Program Files\\dotnet\\shared\\Microsoft.NETCore.App\\3.1.6\\");
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

	// Initialize Paths
	string app_path = get_application_path();
	string runtime_path = get_runtime_path();

	string coreclr_path = string(runtime_path);
	coreclr_path.append(CORECLR_FILE_NAME);

	string appdll_path = string(app_path);
	appdll_path.append(APPDLL_FILE_NAME);


	//
	// Load coreclr
	//
	HMODULE hm_coreclr = LoadLibraryExA(coreclr_path.c_str(), NULL, 0);
	if (hm_coreclr == 0)
	{
		printf("ERROR: Failed to load coreclr from %s\n", coreclr_path.c_str());
		return -1;
	}
	else
	{
		printf("INFO: Loaded coreclr from %s\n", coreclr_path.c_str());
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
	const char* propertyKeys[] = {
				"APPBASE",
				"TRUSTED_PLATFORM_ASSEMBLIES",
				"APP_PATHS"
	};
	const char* propertyValues[] = {
		// APPBASE
		app_path.c_str(),
		// TRUSTED_PLATFORM_ASSEMBLIES
		tpa_list.c_str(),
		// APP_PATHS
		app_path.c_str()
	};

	//Initialize the CoreCLR.Creates and starts CoreCLR hostand creates an app domain
	void* hostHandle;
	unsigned int domainId;

	int hr = p_coreclr_initialize(
		coreclr_path.c_str(),        // App base path
		"CoreClrHost",       // AppDomain friendly name
		sizeof(propertyKeys) / sizeof(char*),   // Property count
		propertyKeys,       // Property names
		propertyValues,     // Property values
		&hostHandle,        // Host handle
		&domainId);         // AppDomain ID


	if (FAILED(hr))
	{
		printf("ERROR: coreclr_initialize failed 0x%08x\n", hr);
		return -1;
	}
	else
	{
		printf("INFO: coreclr_initialize\n");
	}

	//
	//Create a native callable function pointer for a managed method.
	//
	managed_ptr p_managed;

	hr = p_create_managed_delegate(
		hostHandle,
		domainId,
		"Hello", // Assembly Name 
		"Hello.Program", // Namespace.Class
		"Main", // Static Method
		(void**)&p_managed); // Pointer to managed method


	if (FAILED(hr))
	{
		printf("ERROR: coreclr_create_delegate failed\n");
		return -1;
	}
	else
	{
		printf("INFO: coreclr_create_delegate failed - status: 0x%08x\n", hr);
	}

	//
	// Call managed method
	//
	if (p_managed == 0)
	{
		printf("ERROR: p_managed invalid\n");
		return -1;
	}
	else
	{
		printf("INFO: p_managed valid\n");
	}

	p_managed();

	//
	// Shutdown CoreCLR. It unloads the app domain and stops the CoreCLR host.
	//
	hr = p_shutdown_coreclr(hostHandle, domainId);
	if (FAILED(hr))
	{
		printf("ERROR: coreclr_shutdown failed - status: 0x%08x\n", hr);
	}
	else
	{
		printf("INFO: coreclr_shutdown\n");
		return -1;
	}

	return 0;
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

