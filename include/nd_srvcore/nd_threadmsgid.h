/* file nd_threadmsgid.h
 *
 * define thread message id of nd 
 *
 * create by duan 
 * 2011/7/29 15:15:00
 */
 
#ifndef _ND_THREADMSG_H_
#define _ND_THREADMSG_H_

//net thread message id define
enum eThreadMsgID 
{
	E_THMSGID_ADDTO_THREAD,		//session add to thread 
	E_THMSGID_NETMSG_HANDLE,	//send net message to aim session
	E_THMSGID_SENDTO_CLIENT ,	//send message to client
	E_THMSGID_DELFROM_THREAD,	//delete session from thread
	E_THMSGID_CLOSE_SESSION,	//close session
	E_THMSGID_DELAY_CLOSE_RAND,	// close session after waiting rand() ms
	E_THMSGID_SEND_UDP_DATA,	// send udp data to thread handler
	E_THMSG_NUMBER
};

#define THMSGID_USER_START 128	//user define message id start-index
#endif 
