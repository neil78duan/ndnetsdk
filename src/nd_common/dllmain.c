/************************************************************************* 
 * file : dllmain.c
 * description : This file is dll entry. but nothing to do 
 * version : 1.0 
 * author : neil duan 
 * date : 2004-10-31 
 *************************************************************************/

#if defined(WIN32) || defined(WIN64) || defined(_WINDOWS) 
#include <windows.h>
#else
#error This file IS dllMain ONLY CAN BE use in windows
#endif

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}
