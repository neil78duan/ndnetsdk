/* file nd_threadmsgid.h
 *
 * define thread message id of nd 
 *
 * create by duan 
 * 2011/7/29 15:15:00
 */
 
#ifndef _ND_THREADMSG_H_
#define _ND_THREADMSG_H_

//线程通讯消息编号
enum eThreadMsgID 
{
	E_THMSGID_ADDTO_THREAD,		//把session添加到目标线程
	E_THMSGID_NETMSG_HANDLE,	//把消息让目标线程作为网络消息处理
	E_THMSGID_SENDTO_CLIENT ,	//把消息发送给目标session
	E_THMSGID_DELFROM_THREAD,	//把session从线程中删除
	E_THMSGID_CLOSE_SESSION,	//关闭session
	E_THMSGID_DELAY_CLOSE_RAND,	//等待随机的时间后关闭
	E_THMSG_NUMBER
};

#define THMSGID_USER_START 128	//用户消息起始编号
#endif 
