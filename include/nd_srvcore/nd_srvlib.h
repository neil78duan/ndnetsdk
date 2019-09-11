/* file nd_srvlib.h
 * nd engine server thread 
 * version 1.0 all right reserved by neil
 * 2007-10
 */

#ifndef _ND_SRVLIB_H_
#define _ND_SRVLIB_H_

#include "nd_common/nd_common.h"
#include "nd_net/nd_netlib.h"

#include "nd_srvcore/nd_thread.h"
#include "nd_srvcore/nd_thpool.h"
#include "nd_srvcore/nd_session.h"
#include "nd_srvcore/nd_srvobj.h"
#include "nd_srvcore/nd_threadmsgid.h"

ND_SRV_API int nd_srvcore_init(void);
ND_SRV_API void nd_srvcore_destroy(void);
#if !defined(ND_UNIX) 
#endif
#endif
