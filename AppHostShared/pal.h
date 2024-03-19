#ifndef __PAL_H__
#define __PAL_H__

//
// Include
//

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string>
#include <vector>

#include "coreclrhost.h"

//
// Structs
//
struct PalPaths
{
	std::string AppDirPath;
	std::string MaxRuntimeVersion;
	std::string RuntimeDirPath;
	std::string AspNetDirPath;
	std::string CoreCrlFileFullPath;
	std::string CoreCrlFileName;
	std::vector<std::string> TpaFiles;
	std::string TpaList;
};

struct PalPointers
{
	void* PtrCoreCrl;
	coreclr_initialize_ptr PtrInitialize;
	coreclr_create_delegate_ptr PtrCreateDelegate;
	coreclr_shutdown_2_ptr PtrShutdown;
	coreclr_set_error_writer_ptr PtrSetErrorWriter;
};

struct PalAssembly
{
	char* Bytes = nullptr;
	int Size = 0;

	~PalAssembly()
	{
		if (Bytes != nullptr) free(Bytes);
		Bytes = nullptr;
		Size = 0;
	}
};

//
// Enums
//
enum pal_error
{
	assert = -1,
	init = -2,
	create_delegate = -3,
	delegate = -4,
	shutdown_crl = -5
};


//
// Public functions
//
void pal_assert(bool condition, const char* function, const char* format, ...);
void pal_info(const char* format, ...);
void pal_error(int code, const char* format, ...);
void pal_trace(const char* message);

void pal_get_paths(PalPaths* path, int major_rt_version);
void pal_get_pointers(PalPointers* pointers, const char* corecrl_file_name);
void pal_load_assembly(PalAssembly* assembly);

#endif