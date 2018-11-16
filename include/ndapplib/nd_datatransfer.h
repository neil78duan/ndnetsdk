//
//  nd_datatransfer.h
//  ndMacStatic
//
//  Created by duanxiuyun on 14-12-31.
//  Copyright (c) 2014 duanxiuyun. All rights reserved.
//

#ifndef __ndMacStatic__nd_datatransfer__
#define __ndMacStatic__nd_datatransfer__

#include "nd_net/nd_netlib.h"
#include "nd_common/nd_common.h"
#include "ndcli/nd_api_c.h"

typedef void (*data_transfer_callback) (nd_handle nethandle, NDUINT64 param , int error_code);

typedef void (*data_recv_callback) (void *receiver, NDUINT64 param , void *data, size_t size);

class NDIStreamMsg;
class NDBigDataTransfer ;
struct data_transfer_node
{
	struct list_head list ;
	NDBigDataTransfer* pDataTransfer ;
	
};

enum eBigDataType{
	EBIG_DATA_START ,
	EBIG_DATA_CONTINUE,
	EBIG_DATA_END
};


class  ND_COMMON_CLASS NDBigDataTransfer
{
public:
	enum {TRANSFER_UNIT_SIZE = ND_PACKET_SIZE - 1024 } ;
	
	NDBigDataTransfer(nd_handle netObject,data_transfer_callback cb=NULL) ;
	
	virtual ~NDBigDataTransfer() ;
	
	int asyncSend(int maxID, int minID, void*data, size_t datalen, NDUINT64 param ) ;
	void setCallback(data_transfer_callback cb) ;
	
	int timerSend() ;
	void Destroy(int errorCode=0) ;
private:
	int sendUnit() ;

	int m_SendIndex ;
	int m_maxid, m_minid ;
	nd_handle m_objhandle ;
	NDUINT64 m_param ;
	data_transfer_callback m_completed_callback;
	
	nd_linebuf m_buf ;
	data_transfer_node m_node ;
};


class  ND_COMMON_CLASS NDBigDataReceiver
{
public:
	NDBigDataReceiver(data_recv_callback cb, void *receiver=NULL) ;
	virtual ~NDBigDataReceiver() ;
	int OnRecv(NDIStreamMsg &inmsg) ;
	void Reset() ;
	void SetReceive(void *p) { m_receiver = p;}
	void SetHandler(data_recv_callback handler)  {m_recv_ok_callback = handler;} 
private:
	bool CheckInit() ;
	void *m_receiver ;	
	NDUINT64 m_param ;
	size_t m_dataSize ;
	data_recv_callback m_recv_ok_callback ;
	nd_linebuf m_buf ;
};

ND_CONNCLI_API int BigDataAsyncSend(nd_handle connector, void *data, size_t datalen, NDUINT64 param, data_transfer_callback callback);

#endif /* defined(__ndMacStatic__nd_datatransfer__) */
