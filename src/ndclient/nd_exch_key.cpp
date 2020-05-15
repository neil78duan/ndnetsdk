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
//#include "ndcli/nd_api_c.h"
#include "ndcli/nd_iconn.h"

#include "nd_msg.h"

#define SEND_MSG(__handle, _omsg) \
	nd_connector_send(__handle,(nd_packhdr_t*) (_omsg.GetMsgAddr()), ESF_URGENCY)



#define SEND_AND_WAIT(_handle, _omsg, _rmsg_buf,_wait_min_id) \
	if(0 != ndSendAndWaitMessage(_handle, _omsg.GetMsgAddr(), _rmsg_buf,ND_MAIN_ID_SYS,_wait_min_id, 0,  WAITMSG_TIMEOUT) ) {	\
		return -1;				\
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
			nd_object_seterror(nethandle, NDERR_KEY_UNMATCH) ;
			return -1 ;
		}
		size_t md5size = inmsg.Read(md5text,sizeof(md5text)) ;
		if (md5size < 32 ) {
			nd_object_seterror(nethandle, NDERR_BADPACKET) ;
			return -1 ;
		}
		if(0!=strcmp((const char*)md5text,(const char*) nd_calc_publickey_md5((char*)mymd5) ) ) {
			nd_object_seterror(nethandle, NDERR_KEY_UNMATCH) ;
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
			nd_object_seterror(nethandle, NDERR_INITIAL_ERROR) ;
			return  -1 ;
		}

		//check public key md5
		MD5Crypt16((char*)key_buf, keySize, keymd5) ;
		if(MD5cmp(keymd5, srvmd5)) {
			nd_object_seterror(nethandle, NDERR_KEY_UNMATCH) ;
			return  -1 ;
		}
		

		if (-1== nd_rsa_read_key((R_RSA_PRIVATE_KEY*)&output_key, (const char*)key_buf, keySize, 0)) {
			nd_object_seterror(nethandle, NDERR_KEY_UNMATCH) ;
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

	tea_k newKey = {0};
	tea_key(&newKey) ;
	
	MD5Crypt16((char*)&newKey,	sizeof(tea_k) , mykey.md5);
	nd_TEAencrypt((unsigned char*)mykey.md5, 16, &newKey) ;
	
	nd_teaKeyToNetorder(&mykey.k, &newKey) ;

	NDUINT8  key_crypt[1024];
	int crypt_size = sizeof(key_crypt) ;

	if(0!=rsa_pub_encrypt((char*)key_crypt, &crypt_size, (char*)&mykey, (int)sizeof(mykey), &pub_key) ) {
		nd_object_seterror(nethandle, NDERR_INITIAL_ERROR) ;
		
		nd_logdebug("rea encrypt symm-key error \n") ;
		return -1;
	}
	omsg.WriteBin(key_crypt, crypt_size) ;
	
//	nd_log_screen("orgkey={%x,%x,%x,%x} convertKey={%x,%x,%x,%x}\n",
//				  newKey.k[0],newKey.k[1],newKey.k[2],newKey.k[3],
//				  mykey.k.k[0],mykey.k.k[1],mykey.k.k[2],mykey.k.k[3]);

	//nd_log_screen("key-md5 = %s \n",  MD5ToString(mykey.md5, key_crypt)) ;
	//ready to receive crypt data
	nd_connector_set_crypt(nethandle,(void*)&newKey, sizeof(newKey)) ;

	SEND_AND_WAIT(nethandle, omsg, &rmsg, ND_MSG_SYS_SYM_KEY_ACK)
	else {
		NDIStreamMsg inpkimsg(&rmsg) ;

		NDUINT16 client_sission ;
		if(inpkimsg.Read(client_sission)==-1) {
			nd_object_seterror(nethandle, NDERR_BADPACKET) ;
			return -1 ;
		}
		if(0==client_sission) {
			nd_object_seterror(nethandle, NDERR_KEY_UNMATCH) ;
			return -1 ;

		}
		int sessionid = client_sission ;
		int size = sizeof(int) ;
		nd_net_ioctl((nd_netui_handle)nethandle, NDIOCTL_SET_SESSIONID,&sessionid, &size) ;
		nd_connect_level_set(nethandle, EPL_CONNECT) ;
		

		//tea_k &k = mykey.k ;
		//nd_logdebug("sym-key = { %x, %x, %x, %x} \n" AND k.k[0]  AND  k.k[1] AND  k.k[2] AND  k.k[3]);
	}

	return 0;
}

int nd_exchange_key(nd_handle nethandle,void *out_public_key)
{
	if (-1== _check_public_key( nethandle)) {
		nd_logdebug("check public key md5 error\n") ;
		return -1;
	}

	struct {
		char keymd5[16] ;
		R_RSA_PUBLIC_KEY pub_key ;
	} key ;//= {0} ;
	memset(&key,0,sizeof(key)) ;
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
