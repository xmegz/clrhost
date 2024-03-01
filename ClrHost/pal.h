#ifndef __PAL_H__
#define __PAL_H__

#include <stdio.h>
#include <assert.h>
#include <string>
#include <vector>

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
};

void* pal_load_library(const char*);
void* pal_get_export(void*, const char*);

string pal_get_runtime_dir_base_path(void);
string pal_get_app_dir_path(void);
string pal_get_aspnet_dir_base_path(void);
string pal_get_corecrl_file_name(void);
string pal_get_max_runtime_version(string base_dir, int version);

void pal_get_dll_list_for_tpa(vector<string>* result, string dirname);

void pal_get_pal_paths(PalPaths* path, int dotnet_version, const char* fileName);

#endif