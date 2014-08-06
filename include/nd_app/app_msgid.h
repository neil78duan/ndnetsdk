/* file app_msgid.h
 * define applitions message id 
 *
 * neil duan 
 * 2008-6
 */

#ifndef _APP_MSGID_H_
#define _APP_MSGID_H_

#include "nd_common/nd_common.h"
#include "nd_net/nd_netlib.h"


#define MSG_CLASS_NUM		16		//��16����Ϣ
#define MAXID_BASE			1		//��Ϣ��ʼ���

//��������Ϣ��
enum eMsgMaxid{
	MAXID_START_SESSION = MAXID_BASE		//��ʼ�Ự����Ϣ(��Կ����,��½,��ȡ��������Ϣ...)

	,MAXID_SYS = (MSG_CLASS_NUM + MAXID_BASE-1)		//ϵͳ��Ϣ
};

/*�ֱ������Ϣ��*/

//���忪ʼ�Ự����Ϣ�Ĵ���Ϣ��
enum eStartSessionMinid{
	SSM_GETVERSION = 0,				//�õ��汾��Ϣ
	SSM_GETPKI_DIGEST,				//�õ�������Կ��ժҪ
	SSM_GETPKI_KEY,					//�õ�������Կ
	SSM_EXCH_CRYPTKEY,				//��Կ����
	SSM_PKI_ARITHMETIC,				//�õ�������Կ�����㷨
	SSM_SYMMCRYPT_ARITHMETIC,		//�õ��ԳƼ����㷨
	SSM_LOGIN_GATE,					//�õ���½��ڵ�ַ(IP:port)
	SSM_LOGIN_IN,					//��¼(�����û���������)
	SSM_LOGOUT,						//�ǳ�				
	SSM_MSG_NUM
};


//define system message min id
enum eSysMsgid {
	SYM_ECHO = 0 					//ECHO��Ϣ
	,SYM_TEST						//����
	,SYM_BROADCAST					//���͹㲥
} ;

#endif
