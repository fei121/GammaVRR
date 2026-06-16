#ifndef _HV7607B_API_GAMMAVRR_H_
#define _HV7607B_API_GAMMAVRR_H_
#include "HisenseTool.h"
#ifdef _WIN32
#ifndef EXPORT_DLL
#define EXPORT_API extern "C" __declspec(dllimport)
#else
#define EXPORT_API extern "C" __declspec(dllexport)
#endif // !EXPORT_DLL
#endif

#ifdef __linux__
#ifndef EXPORT_DLL
#define EXPORT_API extern "C" __attribute__((visibility("default")))
#endif // !EXPORT_DLL
#endif

#ifndef EXPORT_API
#define EXPORT_API
#endif

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
	EXPORT_API bool HV7607B_DllTestGammaVrr();
	EXPORT_API bool HV7607B_LoadAlgParaGammaVrr(char* cfg_path, bool show_cfg_en, int* w, int* h);
	EXPORT_API bool HV7607B_RunGammaVrr(char InLUTName[3][1024], PicStruct* imageIn, PicStruct* imageOut, char* OutPutFolderPath);
#ifdef __cplusplus
}
#endif // _cplusplus

#endif // !_HV7607B_API_GAMMAVRR_H_
