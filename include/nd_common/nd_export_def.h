/* file nd_export_def.h
 *
 * define export function or class for dll on windows 
 *
 * create by duan 
 *
 * 2015-10-22
 */

#ifndef _ND_EXPORT_DEF_H_
#define _ND_EXPORT_DEF_H_

#ifndef CPPAPI
#ifdef __cplusplus
#define CPPAPI extern "C" 
#else 
#define CPPAPI 
#endif 
#endif

#ifdef _MSC_VER

#ifdef CONN_CLI_EXPORTS
#define ND_CONNCLI_API 				CPPAPI  __declspec(dllexport)
#define ND_CONNCLI_CLASS 			__declspec(dllexport)
#else 
#define ND_CONNCLI_API 				CPPAPI __declspec(dllimport)
#define ND_CONNCLI_CLASS 			__declspec(dllimport)
#endif

#else

#define ND_CONNCLI_API 				CPPAPI
#define ND_CONNCLI_CLASS 			
#endif


#endif
