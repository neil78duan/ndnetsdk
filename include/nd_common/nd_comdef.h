/* file nd_comdef.h
 * define nd common symbols
 *
 * 2009-1-13 23:58
 * all right reserved by neil duan
 */
#ifndef _ND_COMDEF_H_
#define _ND_COMDEF_H_

enum END_ERROR_TYPE
{
#undef ErrorElement 
#define ErrorElement(_errId, _err_description) ND##_errId 
	NDERR_SUCCESS  = 0 ,	//正确
#include "_nderr.h"
	NDERR_SYS_MAX_NUMBER 
#undef ErrorElement 
};

static int nd_error_max_sys_number()
{
	return NDERR_SYS_MAX_NUMBER;
}


#define NDERR_USERDEFINE 1024 

//定义句柄类型
enum END_OBJECT_TYPE
{
	NDHANDLE_UNKNOW =0,
	NDHANDLE_MMPOOL , 
	NDHANDLE_TCPNODE , 
	NDHANDLE_UDPNODE , 
	NDHANDLE_LISTEN , 
	NDHANDLE_CMALLOCATOR ,
	NDHANDLE_STATICALLOCATOR,
	NDHANDLE_NETMSG,
	NDHANDLE_TIMER,
	NDHANDLE_SUB_ALLOCATOR,
    NDHANDLE_USER1,
	NDHANDLE_NUMBERS
};

#endif 
