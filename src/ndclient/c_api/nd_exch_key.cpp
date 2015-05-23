//
//  nd_exch_key.cpp
//  ndclient_mac
//
//  Created by duanxiuyun on 14-11-13.
//  Copyright (c) 2014 duanxiuyun. All rights reserved.
//


#include "nd_common/nd_common.h"
#include "nd_net/nd_netlib.h"
#include "nd_crypt/nd_crypt.h"

#include "ndapplib/nd_msgpacket.h"
#include "ndcli/nd_api_c.h"
#include "ndcli/nd_iconn.h"

#include "nd_msg.h"

/*
  ,	//get RSA public-key md5
	ND_MSG_SYS_GET_PUBLIC_KEY,		//get RSA public
	ND_MSG_SYS_SEND_SYM_KEY ,		//send SYM encrypt key
	ND_MSG_SYS_SYM_KEY_ACK ,		// acknowledgement of send-sym-key
 */
#define SEND_MSG(__handle, _omsg) \
	nd_connector_send(__handle,(nd_packhdr_t*) (_omsg.GetMsgAddr()), ESF_URGENCY)



#define SEND_AND_WAIT(_handle, _omsg, _rmsg_buf,_wait_min_id) \
if(SEND_MSG(_handle, _omsg) <= 0) {	\
	nd_object_seterror(_handle, NDERR_WRITE) ;	\
	nd_logdebug("send data error \n") ;			\
	return -1 ;									\
}												\
if(-1==ndWaitMsg(_handle, (char*)_rmsg_buf, WAITMSG_TIMEOUT)) {	\
	nd_object_seterror(_handle, NDERR_TIMEOUT) ;	\
	nd_logdebug("wait message error \n") ;			\
	return -1 ;									\
}												\
else if(nd_checkErrorMsg((netObject)_handle ,(ndMsgData *)_rmsg_buf) )   {\
	nd_logdebug("object connect error \n") ;			\
	return -1 ;	\
}\
else if(ND_USERMSG_MAXID(_rmsg_buf)!=ND_MAIN_ID_SYS || ND_USERMSG_MINID(_rmsg_buf) != _wait_min_id) { \
	nd_object_seterror(_handle, NDERR_BADPACKET) ;	\
	nd_logdebug("received error-return message(%d,%d) \n" AND ND_USERMSG_MAXID(_rmsg_buf) AND ND_USERMSG_MINID(_rmsg_buf)) ;				\
	return -1 ;										\
}

static int _check_public_key(nd_handle nethandle)
{
	NDOStreamMsg omsg(ND_MAIN_ID_SYS, ND_MSG_SYS_CHECK_CRYPT_VERSION) ;
	nd_usermsgbuf_t rmsg ; ;

	SEND_AND_WAIT(nethandle, omsg, &rmsg,ND_MSG_SYS_CHECK_CRYPT_VERSION)
	else {
		NDUINT16 ver_num  ;
		NDUINT8 md5text[33] , mymd5[33];
		NDIStreamMsg inmsg(&rmsg) ;
		if (-1==inmsg.Read(ver_num)) {
			nd_object_seterror(nethandle, NDERR_BADPACKET) ;
			return -1 ;
		}
		if ( nd_get_public_certificate_version() != ver_num) {
			nd_object_seterror(nethandle, NDERR_VERSION) ;
			return -1 ;
		}
		size_t md5size = inmsg.Read(md5text,sizeof(md5text)) ;
		if (md5size < 32 ) {
			nd_object_seterror(nethandle, NDERR_BADPACKET) ;
			return -1 ;
		}
		if(0!=strcmp((const char*)md5text,(const char*) nd_calc_publickey_md5((char*)mymd5) ) ) {
			nd_object_seterror(nethandle, NDERR_VERSION) ;
			return  -1 ;
		}
	}
	return 0 ;
}

