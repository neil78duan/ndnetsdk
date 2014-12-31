//
//  nd_datatransfer.h
//  ndMacStatic
//
//  Created by duanxiuyun on 14-12-31.
//  Copyright (c) 2014年 duanxiuyun. All rights reserved.
//

#ifndef __ndMacStatic__nd_datatransfer__
#define __ndMacStatic__nd_datatransfer__

#include "nd_net/nd_netlib.h"
#include "nd_common/nd_common.h"

typedef void (*data_transfer_callback) (nd_handle nethandle, NDUINT64 param , int error_code);

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


class NDBigDataTransfer 
{
public:
	enum {TRANSFER_UNIT_SIZE = ND_PACKET_SIZE - 1024 } ;
	
	NDBigDataTransfer(nd_handle netObject,data_transfer_callback cb=NULL) ;
	
	virtual ~NDBigDataTransfer() ;
	
	int asyncSend(int maxID, int minID, void*data, size_t datalen, NDUINT64 param ) ;
	void setCallback(data_transfer_callback cb) ;
	
	int timerSend() ;
private:
	void Destroy() ;
	int sendUnit() ;

	int m_maxid, m_minid ;
	nd_handle m_objhandle ;
	NDUINT64 m_param ;
	data_transfer_callback m_completed_callback;
	
	line_buf_hdr m_buf ;
	data_transfer_node m_node ;
};

int BigDataAsyncSend(nd_handle connector, void *data, size_t datalen, NDUINT64 param,data_transfer_callback callback) ;

#endif /* defined(__ndMacStatic__nd_datatransfer__) */
