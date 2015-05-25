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
#define ErrorElement(a) a
	NDERR_SUCCESS  = 0 ,	//正确
#include "_nderr.h"
	/*
	NDERR_INVALID_HANDLE,	//无效句柄
	NDERR_TIMEOUT   ,		//超时
	NDERR_NOSOURCE ,		//没有足够资源
	NDERR_OPENFILE,			//不能打开文件
	NDERR_BADTHREAD,		//不能打开线程
	NDERR_LIMITED,			//资源超过上限
	NDERR_USER,				//处理用户数据出错(消息回调函数返回-1
	NDERR_INVALID_INPUT ,	//无效的输入(DATA IS TO BIG OR ZERO
	NDERR_IO		,		//IO bad SYSTEM IO BAD
	NDERR_WUOLD_BLOCK ,		//需要阻塞	
	NDERR_CLOSED,			//socket closed by peer
	NDERR_BADPACKET  ,		//网络输入数据错误(too long or short)
	NDERR_BADSOCKET ,		//无效的socket
	NDERR_READ,				//read error
	NDERR_WRITE,			//write error	
	NDERR_NO_PRIVILAGE,		//没有权限
	NDERR_RESET,			//被重置
	NDERR_USER_BREAK,		//用户中断(退出循环)
	NDERR_VERSION,			//版本号错误
	NDERR_UNKNOW			//unknowwing error
	 */
	
#undef ErrorElement 
};


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
