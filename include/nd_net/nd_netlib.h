/* file : nd_netlib.h
 * version 1.0 
 * define nd_engine net module
 * 
 * neil
 * 2007-10
 * all right reserved 
 */

#ifndef _ND_NETLIB_H_
#define _ND_NETLIB_H_

#if  defined(ND_COMPILE_AS_DLL) && (defined(WIN32) || defined(_WINDOWS) || defined(WIN64))
	#if  defined(ND_NET_EXPORTS) 
	# define ND_NET_API 				CPPAPI __declspec(dllexport)
	#else
	# define ND_NET_API 				CPPAPI __declspec(dllimport)
	#endif
#else 
	# define ND_NET_API 				CPPAPI
#endif 

#include "nd_common/nd_common.h"

#include "nd_net/nd_sock.h"
#include "nd_net/nd_tcp.h"
#include "nd_net/nd_srv.h"
#include "nd_net/nd_netui.h"
#include "nd_net/nd_netcrypt.h"
#include "nd_net/nd_iphdr.h"
#include "nd_net/nd_msgentry.h"
#include "nd_net/nd_netpack.h"
#include "nd_net/nd_udp.h"
#include "nd_net/nd_udt.h"

#include "nd_common/nd_alloc.h"

#include "nd_crypt/nd_pubkey.h"

ND_NET_API int nd_net_init(void);

ND_NET_API void nd_net_destroy(void);
#endif
