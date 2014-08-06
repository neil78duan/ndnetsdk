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


#define MSG_CLASS_NUM		16		//有16类消息
#define MAXID_BASE			1		//消息起始编号

//定义主消息号
enum eMsgMaxid{
	MAXID_START_SESSION = MAXID_BASE		//开始会话类消息(密钥交换,登陆,获取服务器信息...)

	,MAXID_SYS = (MSG_CLASS_NUM + MAXID_BASE-1)		//系统消息
};

/*分别定义次消息号*/

//定义开始会话类消息的次消息号
enum eStartSessionMinid{
	SSM_GETVERSION = 0,				//得到版本信息
	SSM_GETPKI_DIGEST,				//得到公开密钥的摘要
	SSM_GETPKI_KEY,					//得到公开密钥
	SSM_EXCH_CRYPTKEY,				//密钥交换
	SSM_PKI_ARITHMETIC,				//得到公开密钥加密算法
	SSM_SYMMCRYPT_ARITHMETIC,		//得到对称加密算法
	SSM_LOGIN_GATE,					//得到登陆入口地址(IP:port)
	SSM_LOGIN_IN,					//登录(发送用户名和密码)
	SSM_LOGOUT,						//登出				
	SSM_MSG_NUM
};


//define system message min id
enum eSysMsgid {
	SYM_ECHO = 0 					//ECHO消息
	,SYM_TEST						//测试
	,SYM_BROADCAST					//发送广播
} ;

#endif
