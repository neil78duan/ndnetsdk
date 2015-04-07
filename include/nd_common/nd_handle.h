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

/*
 * 如果需要实现自己的handle类型,请在包含当前文件前 定义 ND_IMPLEMENT_HANDLE
 * 为了避免自定义类型和当前文件中的类型冲突
 */

typedef int(*nd_close_callback)(void* handle, int flag) ;			//对象关闭函数
typedef void (*nd_init_func)(void*)  ;								//对象初始化函数
typedef const char* (*nd_error_convert)(int errcode)  ;

//对象关闭方式
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
	struct list_head __release_cb_hdr 

#endif

struct tag_nd_handle
{
	ND_OBJ_BASE ;
	/*
	unsigned int size ;						//句柄的大小
	unsigned int type  ;					//句柄类型	
	NDUINT32	myerrno;
	nd_close_callback close_entry ;			//句柄释放函数
	*/
} ;

#ifndef ND_IMPLEMENT_HANDLE
	/* 定义一个通用的句柄类型*/
	typedef struct tag_nd_handle *nd_handle ;
#else 
#endif

/* 创建一个对象实例,在nd_create_object() 函数之前注册一个名字叫name的对象类型,就可以用create函数创建
 * 返回对象的句柄
 * nd_object_create() 使用malloc函数申请了一块内存,但是并没有在
 */
ND_COMMON_API nd_handle _object_create(const char *name) ;

/* 
 * 销毁一个句柄, 如果是用nd_object_create 函数创建的对象,则不需要自己释放内存
 * 否则需要手动释放内存
 * force = 0 正常释放,等等所占用的资源释放后返回
 * force = 1强制释放,不等待 ref eObjectClose
 */
ND_COMMON_API int _object_destroy(nd_handle handle, int force) ;

#define OBJECT_NAME_SIZE		40
/*句柄注册信息*/
struct nd_handle_reginfo
{
	unsigned int object_size ;		//句柄对象的大小
	nd_close_callback close_entry ;	//关闭函数
	nd_init_func	init_entry ;	//初始化函数
	char name[OBJECT_NAME_SIZE] ;	//对象名字
};

/* 注册一个对象类型,类似于windows窗口类型的注册
 * 注册完成之后就可以使用创建函数创建一个对于的实例
 */
ND_COMMON_API int nd_object_register(struct nd_handle_reginfo *reginfo) ;

int destroy_object_register_manager(void) ;

#ifdef ND_SOURCE_TRACE

static __INLINE__ int nd_object_set_instname(nd_handle handle, const char *name)
{
	struct tag_nd_handle *ptag = (struct tag_nd_handle *)handle ;
	if (name && name[0]){
		strncpy(ptag->inst_name,name, sizeof(ptag->inst_name)) ;
		return (int)strlen(ptag->inst_name);
	}
	return 0;
}
static __INLINE__ char* nd_object_get_instname(nd_handle handle)
{
	return ((struct tag_nd_handle *)handle )->inst_name ;
}

static __INLINE__ nd_handle  object_create(const char *name,const char *file, int line)
{	
	nd_handle p = _object_create(name )  ;
	if(p) {
		nd_object_set_instname(p,name) ;
		_source_log(p,(char*)"nd_object_create",(char*)"object not release!", file,line) ;
	}
	return p ;
}
static __INLINE__ int object_destroy(nd_handle handle, int force) 
{
	if(handle){
		_source_release(handle) ;
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
static __INLINE__ int nd_object_set_instname(nd_handle handle, const char *name)
{
	return 0;
}
static __INLINE__ int nd_object_get_instname(nd_handle handle)
{
	return 0;
}

#endif

static __INLINE__ NDUINT32 nd_object_lasterror(nd_handle h)
{
	return ((struct tag_nd_handle*)h)->myerrno ;
}

static __INLINE__ void nd_object_seterror(nd_handle h, NDUINT32 errcode)
{
	((struct tag_nd_handle*)h)->myerrno =(NDUINT16) errcode;
}

ND_COMMON_API const char *nd_object_errordesc(nd_handle h) ;
ND_COMMON_API int nd_object_check_error(nd_handle h) ;//检测句柄是否出错无效
ND_COMMON_API int nd_tryto_clear_err(nd_handle h) ;

//登记一个创建好的句柄
ND_COMMON_API int nd_reg_handle(nd_handle hobj) ;
ND_COMMON_API int nd_unreg_handle(nd_handle hobj) ;
ND_COMMON_API int nd_handle_checkvalid(nd_handle hobj, NDUINT16 objtype);

ND_COMMON_API const char *nd_error_desc(int errcode);
ND_COMMON_API nd_error_convert nd_register_error_convert(nd_error_convert func);


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
