/* file: nd_common.h
 * define common functon module
 * author : neil
 * version 1.0
 * 2007-9-27 16:02
 * all right reserved 2007
 */

#ifndef _ND_COMMON_H_
#define _ND_COMMON_H_

#include "nd_common/nd_comcfg.h"

#include "nd_common/nd_define.h"

#include "nd_common/ndchar.h"


#include "nd_common/nd_comdef.h"

#include "nd_common/nd_os.h"

#include "nd_common/nd_dbg.h"

#include "nd_common/nd_handle.h"

#include "nd_common/nd_mempool.h"

#include "nd_common/nd_timer.h"

#include "nd_common/nd_recbuf.h"

#include "nd_common/list.h"

#include "nd_common/nd_bintree.h"

#include "nd_common/nd_str.h"

#include "nd_common/nd_xml.h"

#include "nd_common/nd_atomic.h"

#include "nd_common/nd_static_alloc.h"

#include "nd_common/nd_node_mgr.h"

#include "nd_common/nddir.h"
#include "nd_common/nd_cmdline.h"

ND_COMMON_API ndtime_t		nd_time(void) ;
ND_COMMON_API ndbigtime_t	nd_bigtime(void);
ND_COMMON_API int nd_common_init();
ND_COMMON_API void nd_common_release();
ND_COMMON_API int nd_common_isinit() ;

ND_COMMON_API const char *nd_process_name() ;
//ND_COMMON_API char *process_name();
ND_COMMON_API int nd_arg(int argc, const char *argv[]);

#ifdef ND_FILE_TRACE
	#undef  fopen
	#undef  fclose
	#define fopen(filename, mod) nd_fopen_dbg(filename, mod,__FILE__,__LINE__)
	#define fclose(fp)			nd_fclose_dbg(fp)
#endif

#endif
