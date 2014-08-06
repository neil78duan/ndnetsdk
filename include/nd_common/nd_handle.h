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
 * �����Ҫʵ���Լ���handle����,���ڰ�����ǰ�ļ�ǰ ���� ND_IMPLEMENT_HANDLE
 * Ϊ�˱����Զ������ͺ͵�ǰ�ļ��е����ͳ�ͻ
 */

typedef int(*nd_close_callback)(void* handle, int flag) ;			//����رպ���
typedef void (*nd_init_func)(void*)  ;								//�����ʼ������

//����رշ�ʽ
enum eObjectClose{
	COMMON_CLOSE,
	FORCE_CLOSE
};

#ifdef ND_SOURCE_TRACE

#define ND_OBJ_BASE \
	NDUINT32 size ;	\
	NDUINT16 type ;	\
	NDUINT16 myerrno;	\
	nd_close_callback close_entry ;\
	ndatomic_t __created ;\
	char inst_name[32];				\
	NDUINT32 __objid ;				\
	struct nd_rb_node  __self_rbnode
#else 

#define ND_OBJ_BASE \
	NDUINT32 size ;	\
	NDUINT16 type ;	\
	NDUINT16 myerrno;	\
	nd_close_callback close_entry ;\
	ndatomic_t __created 

#endif

struct tag_nd_handle
{
	ND_OBJ_BASE ;
	/*
	unsigned int size ;						//����Ĵ�С
	unsigned int type  ;					//�������	
	NDUINT32	myerrno;
	nd_close_callback close_entry ;			//����ͷź���
	*/
} ;

#ifndef ND_IMPLEMENT_HANDLE
	/* ����һ��ͨ�õľ������*/
	typedef struct tag_nd_handle *nd_handle ;
#else 
#endif

/* ����һ������ʵ��,��nd_create_object() ����֮ǰע��һ�����ֽ�name�Ķ�������,�Ϳ�����create��������
 * ���ض���ľ��
 * nd_object_create() ʹ��malloc����������һ���ڴ�,���ǲ�û����
 */
ND_COMMON_API nd_handle _object_create(char *name) ;

/* 
 * ����һ�����, �������nd_object_create ���������Ķ���,����Ҫ�Լ��ͷ��ڴ�
 * ������Ҫ�ֶ��ͷ��ڴ�
 * force = 0 �����ͷ�,�ȵ���ռ�õ���Դ�ͷź󷵻�
 * force = 1ǿ���ͷ�,���ȴ� ref eObjectClose
 */
ND_COMMON_API int _object_destroy(nd_handle handle, int force) ;

#define OBJECT_NAME_SIZE		40
/*���ע����Ϣ*/
struct nd_handle_reginfo
{
	unsigned int object_size ;		//�������Ĵ�С
	nd_close_callback close_entry ;	//�رպ���
	nd_init_func	init_entry ;	//��ʼ������
	char name[OBJECT_NAME_SIZE] ;	//��������
};

/* ע��һ����������,������windows�������͵�ע��
 * ע�����֮��Ϳ���ʹ�ô�����������һ�����ڵ�ʵ��
 */
ND_COMMON_API int nd_object_register(struct nd_handle_reginfo *reginfo) ;

int destroy_object_register_manager(void) ;

#ifdef ND_SOURCE_TRACE

static __INLINE__ int nd_object_set_instname(nd_handle handle, char *name)
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

static __INLINE__ nd_handle  object_create(char *name,char *file, int line) 
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
static __INLINE__ int nd_object_set_instname(nd_handle handle, char *name)
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

ND_COMMON_API char *nd_object_errordesc(nd_handle h) ;
ND_COMMON_API int nd_object_check_error(nd_handle h) ;//������Ƿ������Ч
ND_COMMON_API int nd_tryto_clear_err(nd_handle h) ;

//�Ǽ�һ�������õľ��
ND_COMMON_API int nd_reg_handle(nd_handle hobj) ;
ND_COMMON_API int nd_unreg_handle(nd_handle hobj) ;
ND_COMMON_API int nd_handle_checkvalid(nd_handle hobj, NDUINT16 objtype);
#endif
