/*-----------------------------------------------------------------------------
 * Project:    CrlHost
 * Repository: https://github.com/xmegz/clrhost
 * Author:     Pádár Tamás
 -----------------------------------------------------------------------------*/

 //
 // Include
 //
#define _CRT_SECURE_NO_WARNINGS 
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#include "pal.h"

//
// Using namespace
//
using namespace std;


//
// Internal functions
//

#ifdef WINDOWS

#include <windows.h>

#define RUNTIME_DIR_BASE_PATH "c:\\Program Files\\dotnet\\shared\\Microsoft.NETCore.App\\"
#define ASPNET_DIR_BASE_PATH "c:\\Program Files\\dotnet\\shared\\Microsoft.AspNetCore.App\\"

#define CORECLR_FILE_NAME "coreclr.dll"
#define DIR_SEPARATOR "\\"
#define TPA_LIST_SEPARATOR ";"
#define ASM_ID "IDR_RCDATA1"
#define RUNTIME_IDENTIFIER "win-x64"

//-----------------------------------------------------------------------------
static inline void* pal_load_library(const char* path)
//-----------------------------------------------------------------------------
{
	HMODULE hm = ::LoadLibraryA(path);

	pal_assert(hm != nullptr, "pal_get_export", "not exist %s", path);

	return (void*)hm;
}
//-----------------------------------------------------------------------------
static inline void* pal_get_export(void* h, const char* name)
//-----------------------------------------------------------------------------
{
	void* f = (void*)::GetProcAddress((HMODULE)h, name);

	pal_assert(f != nullptr, "pal_get_export", "not exist %s", name);

	return f;
}

//-----------------------------------------------------------------------------
static inline string pal_get_app_dir_path(void)
//-----------------------------------------------------------------------------
{
	HRESULT hr = S_OK;

	char buffer[MAX_PATH] = { 0 };

	hr = GetModuleFileNameA(NULL, buffer, MAX_PATH);

	pal_assert(hr > 0, "pal_get_app_dir_path", "not found");


	std::string::size_type pos = string(buffer).find_last_of("\\/");
	return string(buffer).substr(0, pos + 1);
}

//-----------------------------------------------------------------------------
static inline void pal_get_tpa_list(vector<string>* result, string dirname)
//-----------------------------------------------------------------------------
{
	struct stat st {};

	// Check dir is exist
	if (stat(dirname.c_str(), &st) != 0)
		return;

	string search_pattern = dirname;
	search_pattern.append(DIR_SEPARATOR);
	search_pattern.append("*.dll");

	WIN32_FIND_DATAA ffd;
	HANDLE h = ::FindFirstFileA(search_pattern.c_str(), &ffd);

	if (INVALID_HANDLE_VALUE == h)
		return;

	string first;
	first.append(dirname);
	first.append(DIR_SEPARATOR);
	first.append(string(ffd.cFileName));
	result->push_back(first);

	while (FindNextFileA(h, &ffd))
	{
		string item;
		item.append(dirname);
		if (dirname.back() != DIR_SEPARATOR[0])
			item.append(DIR_SEPARATOR);
		item.append(string(ffd.cFileName));
		result->push_back(item);
	}
}

//-----------------------------------------------------------------------------
static inline void pal_get_probe_paths(vector<string>* result, string base_path)
//-----------------------------------------------------------------------------
{	
	string path;

	path = base_path;
	path.append("runtimes\\win-x64\\native");
	result->push_back(path);

	path = base_path;
	path.append("runtimes\\win\\lib\\netcoreapp3.0");
	result->push_back(path);
	
	path = base_path;
	path.append("runtimes\\win\\lib\\netstandard2.0");
	result->push_back(path);	
	
}

