/********************************************************************************************
 * Function used to load and activate .NET Core
 ********************************************************************************************/
 //
 // Include
 //
#include "pal.h"

// Using namespace
using namespace std;

//
// Internal functions
//

#ifdef WINDOWS

#include <windows.h>

#if defined(_WIN32) && defined(_M_IX86)
#define RUNTIME_DIR_BASE_PATH "c:\\Program Files (x86)\\dotnet\\shared\\Microsoft.NETCore.App\\"
#define ASPNET_DIR_BASE_PATH "c:\\Program Files (x86)\\dotnet\\shared\\Microsoft.AspNetCore.App\\"
#else
#define RUNTIME_DIR_BASE_PATH "c:\\Program Files\\dotnet\\shared\\Microsoft.NETCore.App\\"
#define ASPNET_DIR_BASE_PATH "c:\\Program Files\\dotnet\\shared\\Microsoft.AspNetCore.App\\"
#endif

#define CORECLR_FILE_NAME "coreclr.dll"
#define DIR_SEPARATOR "\\"
#define TPA_LIST_SEPARATOR ";"

//----------------------------------------------------------------------------------------
static inline void* pal_load_library(const char* path)
//-----------------------------------------------------------------------------------------
{
	HMODULE hm = ::LoadLibraryA(path);

	assert(hm != nullptr);

	return (void*)hm;
}
//-----------------------------------------------------------------------------------------
static inline void* pal_get_export(void* h, const char* name)
//-----------------------------------------------------------------------------------------
{
	void* f = (void*)::GetProcAddress((HMODULE)h, name);

	assert(f != nullptr);

	return f;
}

//-----------------------------------------------------------------------------------------
static inline string pal_get_app_dir_path(void)
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
static inline void pal_get_tpa_list(vector<string>* result, string dirname)
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
static inline string pal_get_max_runtime_version(string base_dir, int major_runtime_version)
//-----------------------------------------------------------------------------------------
{
	vector<string> result;

	string search_pattern = base_dir;

	search_pattern.append(std::to_string(major_runtime_version));
	search_pattern.append(".*");

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

	return result.back();
}


#else

#include <dlfcn.h>
#include <unistd.h>
#include <libgen.h>
#include <dirent.h>

#define RUNTIME_DIR_BASE_PATH "/usr/share/dotnet/shared/Microsoft.NETCore.App/"
#define ASPNET_DIR_BASE_PATH "/usr/share/dotnet/shared/Microsoft.AspNetCore.App/"


#define CORECLR_FILE_NAME "libcoreclr.so"
#define DIR_SEPARATOR "/"
#define TPA_LIST_SEPARATOR ":"

static inline void* pal_load_library(const char* path)
{
	void* h = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
	assert(h != nullptr);
	return h;
}
static inline void* pal_get_export(void* h, const char* name)
{
	void* f = dlsym(h, name);
	assert(f != nullptr);
	return f;
}

static inline string pal_get_app_dir_path()
{
	string ret = string();

	char result[PATH_MAX];
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);

	if (count != -1)
	{
		const char* paths;
		paths = dirname(result);

		ret.append(paths);
		ret.append("/");
	}

	return ret;
}

static inline string pal_get_max_runtime_version(string base_dir, int major_runtime_version)
{
	vector<string> result;

	struct dirent* entry;
	DIR* dir = opendir(base_dir.c_str());

	assert(dir != nullptr);

	while ((entry = readdir(dir)) != NULL)
	{
		string file;
		file.append(entry->d_name);
		if (file.rfind(std::to_string(major_runtime_version).c_str(), 0) == 0)
		{
			result.push_back(file);
		}
	}

	closedir(dir);

	return result.back();
}

