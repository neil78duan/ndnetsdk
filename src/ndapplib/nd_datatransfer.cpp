//
//  nd_datatransfer.cpp
//  ndMacStatic
//
//  Created by duanxiuyun on 14-12-31.
//  Copyright (c) 2014 duanxiuyun. All rights reserved.
//

#include "ndapplib/nd_datatransfer.h"
#include "ndapplib/nd_msgpacket.h"
#include "nd_msg.h"

int BigDataAsyncSend(nd_handle connector,  void *data, size_t datalen, NDUINT64 param,data_transfer_callback callback) 
{
	if (datalen <= NDBigDataTransfer::TRANSFER_UNIT_SIZE ) {
		NDOStreamMsg omsg(ND_MAIN_ID_SYS, ND_MSG_BIG_DATA_TRANSFER) ;
		omsg.Write((NDUINT8)EBIG_DATA_START) ;
		omsg.Write(param);
		omsg.Write((NDUINT32)datalen) ;
		omsg.WriteBin((NDUINT8*)data, datalen) ;
		return nd_connector_send(connector, (nd_packhdr_t*)omsg.GetMsgAddr(), 0) ;
	}
	else {
		nd_logmsg("send big data len=%d\n", (int)datalen) ;
		NDBigDataTransfer *pTransferHelper = new NDBigDataTransfer(connector, callback) ;
		return  pTransferHelper->asyncSend(ND_MAIN_ID_SYS, ND_MSG_BIG_DATA_TRANSFER, data, datalen, param) ;		
	}
}


static void _destroy_on_clost(nd_handle handle, void *param) 
{
	data_transfer_node *node,*next ;
	
	struct list_head *hdr = (struct list_head *)param ;
	list_for_each_entry_safe(node, next, hdr, data_transfer_node, list) {
		if (node->pDataTransfer) {
			node->pDataTransfer->Destroy(NDERR_CLOSED) ;
			delete node->pDataTransfer ;
			node->pDataTransfer = 0 ;
		}
	}
	
	
	nd_userdata_t pWritableParam= 0 ;
	int param_size = sizeof(pWritableParam);
	
	nd_net_ioctl((nd_netui_handle)handle,NDIOCTL_SET_WRITABLE_CALLBACK_PARAM, &pWritableParam, &param_size) ;
	nd_net_ioctl((nd_netui_handle)handle,NDIOCTL_SET_WRITABLE_CALLBACK, &pWritableParam, &param_size) ;
	
	
	nd_logmsg("send data not success when closed \n") ;
	delete hdr ;
	
}

