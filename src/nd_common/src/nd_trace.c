/* file : nd_trace.c
 * log source usage such as malloc / new / fopen etc..
 * version 1.0
 * neil
 * 2005-11-16
 */

#include "nd_common/list.h"
#include "nd_common/nd_os.h"
#include "nd_common/_source_trace.h"
#include "nd_common/nd_handle.h"
#include "nd_common/nd_mempool.h"

#include <stdio.h>

#ifdef _ND_MEMORY_H_
 //use malloc in libc
#error do not include nd_alloc.h
#endif

#ifdef ND_SOURCE_TRACE

#ifdef ND_MULTI_THREADED
 //mutil-thread
static nd_mutex __S_source_lock;
static int __s_source_init = 0;
nd_handle __s_srclog_pool;

#define __LOCK() do{ \
	nd_assert(__s_source_init) ;\
	nd_mutex_lock(&__S_source_lock) ; \
}while(0)
#define __UNLOCK() nd_mutex_unlock(&__S_source_lock) 
#else 
#define __LOCK() (void)0
#define __UNLOCK() (void)0
#endif

static ND_LIST_HEAD(__s_source_head);
static int _source_numbers = 0;
#define SOURCE_DUMP nd_logfatal
//ndfprintf(stderr,msg) 
struct _Source_loginfo {
	void *__source;
	int __line;			//line munber in file
	char __file[256];
	char __operate[32];
	char __msg[128];
	struct list_head __list;
};

int _source_log(void *p, const char *operate, const char *msg, const char *file, int line)
{
	if (__s_source_init == 0) {
		nd_sourcelog_init();
	}
	if (!p) {
		return -1;
	}
	else {
		struct _Source_loginfo *node;
		node = (struct _Source_loginfo *)
			nd_pool_alloc_real(__s_srclog_pool, sizeof(struct _Source_loginfo));
		//malloc(sizeof(struct _Source_loginfo ) ) ;
		if (!node) {
			return -1;
		}
		node->__source = p;
		node->__line = line;
		//node->__file = file ;
		//node->__operate = operate;
		//node->__msg = msg ;
		if (operate)
			ndstrncpy(node->__operate, operate, 32);
		if (msg)
			ndstrncpy(node->__msg, msg, 128);
		if (file)
			ndstrncpy(node->__file, file, 256);

		__LOCK();
		INIT_LIST_HEAD(&(node->__list));
		list_add(&(node->__list), &__s_source_head);
		++_source_numbers;
		__UNLOCK();
		return 0;
	}
}
static void _destroy_source_node(struct _Source_loginfo *node)
{
	nd_assert(node);
	list_del_init(&(node->__list));
	--_source_numbers;
	//free(node) ;	
	nd_pool_free_real(__s_srclog_pool, node);
}

static void _dump_source(struct _Source_loginfo *node)
{
	//char buf[1024] = {0};
	nd_assert(node);
	SOURCE_DUMP("ERROR %s operate [%s] in file %s line %d\n",
		node->__msg, node->__operate, node->__file, node->__line);

	//SOURCE_DUMP(buf) ;
	list_del(&(node->__list));
	//free(node) ;
	nd_pool_free_real(__s_srclog_pool, node);
}
int _source_release(void *source)
{
	int ret = -1;
	if (!source) {
		nd_assert(source);
		return  -1;
	}
	else {
		struct list_head *pos;
		struct _Source_loginfo *node = NULL;
		__LOCK();
		pos = __s_source_head.next;
		list_for_each(pos, &__s_source_head) {
			node = list_entry(pos, struct _Source_loginfo, __list);
			if (source == node->__source) {
				_destroy_source_node(node);
				ret = 0;
				break;
			}
		}
		__UNLOCK();
	}
	return ret;
}
int nd_sourcelog_init()
{

	if (__s_source_init == 0) {
		__s_srclog_pool = nd_pool_create((size_t)-1, "source_log");
		if (!__s_srclog_pool) {
			nd_logfatal("source log memory pool create failed!\n");
			return -1;
		}

#ifdef ND_MULTI_THREADED
		if (-1 == nd_mutex_init(&__S_source_lock)) {
			nd_logfatal("source log initilized failed!\n");
			nd_pool_destroy(__s_srclog_pool, 0);
			__s_srclog_pool = 0;
			return -1;
		}
#endif
	}
	__s_source_init = 1;
	return 0;
}
void nd_sourcelog_dump()
{
	struct list_head *pos, *next;
	struct _Source_loginfo *node;
	if (__s_source_init == 0) {
		return;
	}

	list_for_each_safe(pos, next, &__s_source_head) {
		node = list_entry(pos, struct _Source_loginfo, __list);
		_dump_source(node);
	}

	nd_pool_destroy(__s_srclog_pool, 0);
	__s_srclog_pool = 0;

#ifdef ND_MULTI_THREADED
	nd_mutex_destroy(&__S_source_lock);
#endif
	__s_source_init = 0;
}

#undef  __LOCK
#undef __UNLOCK

#if defined(ND_FILE_TRACE) 

#ifdef _ND_TRACE_H_
#error not include nd_common.h
#endif
FILE *nd_fopen_dbg(const char *filename, const char *mode, const char *file, int line)
{
	FILE *fp = fopen(filename, mode);
	_source_log(fp, "fopen ", "file not closed", file, line);
	return fp;
}
void nd_fclose_dbg(FILE *fp)
{
	nd_assert(fp);
	fclose(fp);
	_source_release(fp);
}

#endif

#endif		//ND_DEBUG

