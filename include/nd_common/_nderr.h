//
//  _nderr.h
//  Define error-code number
//
//  Created by duanxiuyun on 14-12-19.
//  Copyright (c) 2014 duanxiuyun. All rights reserved.
//
#ifdef _MSC_VER
#pragma  warning(disable: 4819)
#endif

//

ErrorElement(NDERR_INVALID_HANDLE),	//无效句柄
ErrorElement(NDERR_TIMEOUT)   ,		//超时
ErrorElement(NDERR_NOSOURCE) ,		//没有足够资源
ErrorElement(NDERR_OPENFILE),			//不能打开文件
ErrorElement(NDERR_BADTHREAD),		//线程错误
ErrorElement(NDERR_LIMITED),			//限制使用
ErrorElement(NDERR_USER),				//没用用户
ErrorElement(NDERR_INVALID_INPUT) ,	//无效的输入(DATA IS TO BIG OR ZERO
ErrorElement(NDERR_IO)		,		//IO bad SYSTEM IO BAD
ErrorElement(NDERR_WUOLD_BLOCK) ,		//需要阻塞	
ErrorElement(NDERR_CLOSED),			//socket closed by peer
ErrorElement(NDERR_BADPACKET)  ,		//封包错误(too long or short)
ErrorElement(NDERR_BADSOCKET) ,		//无效的socket
ErrorElement(NDERR_READ),				//read error
ErrorElement(NDERR_WRITE),			//write error	
ErrorElement(NDERR_NO_PRIVILAGE),		//没有权限
ErrorElement(NDERR_RESET),			//被重置
ErrorElement(NDERR_USER_BREAK),		//用户中断
ErrorElement(NDERR_VERSION),			//版本错误
ErrorElement(NDERR_UNHANDLED_MSG),			//unknow message
ErrorElement(NDERR_HOST_CRASH),			//PROGRAM crash
ErrorElement(NDERR_HOST_SHUTDOWN),			//Shutdown by manual
ErrorElement(NDERR_UNKNOW)			//unknowwing error