static int _get_public_key(nd_handle nethandle,R_RSA_PUBLIC_KEY &output_key, char *output_md5)
{
	NDOStreamMsg omsg(ND_MAIN_ID_SYS, ND_MSG_SYS_GET_PUBLIC_KEY) ;
	nd_usermsgbuf_t rmsg ;

	SEND_AND_WAIT(nethandle, omsg, &rmsg ,ND_MSG_SYS_GET_PUBLIC_KEY)
	else {
		char keymd5[16], srvmd5[16] ;
		NDUINT8 recv_buf[2048] ;
		NDIStreamMsg inmsg(&rmsg) ;
		int size = (int) inmsg.ReadBin(recv_buf, sizeof(recv_buf)) ;
		if (size <= 0) {
			nd_object_seterror(nethandle, NDERR_BADPACKET) ;
			return  -1 ;
		}
		if(16!=inmsg.ReadBin(srvmd5, sizeof(srvmd5))  ) {
			nd_object_seterror(nethandle, NDERR_BADPACKET) ;
			return  -1 ;
		}

		R_RSA_PUBLIC_KEY *embedKey = nd_get_publickey();
		nd_assert(embedKey) ;

		char key_buf[2048] ;
		int keySize = sizeof(key_buf) ;

		if(-1==rsa_pub_decrypt(key_buf, &keySize, (char*)recv_buf, size,embedKey) ){
			nd_object_seterror(nethandle, NDERR_VERSION) ;
			return  -1 ;
		}

		//check public key md5
		MD5Crypt16((char*)key_buf, keySize, keymd5) ;
		if(MD5cmp(keymd5, srvmd5)) {
			nd_object_seterror(nethandle, NDERR_BADPACKET) ;
			return  -1 ;
		}
		

		if (-1== nd_rsa_read_key((R_RSA_PRIVATE_KEY*)&output_key, (const char*)key_buf, keySize, 0)) {
			nd_object_seterror(nethandle, NDERR_BADPACKET) ;
			return  -1 ;
		}
		memcpy(output_md5, keymd5, sizeof(keymd5)) ;

	}

	return 0;
}

static int _get_sym_key(nd_handle nethandle,R_RSA_PUBLIC_KEY &pub_key)
{

	NDOStreamMsg omsg(ND_MAIN_ID_SYS, ND_MSG_SYS_SEND_SYM_KEY) ;
	nd_usermsgbuf_t rmsg ;

	//create tea key
	struct {
		tea_k k ;
		char  md5[16] ;
	} mykey;

	tea_key(&mykey.k) ;
	MD5Crypt16((char*)&mykey.k,	sizeof(tea_k) , mykey.md5);
	int len = nd_TEAencrypt((unsigned char*)mykey.md5, 16, &mykey.k) ;
	nd_assert(len == 16) ;


	NDUINT8  key_crypt[1024];
	int crypt_size = sizeof(key_crypt) ;

	if(0!=rsa_pub_encrypt((char*)key_crypt, &crypt_size, (char*)&mykey, (int)sizeof(mykey), &pub_key) ) {
		nd_object_seterror(nethandle, NDERR_VERSION) ;
		
		nd_logdebug("rea encrypt symm-key error \n") ;
		return -1;
	}
	omsg.WriteBin(key_crypt, crypt_size) ;

	//ready to receive crypt data
	nd_connector_set_crypt(nethandle,(void*)&mykey.k, sizeof(mykey.k)) ;

	SEND_AND_WAIT(nethandle, omsg, &rmsg, ND_MSG_SYS_SYM_KEY_ACK)
	else {
		NDIStreamMsg inpkimsg(&rmsg) ;

		NDUINT16 client_sission ;
		if(inpkimsg.Read(client_sission)==-1) {
			nd_object_seterror(nethandle, NDERR_BADPACKET) ;
			return -1 ;
		}
		if(0==client_sission) {
			nd_object_seterror(nethandle, NDERR_BADPACKET) ;
			return -1 ;

		}
		int sessionid = client_sission ;
		int size = sizeof(int) ;
		nd_net_ioctl((nd_netui_handle)nethandle, NDIOCTL_SET_SESSIONID,&sessionid, &size) ;
		nd_connect_level_set(nethandle, EPL_CONNECT) ;
		
		tea_k &k = mykey.k ;
		nd_logdebug("sym-key = { %x, %x, %x, %x} \n", k.k[0],k.k[1],k.k[2],k.k[3] ) ;
	}

	return 0;
}

int nd_exchange_key(netObject netObject,void *out_public_key)
{
	nd_handle nethandle = (nd_handle) netObject ;
	if (-1== _check_public_key( nethandle)) {
		nd_logdebug("check public key md5 error\n") ;
		return -1;
	}

	struct {
		char keymd5[16] ;
		R_RSA_PUBLIC_KEY pub_key ;
	} key = {0} ;
	if (-1== _get_public_key( nethandle,key.pub_key, key.keymd5)) {
		nd_logdebug("get public key md5 error\n") ;
		return -1;
	}

	if (0==_get_sym_key(nethandle,key.pub_key) ){
		if(out_public_key) {
			memcpy(out_public_key, &key, sizeof(key)) ;
		}
		return 0;
	}
	
	nd_logdebug("exchange symm-key error\n") ;
	return -1;
}