//-----------------------------------------------------------------------------
static inline string pal_get_max_rt_version(string base_dir, int major_rt_version)
//-----------------------------------------------------------------------------
{
	vector<string> result;

	string search_pattern = base_dir;

	search_pattern.append(std::to_string(major_rt_version));
	search_pattern.append(".*");

	WIN32_FIND_DATAA ffd;
	HANDLE h = ::FindFirstFileA(search_pattern.c_str(), &ffd);

	pal_assert(h != nullptr, "pal_get_max_rt_version", "dir not exist %s", search_pattern.c_str());

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

//-----------------------------------------------------------------------------
static inline void pal_load_resource(const char* identifier, PalAssembly* assembly)
//-----------------------------------------------------------------------------
{
	HMODULE hModule = GetModuleHandle(NULL); // get the handle to the current module (the executable file)
	pal_assert(hModule != nullptr, "pal_load_resource", "module not found");

	HRSRC hResource = FindResourceA(hModule, identifier, MAKEINTRESOURCEA(10)); // substitute RESOURCE_ID and RESOURCE_TYPE.	
	pal_assert(hResource != nullptr, "pal_load_resource", "resource not found");

	HGLOBAL hMemory = LoadResource(hModule, hResource);
	pal_assert(hMemory != nullptr, "pal_load_resource", "load error");

	DWORD dwSize = SizeofResource(hModule, hResource);
	pal_assert(dwSize != 0, "pal_load_resource", "resource size invalid");

	LPVOID lpAddress = LockResource(hMemory);
	pal_assert(lpAddress != nullptr, "pal_load_resource", "lock resource error");


	if (dwSize)
	{
		assembly->Bytes = (char*)malloc(dwSize);
		memcpy(assembly->Bytes, lpAddress, dwSize);
		assembly->Size = dwSize;
	}
}

#else

#include <dlfcn.h>
#include <unistd.h>
#include <libgen.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <elf.h>
#include <cstdarg>

#define RUNTIME_DIR_BASE_PATH "/usr/share/dotnet/shared/Microsoft.NETCore.App/"
#define ASPNET_DIR_BASE_PATH "/usr/share/dotnet/shared/Microsoft.AspNetCore.App/"


#define CORECLR_FILE_NAME "libcoreclr.so"
#define DIR_SEPARATOR "/"
#define TPA_LIST_SEPARATOR ":"
#define ASM_ID ".idr_rcdata1"
#define RUNTIME_IDENTIFIER "linux-x64"

//-----------------------------------------------------------------------------
static inline void* pal_load_library(const char* path)
//-----------------------------------------------------------------------------
{
	void* h = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
	pal_assert(h != nullptr, "pal_load_library", "not exist %s", path);
	return h;
}

//-----------------------------------------------------------------------------
static inline void* pal_get_export(void* h, const char* name)
//-----------------------------------------------------------------------------
{
	void* f = dlsym(h, name);
	pal_assert(h != nullptr, "pal_get_export", "not exist %s", name);
	return f;
}

//-----------------------------------------------------------------------------
static inline string pal_get_app_dir_path()
//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
static inline string pal_get_max_rt_version(string base_dir, int major_rt_version)
//-----------------------------------------------------------------------------
{
	vector<string> result;

	struct dirent* entry;
	DIR* dir = opendir(base_dir.c_str());

	pal_assert(dir != nullptr, "pal_get_max_rt_version", "dir not exist %s", base_dir.c_str());

	while ((entry = readdir(dir)) != NULL)
	{
		string file;
		file.append(entry->d_name);
		if (file.rfind(std::to_string(major_rt_version).c_str(), 0) == 0)
		{
			result.push_back(file);
		}
	}

	closedir(dir);

	return result.back();
}

//-----------------------------------------------------------------------------
static inline void pal_get_tpa_list(vector<string>* result, string dirname)
//-----------------------------------------------------------------------------
{
	struct stat st {};

	// Check dir is exist
	if (stat(dirname.c_str(), &st) != 0)
		return;

	struct dirent* entry;
	DIR* dir = opendir(dirname.c_str());

	pal_assert(dir != nullptr, "pal_get_tpa_list", "dir not exist %s", dirname.c_str());

	while ((entry = readdir(dir)) != NULL)
	{
		string file;
		file.append(entry->d_name);
		std::size_t found = file.find_last_of(".");
		if (file.substr(found + 1) == string("dll"))
		{
			file.clear();
			file.append(dirname);
			if (dirname.back() != DIR_SEPARATOR[0])
				file.append(DIR_SEPARATOR);
			file.append(entry->d_name);
			result->push_back(file);
		}
	}
}
//-----------------------------------------------------------------------------
static inline void pal_get_probe_paths(vector<string>* result, string base_path)
//-----------------------------------------------------------------------------
{	
	string path;
	
	path = base_path;
	path.append("runtimes/linux-x64/native/");	
	result->push_back(path);

	path = base_path;
	path.append("runtimes/linux/lib/netstandard2.0/");	
	result->push_back(path);

	path = base_path;
	path.append("runtimes/unix/lib/netcoreapp3.0/");	
	result->push_back(path);	
}

//-----------------------------------------------------------------------------
static inline void pal_load_resource(const char* identifier, PalAssembly* assembly)
//-----------------------------------------------------------------------------
{
	struct stat st {};
	const char* fname = "/proc/self/exe";

	int ret = stat(fname, &st);
	(void)ret; // dummy

	assert(ret == 0);

	int fd = open(fname, O_RDONLY);
	char* p = (char*)mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

	Elf64_Ehdr* ehdr = (Elf64_Ehdr*)p;
	Elf64_Shdr* shdr = (Elf64_Shdr*)(p + ehdr->e_shoff);
	int shnum = ehdr->e_shnum;

	Elf64_Shdr* sh_strtab = &shdr[ehdr->e_shstrndx];
	const char* const sh_strtab_p = p + sh_strtab->sh_offset;

	bool found = false;

	for (int i = 0; i < shnum; ++i) {
		const char* sname = sh_strtab_p + shdr[i].sh_name;

		if (!strcmp(sname, identifier))
		{
			//printf("%2d: %4d '%s' %4lu %4lu\n", i, shdr[i].sh_name, sname, shdr[i].sh_offset, shdr[i].sh_size);

			assembly->Bytes = (char*)malloc(shdr[i].sh_size);
			memcpy(assembly->Bytes, p + shdr[i].sh_offset, shdr[i].sh_size);
			assembly->Size = (int)shdr[i].sh_size;

			found = true;
		}
	}

	pal_assert(found == true, "pal_load_resource", "identifier not found %s", identifier);

	munmap(p, st.st_size);
	close(fd);
}


#endif

//
// Public functions
//

//-----------------------------------------------------------------------------
void pal_assert(bool condition, const char* function, const char* format, ...)
//-----------------------------------------------------------------------------
{
	if (condition == false)
	{
		fprintf(stderr, "[ASSERT] ");
		fprintf(stderr, "(%s) ", function);

		va_list args;
		va_start(args, format);
		vfprintf(stderr, format, args);
		va_end(args);

		exit(pal_error::assert);
	}
}

//-----------------------------------------------------------------------------
void pal_debug(const char* format, ...)
//-----------------------------------------------------------------------------
{
#ifdef _DEBUG
	fprintf(stdout, "[DEBUG] ");

	va_list args;
	va_start(args, format);
	vfprintf(stdout, format, args);
	va_end(args);
#endif
}

//-----------------------------------------------------------------------------
void pal_info(const char* format, ...)
//-----------------------------------------------------------------------------
{
	fprintf(stdout, "[INFO] ");

	va_list args;
	va_start(args, format);
	vfprintf(stdout, format, args);
	va_end(args);
}

//-----------------------------------------------------------------------------
void pal_error(int code, const char* format, ...)
//-----------------------------------------------------------------------------
{
	fprintf(stderr, "[ERROR] Code:%d ", code);

	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);

	exit(code);
}

