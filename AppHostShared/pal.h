#ifndef __PAL_H__
#define __PAL_H__

//
// Include
//

#include <stdio.h>
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
	std::string AppFileName;
	std::string AppFileFullPath;
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
};


//
// Public functions
//
void pal_get_paths(PalPaths* path, int dotnet_version, const char* app_file_name);
void pal_get_pointers(PalPointers* pointers, const char* corecrl_file_name);

#endif