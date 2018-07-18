/* file nd_comcfg.h
 *
 * config nd common
 *
 * 2009-12-6 22:15
 */

#ifndef _ND_COMCFG_H_
#define _ND_COMCFG_H_

#define ND_VERSION "3.0"

#define E_SRC_CODE_ANSI  	0
#define E_SRC_CODE_GBK		1
#define E_SRC_CODE_UTF_8	2

#define E_SRC_CODE_UTF8		E_SRC_CODE_UTF_8

#define ND_ENCODE_UTF8		E_SRC_CODE_UTF_8
#define ND_ENCODE_GBK		E_SRC_CODE_GBK
#define ND_ENCODE_ANSI		E_SRC_CODE_ANSI

//source code compiled encode
//define platform
/*
 __ND_MAC__		//MAC_OS
 __ND_LINUX__	//linux
 __ND_IOS__		//iOS
 __ND_ANDROID__ // android 
 __ND_WIN__		//WIN32 WIN64
 __ND_BSD__		//freebsd bsd...

 
 ND_SERVER //build for server
 ND_CLIENT //build for client 
 WITHOUT_ICONV //without iconv
 _GUI_TOOL_ // for gui tool
 ND_UNIX // UNIX-LIKE platform un-windows
 ND_COMPILE_AS_DLL //build as dll
 ND_UNICODE //inter-char as unicode-16
 BUILD_AS_STATIC_LIB //build as static lib
 
*/

#ifdef _MSC_VER
#define ND_ENCODE_TYPE E_SRC_CODE_GBK

#ifndef __ND_WIN__ 
#define __ND_WIN__ 1 
#endif  //_MSC_VER

#else

#define ND_ENCODE_TYPE E_SRC_CODE_UTF_8
#define ND_UNIX 

#endif

#define BUILD_AS_STATIC_LIB 1 
#define ND_BUFSIZE 4096						
#define ND_FILE_PATH_SIZE	1024
#define NOT_SUPPORT_THIS_FUNCTION 0			// old linux version
#define ND_MULTI_THREADED 1				// must be define 
#define ND_MAX_THREAD_NUM 16			//for server

#ifdef ND_CLIENT_ONLY

#include "nd_common/_client_config.h"

#else

#include "nd_common/_server_config.h"

#endif

#endif
