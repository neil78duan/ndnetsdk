/* file nd_handle.h
 * define object handle of nd engine 
 * 
 * all right reserved by neil duan 
 * 2008-12-25 
 * version 1.0
 */

#ifndef _ND_HANDLE_H_
#define _ND_HANDLE_H_

#include "nd_comcfg.h"
#include "list.h"
#include "nd_os.h"
#include "nd_bintree.h"
#include "_source_trace.h"

 //predefine handle type 
enum END_OBJECT_TYPE
{
	NDHANDLE_UNKNOW = 0,
	NDHANDLE_MMPOOL,
	NDHANDLE_TCPNODE,
	NDHANDLE_UDPNODE,
	NDHANDLE_LISTEN,
	NDHANDLE_CMALLOCATOR,
	NDHANDLE_STATICALLOCATOR,
	NDHANDLE_NETMSG,
	NDHANDLE_TIMER,
	NDHANDLE_SUB_ALLOCATOR,
	NDHANDLE_USER1,
	NDHANDLE_NUMBERS
};


/*
 * if you need implemention yourself objec-handle you need to define macro : ND_IMPLEMENT_HANDLE
 */

typedef int(*nd_close_callback)(void* handle, int flag) ;			//handle close call back
typedef void (*nd_init_func)(void*)  ;								//handle initilize function

//close flag
enum eObjectClose{
	COMMON_CLOSE,
	FORCE_CLOSE
};

#ifdef ND_SOURCE_TRACE

#define ND_OBJ_BASE \
	size_t size ;	\
	NDUINT16 type ;	\
	NDUINT16 myerrno;	\
	nd_close_callback close_entry ;\
	ndatomic_t __created ;\
	char inst_name[32];				\
	NDUINT32 __objid ;				\
	struct nd_rb_node  __self_rbnode ; \
	struct list_head __release_cb_hdr 
#else 

#define ND_OBJ_BASE \
	size_t size ;	\
	NDUINT16 type ;	\
	NDUINT16 myerrno;	\
	nd_close_callback close_entry ;\
	ndatomic_t __created  ;\
	char inst_name[32];		\
	struct list_head __release_cb_hdr 

#endif

struct tag_nd_handle
{
	ND_OBJ_BASE ;
	/*
	unsigned int size ;						//object size 
	unsigned int type  ;					//object type 	
	NDUINT32	myerrno;
	nd_close_callback close_entry ;			// close/release callback
	*/
} ;

#ifndef ND_IMPLEMENT_HANDLE
	/* a common handler , you can see it was a handle base class*/
	typedef struct tag_nd_handle *nd_handle ;
#else 
#endif

/*
 * create object , before create call ,you need register create function by nd_object_register
 */
ND_COMMON_API nd_handle _object_create(const char *name) ;

/* 
 * if the object is create by nd_object_create , you needn't free the memory .
 * force = 0 common release 
 * force = 1 release whatever 
 */
ND_COMMON_API int _object_destroy(nd_handle handle, int force) ;

#define OBJECT_NAME_SIZE		40
/* register info */
struct nd_handle_reginfo
{
	unsigned int object_size ;		//
	nd_close_callback close_entry ;	//
	nd_init_func	init_entry ;	//
	char name[OBJECT_NAME_SIZE] ;	//object class name 
};

/* register a object class 
 */
ND_COMMON_API int nd_object_register(struct nd_handle_reginfo *reginfo) ;

CPPAPI int destroy_object_register_manager(void) ;

static __INLINE__ int nd_object_set_instname(nd_handle handle, const char *name)
{
	struct tag_nd_handle *ptag = (struct tag_nd_handle *)handle;
	if (name && name[0]){
		ndstrncpy(ptag->inst_name, name, sizeof(ptag->inst_name));
		return (int)ndstrlen(ptag->inst_name);
	}
	return 0;
}
static __INLINE__ char* nd_object_get_instname(nd_handle handle)
{
	return ((struct tag_nd_handle *)handle)->inst_name;
}

#ifdef ND_SOURCE_TRACE

static __INLINE__ nd_handle  object_create(const char *name,const char *file, int line)
{	
	nd_handle p = _object_create(name )  ;
	if(p) {
		nd_object_set_instname(p,name) ;
		_source_log((void*)p,"nd_object_create","object not release!", file,line) ;
	}
	return p ;
}
static __INLINE__ int object_destroy(nd_handle handle, int force) 
{
	if(handle){
		_source_release((void*)handle) ;
		//nd_assert(0==ret) ;
		return _object_destroy(handle, force) ;
	}
	return -1 ;
}
#define nd_object_create(name)		object_create(name,(char*)__FILE__, __LINE__) 
#define nd_object_destroy(h,flag)	object_destroy(h, flag)


#else 
#define nd_object_create(name)		_object_create(name)
#define nd_object_destroy(h,flag)	_object_destroy(h, flag)

#endif

static __INLINE__ NDUINT32 nd_object_lasterror(nd_handle h)
{
	return ((struct tag_nd_handle*)h)->myerrno ;
}

static __INLINE__ void nd_object_seterror(nd_handle h, NDUINT32 errcode)
{
	((struct tag_nd_handle*)h)->myerrno =(NDUINT16) errcode;
}

ND_COMMON_API int nd_object_get_type(nd_handle h);
ND_COMMON_API const char *nd_object_errordesc(nd_handle h) ;
ND_COMMON_API int nd_object_check_error(nd_handle h) ;//
ND_COMMON_API int nd_tryto_clear_err(nd_handle h) ;

//register a object that already created by other way 
ND_COMMON_API int nd_reg_handle(nd_handle hobj) ;
ND_COMMON_API int nd_unreg_handle(nd_handle hobj) ;
ND_COMMON_API int nd_handle_checkvalid(nd_handle hobj, NDUINT16 objtype);

////////////////
// user resource manager 
typedef void (*nd_object_destroy_callback)(nd_handle handle, void *param) ;

struct release_callback_source_node
{
	struct list_head list ;
	int type ;
	nd_object_destroy_callback func ;
	void *param ;
};
//add function call when object destroyed
ND_COMMON_API int nd_object_add_destroy_cb(nd_handle handle,nd_object_destroy_callback callback, void *param,int type) ; //type default 0 , if the handle is connector or session ,when type ==1 ,it will call on close , type ==0 will call on destroy
ND_COMMON_API int nd_object_del_destroy_cb(nd_handle handle,nd_object_destroy_callback callback, void *param) ;
ND_COMMON_API int _nd_object_on_destroy(nd_handle handle,int type) ;

#endif
