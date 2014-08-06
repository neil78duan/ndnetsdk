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
// ��Ҫ�� int nd_net_ioctl(nd_netui_handle  socket_node, int cmd, void *val, int *size) ʹ��
// ÿ��������� val��size���岻һ��, һ���÷�����
/*
	NDUINT32 val = 1024*1024;
	int size = sizeof(val) ;
	m_conns.Ioctl(NDIOCTL_SET_SENDVBUF,&val, &size) ;
 */
enum ND_IOCTRL_CMD
{
	NDIOCTL_SET_BLOCK = 1 ,		//���÷�����/���� 
	NDIOCTL_GET_BLOCK  ,		//�õ�������/����״̬
	NDIOCTL_SET_SENDVBUF,		// ���ͻ���
	NDIOCTL_GET_SENDBUF,		//
	NDIOCTL_SET_RECVVBUF,		//���ջ���
	NDIOCTL_GET_RECVBUF,
	NDIOCTL_SET_TCP_RECV_WNDSIZE,		//TCP����
	NDIOCTL_GET_TCP_RECV_WNDSIZE,
	NDIOCTL_SET_TCP_SEND_WNDSIZE,
	NDIOCTL_GET_TCP_SEND_WNDSIZE,
	NDIOCTL_SET_TIMEOUT,				//���ó�ʱ(ms)
	NDIOCTL_GET_TIMEOUT,
	NDIOCTL_GET_RECV_PACK_NUM,			//��Ϣ����
	NDIOCTL_SET_RECV_PACK_NUM,
	NDIOCTL_GET_SEND_PACK_NUM,
	NDIOCTL_SET_SEND_PACK_NUM,
    NDIOCTL_GET_LAST_RECV_TIME,
	NDIOCTL_NUMBER						//�������
};


#endif
 