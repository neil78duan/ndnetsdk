/* common.c
 * define nd common initilize /destroy function 
 *
 * version 1.0 
 * all right reserved by neil duan
 * 2007-10
 */

#include "nd_common/nd_common.h"
#include "nd_common/nd_mempool.h"
#include "nd_common/nd_os.h"

#ifdef ND_UNIX
#include <locale.h>
#endif
const char *__g_process_name = NULL;			//process name
static int __s_common_init = 0 ;
static ndtime_t __s_common_init_base_tick = 0;
static time_t	__s_common_init_base_time = 0;

const char *nd_process_name(void)
{
	static char s_proname[128] = {0};

	if(!__g_process_name) {
		return "ndengine" ;
	}
	else if(0==s_proname[0]) {
		size_t len = ndstrlen(__g_process_name) ;
		const char *p= __g_process_name + len;
		char *desc =s_proname;
		while(len-- > 0){
			if(*p==0x2f || *p==0x5c) {
				++p ;
				break ;
			}
			--p ;
		}
		len = 128 ;
		while(*p && len-- > 0) {
			if(*p== '.')
				break ;
			*desc++ = *p++ ;
		}
		*desc = 0 ;
	}
	return s_proname ;
}


int nd_arg(int argc, const char *argv[])
{
	__g_process_name = argv[0] ;
	return 0;
}

ndbigtime_t	nd_bigtime(void)
{
	ndtime_t detla_tick = nd_time() - __s_common_init_base_tick;

	ndbigtime_t ret = (ndbigtime_t)__s_common_init_base_time * 1000 + (ndbigtime_t)detla_tick;

	return ret;
}

int nd_common_init(void)
{
	if (__s_common_init){
		return 0 ;
	}
#ifdef ND_DEBUG
	nd_setxml_log(nd_output) ;
#else
	nd_setxml_log(nd_default_filelog) ;
#endif

	//nd_logmsg("%s common init\n" AND nd_process_name() ) ;
#ifdef ND_UNIX
	setlocale(LC_CTYPE, "zh-CN.UTF-8");  //支持中文
#else 
	
#endif
	//nd_memory_init() ;
	//if(-1==nd_mempool_root_init() )  {
	//	return -1 ;
	//}
	nd_sourcelog_init() ;

	__s_common_init = 1 ;
	__s_common_init_base_tick = nd_time();
	__s_common_init_base_time = time(NULL);

	return 0 ;
}

void nd_common_release_ex(int force)
{
	destroy_object_register_manager() ;
	nd_sourcelog_dump();

	if (force) {
		nd_mempool_root_release();
	}
	//nd_memory_destroy() ;
	__s_common_init = 0;
}
int nd_common_isinit(void) 
{
	return __s_common_init  ;
}
const char * nd_common_machine_info(char buf[], size_t size)
{
	int len = 0;
	char *p = buf;

#if defined( __ND_WIN__)
	len = ndsnprintf(buf, size, "MACH-%s", "win");
#elif defined( __ND_MAC__)
	len = ndsnprintf(buf, size, "MACH-%s", "mac-");
#elif defined( __ND_IOS__)
	len = ndsnprintf(buf, size, "MACH-%s", "ios-");
#elif defined( __ND_ANDROID__)
	len = ndsnprintf(buf, size, "MACH-%s", "android-");
#elif defined( __ND_LINUX__)
	len = ndsnprintf(buf, size, "MACH-%s", "linux-");
#else 
	len = ndsnprintf(buf, size, "MACH-%s", "unknown-");
#endif 
	p += len;
	size -= len;
#if defined(X86_64)
	len = ndsnprintf(p, size, "-%s", "x86-64");
#else 
	len = ndsnprintf(p, size, "-%s", "32");
#endif 
	p += len;
	size -= len;

#ifdef ND_DEBUG
	len = ndsnprintf(p, size, "-%s", "debug");
#else 
	len = ndsnprintf(p, size, "-%s", "release");
#endif
	return buf;
}

// error function defined
static nd_error_convert __error_convert;
nd_error_convert nd_register_error_convert(nd_error_convert func)
{
	nd_error_convert ret = __error_convert;
	__error_convert = func;
	return ret;
}

const char *nd_error_desc(int in_err)
{
	static __ndthread char errdesc[128];

	NDUINT32 errcode = in_err;

	if (__error_convert) {
		return __error_convert(errcode);
	}
	else {
		char *perr[] = {
			"NDERR_SUCCESS ",

#undef ErrorElement 
#define ErrorElement(a,_err_desc) "system(ND"#a "):" _err_desc
#include "nd_common/_nderr.h"		
#undef ErrorElement 
		};

		if (errcode <= NDERR_UNKNOWN) {
			return perr[errcode];
		}
		else {
			ndsnprintf(errdesc, sizeof(errdesc), "Error code =%d", errcode);
			return errdesc;
		}
	}

}


static int _S_max_user_define_error_id = NDERR_USERDEFINE;
int nd_error_get_user_number(void)
{
	return _S_max_user_define_error_id;
}
void nd_error_set_user_number(int max_user_number)
{
	_S_max_user_define_error_id = max_user_number;
}