//-----------------------------------------------------------------------------
void pal_trace(const char* message)
//-----------------------------------------------------------------------------
{
	fprintf(stdout, "[TRACE] %s", message);
}

//-----------------------------------------------------------------------------
inline size_t pal_utf8string(const string str, char* out_buffer, size_t buffer_len)
//-----------------------------------------------------------------------------
{
	size_t len = str.size() + 1;
	
	if (buffer_len < len)
		return len;	
	
	strncpy(out_buffer, str.c_str(), str.size());	
	
	out_buffer[len - 1] = '\0';
	return len;
}

//-----------------------------------------------------------------------------
 size_t HOST_CONTRACT_CALLTYPE get_runtime_property(const char* key,char* value_buffer,size_t value_buffer_size,void* contract_context)
//-----------------------------------------------------------------------------
{
	pal_info("get_runtime_property %s", key);

	if (strcmp(key, HOST_PROPERTY_ENTRY_ASSEMBLY_NAME) == 0)
	{
		return pal_utf8string(string("Hello.dll"), value_buffer, value_buffer_size);
	}

	if (strcmp(key, HOST_PROPERTY_NATIVE_DLL_SEARCH_DIRECTORIES) == 0)
	{
		return pal_utf8string(pal_get_app_dir_path(), value_buffer, value_buffer_size);
	}

	return 0;
}