//-----------------------------------------------------------------------------------------
static inline void pal_get_tpa_list(vector<string>* result, string dirname)
//-----------------------------------------------------------------------------------------
{
	struct dirent* entry;
	DIR* dir = opendir(dirname.c_str());

	assert(dir != nullptr);

	while ((entry = readdir(dir)) != NULL)
	{
		string file;
		file.append(entry->d_name);
		std::size_t found = file.find_last_of(".");
		if (file.substr(found + 1) == string("dll"))
		{
			file.clear();
			file.append(dirname);
			file.append(DIR_SEPARATOR);
			file.append(entry->d_name);
			result->push_back(file);
		}
	}
}


#endif

//
// Public functions
//

/*
struct PalPaths
{
	string AppDirPath;
	string AppFileName;
	string AppFileFullPath;
	string MaxRuntimeVersion;
	string RuntimeDirPath;
	string AspNetDirPath;
	string CoreCrlFileFullPath;
	string CoreCrlFileName;
	vector<string> TpaFiles;
	string TpaList;
};
*/

//-----------------------------------------------------------------------------------------
void pal_get_paths(PalPaths* paths, int major_runtime_version, const char* app_file_name)
//-----------------------------------------------------------------------------------------
{
	paths->AppDirPath = pal_get_app_dir_path();
	paths->AppFileName = string(app_file_name);

	paths->AppFileFullPath = paths->AppDirPath;
	paths->AppFileFullPath.append(paths->AppFileName);

	paths->MaxRuntimeVersion = pal_get_max_runtime_version(string(RUNTIME_DIR_BASE_PATH), major_runtime_version);

	paths->RuntimeDirPath = string(RUNTIME_DIR_BASE_PATH);
	paths->RuntimeDirPath.append(paths->MaxRuntimeVersion);

	paths->AspNetDirPath = string(ASPNET_DIR_BASE_PATH);
	paths->AspNetDirPath.append(paths->MaxRuntimeVersion);

	paths->CoreCrlFileName = string(CORECLR_FILE_NAME);
	paths->CoreCrlFileFullPath = paths->RuntimeDirPath;
	paths->CoreCrlFileFullPath.append(DIR_SEPARATOR);
	paths->CoreCrlFileFullPath.append(CORECLR_FILE_NAME);

	paths->TpaFiles.clear();
	pal_get_tpa_list(&paths->TpaFiles, paths->RuntimeDirPath.c_str());
	pal_get_tpa_list(&paths->TpaFiles, paths->AspNetDirPath.c_str());

	for (unsigned int i = 0; i < paths->TpaFiles.size(); i++)
	{
		paths->TpaList.append(paths->TpaFiles[i]);
		paths->TpaList.append(TPA_LIST_SEPARATOR);
	}
}

/*
struct PalPointers
{
	void* PtrCoreCrl;
	coreclr_initialize_ptr PtrInitialize;
	coreclr_create_delegate_ptr PtrCreateDelegate;
	coreclr_shutdown_2_ptr PtrShutdown;
};
*/

//-----------------------------------------------------------------------------------------
void pal_get_pointers(PalPointers* pointers, const char* corecrl_file_name)
//-----------------------------------------------------------------------------------------
{
	// Load coreclr.dll	
	pointers->PtrCoreCrl = pal_load_library(corecrl_file_name);
	assert(pointers->PtrCoreCrl != nullptr);

	// Set coreclr API host function pointers
	pointers->PtrInitialize = (coreclr_initialize_ptr)pal_get_export(pointers->PtrCoreCrl, "coreclr_initialize");
	assert(pointers->PtrInitialize != nullptr);

	pointers->PtrCreateDelegate = (coreclr_create_delegate_ptr)pal_get_export(pointers->PtrCoreCrl, "coreclr_create_delegate");
	assert(pointers->PtrCreateDelegate != nullptr);

	pointers->PtrShutdown = (coreclr_shutdown_2_ptr)pal_get_export(pointers->PtrCoreCrl, "coreclr_shutdown_2");
	assert(pointers->PtrShutdown != nullptr);

	pointers->PtrErrorWriter = (coreclr_set_error_writer_ptr)pal_get_export(pointers->PtrCoreCrl, "coreclr_set_error_writer");
	assert(pointers->PtrErrorWriter != nullptr);
}
