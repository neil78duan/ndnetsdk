/* file nd_netioctl.h
 *
 * define ioctl function 
 *
 * create by duan 
 *
 * 2013/1/3 10:30:37
 *
 */

#ifndef _ND_NETIOCTL_H_
#define _ND_NETIOCTL_H_

//command of 
// 主要给 int nd_net_ioctl(nd_netui_handle  socket_node, int cmd, void *val, int *size) 使用
// 每个命令参数 val和size意义不一样, 一般用法如下
/*
	NDUINT32 val = 1024*1024;
	int size = sizeof(val) ;
	m_conns.Ioctl(NDIOCTL_SET_SENDVBUF,&val, &size) ;
 */
enum ND_IOCTRL_CMD
{
	NDIOCTL_SET_BLOCK = 1 ,		//设置非阻塞/阻塞 
	NDIOCTL_GET_BLOCK  ,		//得到非阻塞/阻塞状态
	NDIOCTL_SET_SENDVBUF,		// 发送缓冲
	NDIOCTL_GET_SENDBUF,		//
	NDIOCTL_SET_RECVVBUF,		//接收缓冲
	NDIOCTL_GET_RECVBUF,
	NDIOCTL_SET_TCP_RECV_WNDSIZE,		//TCP缓冲
	NDIOCTL_GET_TCP_RECV_WNDSIZE,
	NDIOCTL_SET_TCP_SEND_WNDSIZE,
	NDIOCTL_GET_TCP_SEND_WNDSIZE,
	NDIOCTL_SET_TIMEOUT,				//设置超时(ms)
	NDIOCTL_GET_TIMEOUT,
	NDIOCTL_GET_RECV_PACK_NUM,			//消息个数
	NDIOCTL_SET_RECV_PACK_NUM,
	NDIOCTL_GET_SEND_PACK_NUM,
	NDIOCTL_SET_SEND_PACK_NUM,
    NDIOCTL_GET_LAST_RECV_TIME,
	NDIOCTL_NUMBER						//命令个数
};


#endif
 