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
	NDERR_SUCCESS  = 0 ,	//��ȷ
	NDERR_INVALID_HANDLE,	//��Ч���
	NDERR_TIMEOUT   ,		//��ʱ
	NDERR_NOSOURCE ,		//û���㹻��Դ
	NDERR_OPENFILE,			//���ܴ��ļ�
	NDERR_BADTHREAD,		//���ܴ��߳�
	NDERR_LIMITED,			//��Դ��������
	NDERR_USER,				//�����û����ݳ���(��Ϣ�ص���������-1
	NDERR_INVALID_INPUT ,	//��Ч������(DATA IS TO BIG OR ZERO
	NDERR_IO		,		//IO bad SYSTEM IO BAD
	NDERR_WUOLD_BLOCK ,		//��Ҫ����	
	NDERR_CLOSED,			//socket closed by peer
	NDERR_BADPACKET  ,		//�����������ݴ���(too long or short)
	NDERR_BADSOCKET ,		//��Ч��socket
	NDERR_READ,				//read error
	NDERR_WRITE,			//write error	
	NDERR_NO_PRIVILAGE,		//û��Ȩ��
	NDERR_RESET,			//������
	NDERR_USER_BREAK,		//�û��ж�(�˳�ѭ��)
	NDERR_VERSION,			//�汾�Ŵ���
	NDERR_UNKNOW			//unknowwing error
};

//����������
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
