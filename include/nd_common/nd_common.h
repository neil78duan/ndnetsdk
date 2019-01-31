/* file: nd_common.h
 * define common functon module
 * author : neil
 * version 1.0
 * 2007-9-27 16:02
 * all right reserved 2007
 */

#ifndef _ND_COMMON_H_
#define _ND_COMMON_H_

#include "nd_common/nd_export_def.h"

#include "nd_common/nd_comcfg.h"

#include "nd_common/nd_define.h"

#include "nd_common/nd_os.h"

#include "nd_common/ndstdstring.h"

#include "nd_common/nd_logger.h"

#include "nd_common/_source_trace.h"

#include "nd_common/nd_trace.h"

#include "nd_common/nd_time.h"

#include "nd_common/nd_handle.h"

#include "nd_common/nd_mempool.h"

//#include "nd_common/nd_timer.h"

#include "nd_common/nd_recbuf.h"

#include "nd_common/list.h"

#include "nd_common/nd_bintree.h"

#include "nd_common/nd_str.h"

#include "nd_common/nd_xml.h"

#include "nd_common/nd_atomic.h"

//#include "nd_common/nd_static_alloc.h"

//#include "nd_common/nd_node_mgr.h"

#include "nd_common/nddir.h"

//#include "nd_common/nd_cmdline.h"

//#include "nd_common/nd_iconv.h"

ND_COMMON_API int nd_common_init(void);
ND_COMMON_API void nd_common_release_ex(int force);
ND_COMMON_API int nd_common_isinit(void);
#define nd_common_release()		nd_common_release_ex(0) 

ND_COMMON_API const char * nd_common_machine_info(char buf[], size_t size);

ND_COMMON_API const char *nd_process_name(void) ;
//ND_COMMON_API char *process_name();
ND_COMMON_API int nd_arg(int argc, const char *argv[]);


typedef const char* (*nd_error_convert)(int errcode);

static int __INLINE__ nd_error_max_sys_number()
{
	return NDERR_SYS_MAX_NUMBER;
}


ND_COMMON_API const char *nd_error_desc(int errcode);
ND_COMMON_API int nd_error_get_user_number(void);
ND_COMMON_API void nd_error_set_user_number(int max_user_number);
ND_COMMON_API nd_error_convert nd_register_error_convert(nd_error_convert func);


#endif