static void _writable_entry(nd_handle handle, nd_userdata_t param) 
{
	nd_assert(handle) ;
	struct list_head *header = (struct list_head*) param ;
	
	if (!param || list_empty(header)) {
		
		nd_userdata_t pWritableParam= 0 ;
		int param_size = sizeof(pWritableParam);
		
		
		nd_net_ioctl((nd_netui_handle)handle,NDIOCTL_SET_WRITABLE_CALLBACK_PARAM, &pWritableParam, &param_size) ;
		nd_net_ioctl((nd_netui_handle)handle,NDIOCTL_SET_WRITABLE_CALLBACK, &pWritableParam, &param_size) ;
		
		nd_connector_del_close_callback(handle, _destroy_on_clost, header) ;
		delete header ;
		
		nd_tcpnode_tryto_flush_sendbuf((nd_tcp_node*)handle) ;
		nd_logdebug("send data complete \n") ;
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
	m_SendIndex = 0 ;
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
		nd_net_ioctl((nd_netui_handle)m_objhandle,NDIOCTL_SET_WRITABLE_CALLBACK_PARAM, &pWritableParam, &param_size);
		
		pWritableParam = (nd_userdata_t) _writable_entry;
		nd_net_ioctl((nd_netui_handle)m_objhandle,NDIOCTL_SET_WRITABLE_CALLBACK, &pWritableParam, &param_size);
		
		isEmpty =true ;
		
		nd_connector_add_close_callback(m_objhandle, _destroy_on_clost, list_hdr) ;
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
	
	//size = min(size, (size_t)ndlbuf_datalen(&m_buf)) ;
	if (size > datalen) {
		size = datalen;
	}
	
	if (m_SendIndex==0) {		
		omsg.Write((NDUINT8)EBIG_DATA_START) ;		
		omsg.Write(m_param);
		omsg.Write((NDUINT32)datalen) ;
		
	}
	else {				
		omsg.Write((NDUINT8)EBIG_DATA_CONTINUE) ;
	}
	nd_assert(size > 0) ;
	
	omsg.Write((NDUINT16)m_SendIndex) ;
	omsg.WriteBin(pData, size) ;
	int len = nd_connector_send(m_objhandle, (nd_packhdr_t*)omsg.GetMsgAddr(), 0) ;
	
	if (len== omsg.MsgLength()) {
		
		//nd_logdebug("%d pack-len=%d send data %d success \n",m_SendIndex, len, size) ;
		
		++m_SendIndex ;
		
		ndlbuf_sub_data(&m_buf, size) ;
		if (ndlbuf_datalen(&m_buf)==0 ) {
			
			nd_tcpnode_tryto_flush_sendbuf((nd_tcp_node*)m_objhandle) ;
			
			Destroy() ;
			delete this ;
			
			//nd_logmsg("send data COMPLETED \n") ;
			return  0 ;
		}
	}
	else {
		nd_logerror("send big data(%d) error send-len = %d\n", size, len);
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

void NDBigDataTransfer::Destroy(int errorCode) 
{
	if (m_completed_callback) {
		m_completed_callback(m_objhandle ,m_param, 0) ;
	}	
	
	ndlbuf_destroy(&m_buf) ;
	list_del_init(&m_node.list) ;
	m_SendIndex = 0;
}
////////////////////////////////////////////
NDBigDataReceiver::NDBigDataReceiver(data_recv_callback cb, void *receiver) : m_recv_ok_callback(cb), m_receiver(receiver)
{
	
	memset(&m_buf, 0 ,sizeof(m_buf)) ;
	m_param = 0;
	
}

NDBigDataReceiver::~NDBigDataReceiver() 
{
	Reset() ;
}

int NDBigDataReceiver::OnRecv(NDIStreamMsg &inmsg) 
{
	NDUINT8 type ;
	NDUINT16 index = 0 ;
	size_t read_len =0;
	NDUINT8 data[ND_PACKET_SIZE];
	if (-1== inmsg.Read(type)) {
		return NDERR_BADPACKET ;
	}
	if (type == EBIG_DATA_START) {
		if (CheckInit()) {
			return NDERR_INVALID_INPUT ;
		}
		if (-1==inmsg.Read(m_param)) {
			return NDERR_BADPACKET ;
		}
		
		NDUINT32 datalen =0;
		if (-1==inmsg.Read(datalen)) {
			return NDERR_BADPACKET;
		}
		ndlbuf_init(&m_buf, datalen + sizeof(size_t)) ;	
		m_dataSize = datalen ;
	}
	else {
		if (!CheckInit()) {
			return NDERR_INVALID_INPUT ;
		}
	}
	if (0!=inmsg.Read(index)) {		
		return NDERR_INVALID_INPUT ;
	}
	
	read_len = inmsg.ReadBin(data, sizeof(data)) ;
	if (read_len > ndlbuf_capacity(&m_buf)) {
		return NDERR_INVALID_INPUT ;
	}
	//static int recv_index = 0 ;
	//nd_logmsg("%d recv packet-len = %d data len =%d\n", index,inmsg.MsgLength() ,read_len) ;
	//++recv_index ;
	
	ndlbuf_write(&m_buf, data, read_len, 0) ;
	if (m_dataSize== ndlbuf_datalen(&m_buf)) {
		if (m_recv_ok_callback) {			
			m_recv_ok_callback( m_receiver, m_param, ndlbuf_data(&m_buf), ndlbuf_datalen(&m_buf)) ;
		}
		Reset();
		return NDERR_SUCCESS ;
	}
	
	return NDERR_WUOLD_BLOCK;
}

bool NDBigDataReceiver::CheckInit() 
{
	if(ndlbuf_capacity(&m_buf) > 0) {
		return true ;
	}
	return false;
}

void NDBigDataReceiver::Reset() 
{
	m_param =0 ;
	ndlbuf_destroy(&m_buf) ;
	
}


