//
//  nd_datatransfer.cpp
//  ndMacStatic
//
//  Created by duanxiuyun on 14-12-31.
//  Copyright (c) 2014年 duanxiuyun. All rights reserved.
//

#include "ndapplib/nd_datatransfer.h"
#include "ndapplib/nd_msgpacket.h"
#include "nd_msg.h"

int BigDataAsyncSend(nd_handle connector,  void *data, size_t datalen, NDUINT64 param,data_transfer_callback callback) 
{
	if (datalen <= NDBigDataTransfer::TRANSFER_UNIT_SIZE ) {
		NDOStreamMsg omsg(ND_MAIN_ID_SYS, ND_MSG_BIG_DATA_TRANSFER) ;
		omsg.Write(param);
		omsg.Write((NDUINT8)EBIG_DATA_START) ;
		omsg.Write((NDUINT32)datalen) ;
		omsg.WriteBin((NDUINT8*)data, datalen) ;
		return nd_connector_send(connector, (nd_packhdr_t*)omsg.GetMsgAddr(), 0) ;
	}
	else {
		//这里可能会导致内存泄漏需要即使释放
		NDBigDataTransfer *pTransferHelper = new NDBigDataTransfer(connector, callback) ;
		return  pTransferHelper->asyncSend(ND_MAIN_ID_SYS, ND_MSG_BIG_DATA_TRANSFER, data, datalen, param) ;		
	}
}

void _writable_entry(nd_handle handle, nd_userdata_t param) 
{
	nd_assert(handle) ;
	struct list_head *header = (struct list_head*) param ;
	
	if (!param || list_empty(header)) {
		
		nd_userdata_t pWritableParam= 0 ;
		int param_size = sizeof(pWritableParam);
		
		nd_net_ioctl((nd_netui_handle)handle,NDIOCTL_SET_WRITABLE_CALLBACK_PARAM, &pWritableParam, &param_size) ;
		nd_net_ioctl((nd_netui_handle)handle,NDIOCTL_SET_WRITABLE_CALLBACK, &pWritableParam, &param_size) ;
		
		delete header ;
		
		return ;
	}
	struct list_head *next = header->next ;
	data_transfer_node *node = list_entry(next, data_transfer_node, list) ;	
	node->pDataTransfer->timerSend() ;
	
}

NDBigDataTransfer::NDBigDataTransfer(nd_handle netObject,data_transfer_callback cb) : m_objhandle(netObject),m_completed_callback(cb)
{
	m_maxid = 0;
 	m_minid = 0;
	m_param = 0 ;	
	memset(&m_buf, 0 ,sizeof(m_buf)) ;
	
	INIT_LIST_HEAD(&m_node.list) ;
	m_node.pDataTransfer = this ;
}

NDBigDataTransfer::~NDBigDataTransfer() 
{
	//Destroy() ;
}

int NDBigDataTransfer::asyncSend(int maxID, int minID, void*data, size_t datalen, NDUINT64 param ) 
{	
	m_maxid = maxID;
	m_minid = minID;
	m_param = param ;	
	
	ndlbuf_init(&m_buf, datalen) ;
	ndlbuf_write(&m_buf, data, datalen, 0) ;
	//try to create data transfer list 
	bool isEmpty = false ;
	 
	nd_userdata_t pWritableParam= 0 ;
	int param_size = sizeof(pWritableParam);
	struct list_head *list_hdr = NULL ;
	
	if(-1==nd_net_ioctl((nd_netui_handle)m_objhandle,NDIOCTL_GET_WRITABLE_CALLBACK_PARAM, &pWritableParam, &param_size) ) {		
		return -1;
	}
	if (pWritableParam==0) {
		list_hdr = new list_head() ;
		INIT_LIST_HEAD(list_hdr) ;
		pWritableParam = list_hdr ;
		
		param_size = sizeof(pWritableParam) ;
		nd_net_ioctl((nd_netui_handle)m_objhandle,NDIOCTL_SET_WRITABLE_CALLBACK_PARAM, pWritableParam, &param_size);
		
		pWritableParam = (nd_userdata_t) _writable_entry;
		nd_net_ioctl((nd_netui_handle)m_objhandle,NDIOCTL_SET_WRITABLE_CALLBACK, pWritableParam, &param_size);
		
		isEmpty =true ;
	}
	else {
		list_hdr = (struct list_head*) pWritableParam ;
		isEmpty = list_empty(list_hdr) ;
	}
	
	list_add_tail(list_hdr, &m_node.list) ;
	
	if (isEmpty) {
		timerSend() ;
	}
	
	return (int) datalen;
}


int NDBigDataTransfer::sendUnit() 
{
	size_t datalen = ndlbuf_datalen( &m_buf ) ;
	size_t size = TRANSFER_UNIT_SIZE ;	
	NDUINT8 *pData = (NDUINT8*) ndlbuf_data(&m_buf) ;
	
	NDOStreamMsg omsg(m_maxid, m_minid) ;
	omsg.Write(m_param);
	
	//size = min(size, (size_t)ndlbuf_datalen(&m_buf)) ;
	if (size > datalen) {
		size = datalen;
	}
	
	if (pData == ndlbuf_raw_addr(&m_buf) ) {		
		omsg.Write((NDUINT8)EBIG_DATA_START) ;
		omsg.Write((NDUINT32)datalen) ;
	}
	else {		
		omsg.Write((NDUINT8)EBIG_DATA_CONTINUE) ;
	}
	nd_assert(size > 0) ;
	
	
	omsg.WriteBin(pData, size) ;
	int len = nd_connector_send(m_objhandle, (nd_packhdr_t*)omsg.GetMsgAddr(), 0) ;
	
	if (len > 0) {
		ndlbuf_sub_data(&m_buf, size) ;
		if (ndlbuf_datalen(&m_buf)==0 ) {
			m_completed_callback(m_objhandle ,m_param, 0) ;
			Destroy() ;
			delete this ;
			return  0 ;
		}
	}	
	return len;
}


int NDBigDataTransfer::timerSend() 
{
	do {
		if (sendUnit() ==0) {
			break ;
		}
	} while (nd_socket_wait_writablity(nd_connector_fd(m_objhandle), 0) > 0)  ;
	return 0 ;
}

void NDBigDataTransfer::Destroy() 
{
	ndlbuf_destroy(&m_buf) ;
	list_del_init(&m_node.list) ;
}

