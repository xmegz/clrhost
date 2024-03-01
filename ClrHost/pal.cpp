// Using namespace
using namespace std;

#include "pal.h"
/********************************************************************************************
 * Function used to load and activate .NET Core
 ********************************************************************************************/

#ifdef WINDOWS

#include <windows.h>

#if defined(_WIN32) && defined(_M_IX86)
	#define RUNTIME_DIR_BASE_PATH "c:\\Program Files (x86)\\dotnet\\shared\\Microsoft.NETCore.App\\"
	#define ASPNET_BASE_PATH "c:\\Program Files (x86)\\dotnet\\shared\\Microsoft.AspNetCore.App\\"
#else
	#define RUNTIME_DIR_BASE_PATH "c:\\Program Files\\dotnet\\shared\\Microsoft.NETCore.App\\"
	#define ASPNET_DIR_BASE_PATH "c:\\Program Files\\dotnet\\shared\\Microsoft.AspNetCore.App\\"
#endif

#define CORECLR_FILE_NAME "coreclr.dll"
#define DIR_SEPARATOR "\\"

//-----------------------------------------------------------------------------------------
void* pal_load_library(const char* path)
//-----------------------------------------------------------------------------------------
{
	HMODULE hm = ::LoadLibraryA(path);
	
	assert(hm != nullptr);
	
	return (void*)hm;
}
//-----------------------------------------------------------------------------------------
void* pal_get_export(void* h, const char* name)
//-----------------------------------------------------------------------------------------
{
	void* f = ::GetProcAddress((HMODULE)h, name);
	
	assert(f != nullptr);
	
	return f;
}

//-----------------------------------------------------------------------------------------
string pal_get_app_dir_path(void)
//-----------------------------------------------------------------------------------------
{
	HRESULT hr = S_OK;

	char buffer[MAX_PATH] = { 0 };

	hr = GetModuleFileNameA(NULL, buffer, MAX_PATH);

	assert(hr > 0);

	std::string::size_type pos = string(buffer).find_last_of("\\/");
	return string(buffer).substr(0, pos + 1);
}

//-----------------------------------------------------------------------------------------
string pal_get_runtime_dir_base_path(void)
//-----------------------------------------------------------------------------------------
{
	return string(RUNTIME_DIR_BASE_PATH);
}

//-----------------------------------------------------------------------------------------
string pal_get_aspnet_dir_base_path(void)
//-----------------------------------------------------------------------------------------
{
	return string(ASPNET_DIR_BASE_PATH);
}

//-----------------------------------------------------------------------------------------
string pal_get_corecrl_file_name(void)
//-----------------------------------------------------------------------------------------
{
	return string(CORECLR_FILE_NAME);
}

//-----------------------------------------------------------------------------------------
void pal_get_dll_list_for_tpa(vector<string>* result, string dirname)
//-----------------------------------------------------------------------------------------
{
	string search_pattern = dirname;
	search_pattern.append(DIR_SEPARATOR);
	search_pattern.append("*.dll");

	WIN32_FIND_DATAA ffd;
	HANDLE h = ::FindFirstFileA(search_pattern.c_str(), &ffd);

	assert(h != nullptr);

	string first;
	first.append(dirname);
	first.append(DIR_SEPARATOR);
	first.append(string(ffd.cFileName));
	result->push_back(first);

	while (FindNextFileA(h, &ffd))
	{
		string item;
		item.append(dirname);
		item.append(DIR_SEPARATOR);
		item.append(string(ffd.cFileName));
		result->push_back(item);
	}
}

//-----------------------------------------------------------------------------------------
string pal_get_max_runtime_version(string base_dir, int version)
//-----------------------------------------------------------------------------------------
{
	vector<string> result;

	string search_pattern = base_dir;
	
	search_pattern.append(std::to_string(version));
	search_pattern.append(".");
	search_pattern.append("*");

	WIN32_FIND_DATAA ffd;
	HANDLE h = ::FindFirstFileA(search_pattern.c_str(), &ffd);

	assert(h != nullptr);

	string first;	
	first.append(string(ffd.cFileName));
	result.push_back(first);

	while (FindNextFileA(h, &ffd))
	{
		string item;		
		item.append(string(ffd.cFileName));
		result.push_back(item);
	}

	return result.front();
}


#else

#define RUNTIME_DIR_BASE_PATH "/usr/share/dotnet/shared/Microsoft.AspNetCore.App"
#define ASPNET_DIR_BASE_PATH  "/usr/share/dotnet/shared/Microsoft.NETCore.App"

#define CORECLR_FILE_NAME "coreclr.so"


void* pal_load_library(const char_t* path)
{
	void* h = dlopen(pa, RTLD_LAZY | RTLD_LOCAL);
	assert(h != nullptr);
	return h;
}
void* pal_get_export(void* h, const char* name)
{
	void* f = dlsym(h, name);
	assert(f != nullptr);
	return f;
}

string pal_get_app_dir_path()
{
	string ret = string();

	char result[PATH_MAX];
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);

	if (count != -1)
	{
		const char* path;
		path = dirname(result);

		ret.append(path);
		ret.append("/");
	}

	return ret;
}
#endif


/*
 struct StartupPaths
{
	string AppDirPath;
	string AppFileName;
	string AppFileFullPath;
	string RuntimeDirPath;
	string AspNetDirPath;
	string CoreCrlFileFullPath;
	string CoreCrlFileName;
};
*/

void pal_get_pal_paths(PalPaths* path, int dotnet_version, const char * fileName)
{
	path->AppDirPath = pal_get_app_dir_path();
	path->AppFileName = string(fileName);
	
	path->AppFileFullPath = pal_get_app_dir_path();	
	path->AppFileFullPath.append(string(fileName));
	
	path->MaxRuntimeVersion = pal_get_max_runtime_version(pal_get_runtime_dir_base_path(), dotnet_version);

	path->RuntimeDirPath = pal_get_runtime_dir_base_path();	
	path->RuntimeDirPath.append(path->MaxRuntimeVersion);

	path->AspNetDirPath = pal_get_aspnet_dir_base_path();	
	path->AspNetDirPath.append(path->MaxRuntimeVersion);

	path->CoreCrlFileName = string(CORECLR_FILE_NAME);
	path->CoreCrlFileFullPath = path->RuntimeDirPath;
	path->CoreCrlFileFullPath.append(DIR_SEPARATOR);
	path->CoreCrlFileFullPath.append(CORECLR_FILE_NAME);
}