host_runtime_contract volatile host_contract = { sizeof(host_runtime_contract),(void*) &get_runtime_property, &get_runtime_property };


//-----------------------------------------------------------------------------
void pal_get_paths(PalPaths* paths, int major_rt_version)
//-----------------------------------------------------------------------------
{
	paths->AppDirPath = pal_get_app_dir_path();
	paths->MaxRuntimeVersion = pal_get_max_rt_version(string(RUNTIME_DIR_BASE_PATH), major_rt_version);
	paths->RuntimeIdentifier = string(RUNTIME_IDENTIFIER);

	paths->RuntimeDirPath = string(RUNTIME_DIR_BASE_PATH);
	paths->RuntimeDirPath.append(paths->MaxRuntimeVersion);

	paths->AspNetDirPath = string(ASPNET_DIR_BASE_PATH);
	paths->AspNetDirPath.append(paths->MaxRuntimeVersion);

	paths->CoreCrlFileName = string(CORECLR_FILE_NAME);
	paths->CoreCrlFileFullPath = paths->RuntimeDirPath;
	paths->CoreCrlFileFullPath.append(DIR_SEPARATOR);
	paths->CoreCrlFileFullPath.append(CORECLR_FILE_NAME);

	paths->ProbePaths.clear();
	
	// Probe Paths
	pal_get_probe_paths(&paths->ProbePaths, paths->AppDirPath);
	paths->ProbePaths.push_back(paths->RuntimeDirPath);
	paths->ProbePaths.push_back(paths->AspNetDirPath);
	paths->ProbePaths.push_back(paths->AppDirPath);

	// Probe List
	for (unsigned int i = 0; i < paths->ProbePaths.size(); i++)
	{		
		paths->ProbeList.append(paths->ProbePaths[i]);
		if (paths->ProbePaths[i].back() != DIR_SEPARATOR[0])
			paths->ProbeList.append(DIR_SEPARATOR);
		
		paths->ProbeList.append(TPA_LIST_SEPARATOR);
	}

	paths->TpaFiles.clear();
	
	// Tpa Files
	for (unsigned int i = 0; i < paths->ProbePaths.size(); i++)
	{		
		pal_get_tpa_list(&paths->TpaFiles, paths->ProbePaths[i].c_str());
	}

	// Tpa List
	for (unsigned int i = 0; i < paths->TpaFiles.size(); i++)
	{
		paths->TpaList.append(paths->TpaFiles[i]);
		paths->TpaList.append(TPA_LIST_SEPARATOR);
	}

	// Runtime Contract
	char buffer[40];
	snprintf(buffer, 40, "0x%zx", (size_t)(&host_contract));
	paths->RuntimeContract = string(buffer);
}

//-----------------------------------------------------------------------------
void pal_get_pointers(PalPointers* pointers, const char* corecrl_file_name)
//-----------------------------------------------------------------------------
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

	pointers->PtrSetErrorWriter = (coreclr_set_error_writer_ptr)pal_get_export(pointers->PtrCoreCrl, "coreclr_set_error_writer");
	assert(pointers->PtrSetErrorWriter != nullptr);
}

//-----------------------------------------------------------------------------
void pal_load_assembly(PalAssembly* assembly)
//-----------------------------------------------------------------------------
{
	pal_load_resource(ASM_ID, assembly);
}
