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

#ifndef __INLINE__
#ifdef __cplusplus
#define __INLINE__			inline	
#else 
#define __INLINE__			__inline	
#endif 
#endif

#ifndef CPPAPI
#ifdef __cplusplus
#define CPPAPI extern "C" 
#else 
#define CPPAPI 
#endif 
#endif

#if  defined(_MSC_VER )
#define DLL_EXPORT __declspec(dllexport)
#define DLL_IMPORT __declspec(dllimport)

#pragma warning (disable:  4018)	
#pragma warning (disable:  4251)	
#pragma warning(disable:4001)	
#pragma warning(disable:4100)	
#pragma warning(disable:4699)	
#pragma warning(disable:4710)	
#pragma warning(disable:4514)	
#pragma warning(disable:4512)
#pragma  warning(disable: 4996)
#pragma  warning(disable: 4819)
#pragma  warning(disable: 4828)

#else 
#define DLL_EXPORT 
#define DLL_IMPORT 
#endif 

#if  defined(ND_COMPILE_AS_DLL)

#if defined(ND_COMMON_EXPORTS)
# define ND_COMMON_API 				CPPAPI DLL_EXPORT
# define ND_CRYPT_API 				CPPAPI DLL_EXPORT
# define ND_COMMON_CLASS			DLL_EXPORT

#elif defined(ND_APPLIB_EXPORTS) || defined(ND_CLI_EXPORTS)

# define ND_CONNCLI_API 			CPPAPI DLL_EXPORT
# define ND_COMMON_API 				CPPAPI DLL_EXPORT
# define ND_CRYPT_API 				CPPAPI DLL_EXPORT
# define ND_APPLIB_API				CPPAPI DLL_EXPORT
# define ND_NET_API 				CPPAPI DLL_EXPORT
# define ND_SRV_API 				CPPAPI DLL_EXPORT
# define ND_VM_API 					CPPAPI DLL_EXPORT

# define ND_CONNCLI_CLASS 			DLL_EXPORT
# define ND_COMMON_CLASS			DLL_EXPORT
# define ND_SRV_CLASS	 			DLL_EXPORT

#else
# define ND_CONNCLI_API 			CPPAPI DLL_IMPORT
# define ND_COMMON_API 				CPPAPI DLL_IMPORT
# define ND_CRYPT_API 				CPPAPI DLL_IMPORT
# define ND_APPLIB_API				CPPAPI DLL_IMPORT
# define ND_NET_API 				CPPAPI DLL_IMPORT
# define ND_SRV_API 				CPPAPI DLL_IMPORT
# define ND_VM_API 					CPPAPI DLL_IMPORT

# define ND_CONNCLI_CLASS 			DLL_IMPORT
# define ND_COMMON_CLASS			DLL_IMPORT
# define ND_SRV_CLASS	 			DLL_IMPORT

#endif


#else

# define ND_COMMON_API 				CPPAPI 
# define ND_CONNCLI_API 			CPPAPI
# define ND_SRV_API 				CPPAPI
# define ND_CRYPT_API 				CPPAPI
# define ND_NET_API 				CPPAPI
# define ND_VM_API					CPPAPI
# define ND_APPLIB_API				CPPAPI

#define ND_CONNCLI_CLASS 
#define ND_COMMON_CLASS
#define ND_SRV_CLASS 

#endif


#endif
