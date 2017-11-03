/* file nd_threadmsgid.h
 *
 * define thread message id of nd 
 *
 * create by duan 
 * 2011/7/29 15:15:00
 */
 
#ifndef _ND_THREADMSG_H_
#define _ND_THREADMSG_H_

//�߳�ͨѶ��Ϣ���
enum eThreadMsgID 
{
	E_THMSGID_ADDTO_THREAD,		//��session��ӵ�Ŀ���߳�
	E_THMSGID_NETMSG_HANDLE,	//����Ϣ��Ŀ���߳���Ϊ������Ϣ����
	E_THMSGID_SENDTO_CLIENT ,	//����Ϣ���͸�Ŀ��session
	E_THMSGID_DELFROM_THREAD,	//��session���߳���ɾ��
	E_THMSGID_CLOSE_SESSION,	//�ر�session
	E_THMSGID_DELAY_CLOSE_RAND,	//�ȴ������ʱ���ر�
	E_THMSG_NUMBER
};

#define THMSGID_USER_START 128	//�û���Ϣ��ʼ���
#endif 
