/* file nd_netui.c
 * implement net user interface
 * 
 *
 * neil duan 
 * all right reserved 2008
 */

/*
 * 实现一个在nd_engine中所以网络协议都统一的一个运用层接口
 */
#define ND_IMPLEMENT_HANDLE
typedef struct netui_info *nd_handle;

//#include "nd_common/nd_common.h"
#include "nd_net/nd_netlib.h"
//#include "nd_common/nd_alloc.h"

static int _crypt_unit_len ;		//加密单元长度
static nd_netcrypt __net_encrypt, __net_decrypt ;
static int _min_packet_len = sizeof(nd_usermsghdr_t) ;
static int nd_net_message_version_error(nd_netui_handle node) ;

/* 更新客户端的网络连接 */
int nd_connector_update(nd_netui_handle net_handle,ndtime_t timeout) 
{
	ENTER_FUNC()
	int ret =0;
	int read_len ;
	nd_assert(net_handle ) ;

	if(!nd_connector_valid(net_handle) || nd_tryto_clear_err(net_handle)) {
		LEAVE_FUNC();
		return -1;
	}

	if(net_handle->type==NDHANDLE_TCPNODE){
		//把tcp的流式协议变成消息模式
		struct nd_tcp_node *socket_node = (struct nd_tcp_node *) net_handle ;
		
		
		read_len = nd_tcpnode_read(socket_node) ;
		if (timeout ){
			if (read_len == 0)	{
				int waitret =nd_socket_wait_read(socket_node->fd,timeout) ;
				if(waitret<=0) {
					
					TCPNODE_TRY_CALLBACK_WRITE(socket_node) ;	//check writable
					if(socket_node->update_entry((nd_handle)socket_node)==-1) {
						waitret = -1;
					}
					LEAVE_FUNC();
					return waitret ;
				}
				read_len = nd_tcpnode_read(socket_node) ;
			}
		}

RE_READ:
		if(-1==read_len) {
			LEAVE_FUNC();
			return -1;
		}
		else if(0==read_len) {
			if (ret > 0) {
				nd_tcpnode_flush_sendbuf((nd_netui_handle)socket_node) ;
			}
			if(socket_node->update_entry((nd_handle)socket_node)==-1) {
				ret = -1;
			}			
			TCPNODE_TRY_CALLBACK_WRITE(socket_node) ;	//check writable
			
			LEAVE_FUNC();
			return ret;
		}
		else {
			ret += read_len ;
			if(-1==handle_recv_data((nd_netui_handle)socket_node, NULL) ) {
				LEAVE_FUNC();
				return -1 ;
			}
			if(TCPNODE_READ_AGAIN(socket_node)) {
				/*read buf is to small , after parse data , read again*/
				read_len = nd_tcpnode_read(socket_node) ;
				goto RE_READ;
			}
			nd_tcpnode_flush_sendbuf((nd_netui_handle)socket_node) ;
			if(socket_node->update_entry((nd_handle)socket_node)==-1) {
				ret = -1;
			}			
			TCPNODE_TRY_CALLBACK_WRITE(socket_node) ;	//check writable
		}
	}
	else if(net_handle->type==NDHANDLE_UDPNODE) {
		nd_udp_node *socket_addr =(nd_udp_node *) net_handle ;
		char read_buf[ND_UDP_PACKET_SIZE] ;

		//socket_addr->myerrno = NDERR_SUCCESS ;

		nd_assert(socket_addr->update_entry) ;
		if(-1== socket_addr->update_entry((nd_handle)socket_addr)) {
			LEAVE_FUNC();
			return -1;
		}
RE_UDTREAD:
		read_len =  socket_addr->sock_read((nd_handle)socket_addr, read_buf,sizeof(read_buf),timeout) ;
		if(read_len > 0 ) {
			read_len = nd_udp_parse((nd_handle)socket_addr, read_buf, read_len) ;
			if(-1==read_len) {
				LEAVE_FUNC();
				return -1 ;
			}
			else if(0==read_len){
				LEAVE_FUNC();
				return ret ;
			}
			ret += read_len ;
			if(-1==socket_addr->update_entry((nd_handle)socket_addr)) {
				LEAVE_FUNC();
				return -1;
			}
			if(TCPNODE_READ_AGAIN(socket_addr)) {
				/*read buf is to small , after parse data , read again*/
				goto RE_UDTREAD;
			}
		}
		else if(read_len==-1 ) {
			if (socket_addr->myerrno==NDERR_BADPACKET)	{
				if(TCPNODE_READ_AGAIN(socket_addr)) {
					goto RE_UDTREAD;
				}
			}
			else {
				LEAVE_FUNC();
				return 0;
			}
		}
	}
	LEAVE_FUNC();
	return ret;	
}

//处理接收到的数据
int handle_recv_data(nd_netui_handle node, nd_handle h_listen)
{
	ENTER_FUNC()
	int ret =0;

	nd_assert(node) ;
	nd_assert(node->data_entry) ;

	if (nd_tryto_clear_err(node))	{
		LEAVE_FUNC();
		return -1;
	}
	
	if(ndlbuf_datalen(&(node->recv_buffer) ) == 0){
		ndlbuf_reset(&(node->recv_buffer));
		LEAVE_FUNC();
		return 0 ;
	}
	//////////////////////////////////////////////////////////////////////////	
	/*if (node->is_session){		
		ret = node->data_entry(node, ndlbuf_data(&(node->recv_buffer)), data_len, node->srv_root) ;
		if(ret > 0 ){
			ndlbuf_sub_data(&(node->recv_buffer),ret) ;
		}		
	}
	else*/ {
		int read_len = 0;
		nd_packetbuf_t pack_buf ;
		nd_hdr_init(&pack_buf.hdr) ;

		//把数据拷贝出来在处理,防止迭代
		while((read_len = nd_net_fetch_msg(node, &pack_buf.hdr) ) > 0){
			if (read_len != pack_buf.hdr.length){
				nd_assert(0);
				return -1;
			}
			read_len = _packet_handler(node, &pack_buf.hdr, h_listen) ;
			if(-1==read_len) {
				ret = -1 ;
				break ;
			}
			ret += read_len;
			if (NDERR_USER_BREAK == nd_object_lasterror(node)){
				nd_object_seterror(node,NDERR_SUCCESS) ;
				break ;
			}
		}
		if (-1==read_len) {
			ret = -1;
		}
	}
	//////////////////////////////////////////////////////////////////////////
	
	LEAVE_FUNC();
	return ret ;

}

//packet handler function entry 
// translate message to message handler 
int _packet_handler(nd_netui_handle node,nd_packhdr_t *msg, nd_handle h_listen)
{
	ENTER_FUNC()
	int ret = nd_pack_len(msg) ;
	nd_assert(node) ;
	nd_assert(node->msg_entry);

	if (msg->ndsys_msg){
		if(-1==nd_net_sysmsg_hander(node,(nd_sysresv_pack_t *)msg))
			ret = -1;
	}
	else {
		ret = node->msg_entry(node, msg,h_listen) ; 
	}
	/*
	if(msg->encrypt) {
		int new_len ,old_len;
		new_len = nd_packet_decrypt(node, (nd_packetbuf_t*)msg) ;
		if(new_len==0) {			
			LEAVE_FUNC();
			return -1;
		}
		CURRENT_IS_CRYPT(node) = 1 ;
		old_len = (int) nd_pack_len(msg) ;
		nd_pack_len(msg)  = (NDUINT16)new_len ;

		ret = node->msg_entry(node, msg,h_listen) ;
		//ret = nd_translate_message((nd_netui_handle)node, msg ) ;
		nd_pack_len(msg) = old_len ;
		ret = old_len ; //返回解密之前的数据长度，无论如何，解密后的数据都认为是已经被处理的
	}
	else {
		CURRENT_IS_CRYPT(node) = 0 ;
		ret = node->msg_entry(node, msg,h_listen) ;
	}
	*/
	LEAVE_FUNC();
	return ret;
}

//处理ND格式的网络消息
//default net message handle , parse data to ND_PROTOCOL
int nd_dft_packet_handler(nd_netui_handle node,void *data , size_t data_len , nd_handle h_listen)
{
	ENTER_FUNC()
	int ret =0;		
	size_t used_len = 0   ;
	nd_packhdr_t *msg_addr ; //; = (nd_packhdr_t *)data ;

	nd_assert(node->msg_entry) ;

	if (nd_tryto_clear_err(node))	{
		LEAVE_FUNC();
		return -1;
	}
	//node->myerrno = NDERR_SUCCESS ;
	
RE_MESSAGE:
	if(data_len < ND_PACKET_HDR_SIZE){
		LEAVE_FUNC();
		return ret ;
	}

	msg_addr  = (nd_packhdr_t *)data ;

	packet_ntoh(msg_addr) ;
	used_len = nd_pack_len(msg_addr) ;

	if(used_len > ND_PACKET_SIZE || used_len < _min_packet_len) {
		//packet_hton(msg_addr) ;
		node->myerrno = NDERR_BADPACKET ;
		LEAVE_FUNC();
		return -1 ;
	}
	if(used_len<=data_len){
		int user_ret ;
		//node->myerrno = NDERR_USER  ;
		user_ret =_packet_handler(node,msg_addr,h_listen) ;
		if(-1==user_ret ) {
			LEAVE_FUNC();
			return -1;
		}
		else if(0==user_ret) {
			//上层函数暂时不能处理这些数据
			node->myerrno = NDERR_SUCCESS ;
			LEAVE_FUNC();
			return ret ;
		}
		ret += (int) used_len ;
		data = (void*) (((char*)data) + used_len );
		data_len -= used_len ;

		if (!nd_connector_valid(node)) {
			LEAVE_FUNC();
			return -1 ;
		}
		if(data_len >= ND_PACKET_HDR_SIZE && node->myerrno != NDERR_USER_BREAK ){
			node->myerrno = NDERR_SUCCESS ;
			goto RE_MESSAGE ;
		}
		node->myerrno = NDERR_SUCCESS ;
	}
	LEAVE_FUNC();
	return ret;
}

//fetch recvd message in nd_packhdr_t format
int nd_net_fetch_msg(nd_netui_handle socket_addr, nd_packhdr_t *msgbuf)
{
	ENTER_FUNC() 
	int data_len =0, valid_len=0;
	nd_packhdr_t  tmp_hdr ;
	nd_netbuf_t  *pbuf;
	nd_packhdr_t *stream_data ;

	if (!nd_connector_valid(socket_addr)) {
		LEAVE_FUNC();
		return -1;
	}
	
RE_FETCH:
	pbuf  = &(socket_addr->recv_buffer) ;
	data_len =(int)  ndlbuf_datalen(pbuf) ;

	if(data_len < ND_PACKET_HDR_SIZE) {
		LEAVE_FUNC();
		return 0;
	}

	stream_data = (nd_packhdr_t *)ndlbuf_data(pbuf) ;

	ND_HDR_SET(&tmp_hdr, stream_data) ;
	//nd_hdr_ntoh(&tmp_hdr) ;

	//check incoming data legnth
	valid_len = (int) socket_addr->get_pack_size(socket_addr, &tmp_hdr) ;
	//valid_len =  nd_pack_len(&tmp_hdr) ;

	if(valid_len >ND_PACKET_SIZE || valid_len < _min_packet_len) {
		socket_addr->myerrno = NDERR_BADPACKET ;
		LEAVE_FUNC();
		return -1 ;	//incoming data error 
	}
	else if(valid_len > data_len ) {
		LEAVE_FUNC();
		return 0 ;	//not enough
	}
	
	if (socket_addr->user_define_packet){
		//user define packet data
		nd_atomic_inc(&socket_addr->recv_pack_times) ;
		ndlbuf_read(pbuf, msgbuf,valid_len,EBUF_SPECIFIED ) ;
        nd_packet_ntoh(msgbuf) ;
		return valid_len ;
	}

	/*if ((int)NDNETMSG_VERSION != nd_pack_version(&tmp_hdr)){
		nd_net_message_version_error(socket_addr);
		return -1;
	}
	else*/ if (tmp_hdr.ndsys_msg){		
		if (-1==nd_net_sysmsg_hander(socket_addr,(nd_sysresv_pack_t *)stream_data) ){
			LEAVE_FUNC();
			return -1;
		}
		ndlbuf_sub_data(pbuf, valid_len) ;
		valid_len = 0;
		goto RE_FETCH;
	}

	if (msgbuf )	{
		ndlbuf_read(pbuf, msgbuf,valid_len,EBUF_SPECIFIED ) ;
		
        nd_packet_ntoh(msgbuf) ;

		nd_assert(valid_len == nd_pack_len(msgbuf)) ;
		if(msgbuf->encrypt) {
			int new_len ;
			new_len = nd_packet_decrypt(socket_addr,(nd_packetbuf_t *) msgbuf) ;
			if(new_len==0) {
				socket_addr->myerrno = NDERR_BADPACKET ;
				LEAVE_FUNC();
				return -1;
			}
			nd_pack_len(msgbuf)  = (NDUINT16)new_len ;
			valid_len = new_len ;
			CURRENT_IS_CRYPT(socket_addr) = 1 ;
		}
		else {
			CURRENT_IS_CRYPT(socket_addr) = 0 ;
		}
	}
	nd_atomic_inc(&socket_addr->recv_pack_times) ;
	LEAVE_FUNC();
	return valid_len ;
}

//发送tcp封包
static int _tcp_connector_send(struct nd_tcp_node* socket_addr, nd_packhdr_t *msg_buf, int flag) 
{
	ENTER_FUNC()
	int ret ;
	if(!TCPNODE_CHECK_OK(socket_addr)) {
		socket_addr->myerrno = NDERR_BADSOCKET ;
		LEAVE_FUNC();
		return -1 ;
	}
	socket_addr->myerrno = NDERR_SUCCESS ;
	nd_send_lock((nd_netui_handle)socket_addr) ;	//注意在这里如果用的时IOCP模式将不能使用metux lock
	ret = nd_tcpnode_send( socket_addr,	msg_buf, flag) ;
	nd_send_unlock((nd_netui_handle)socket_addr) ;

// 	if (ret != nd_pack_len(msg_buf)){
// 		nd_logmsg("send data error datalen=%d send len=%d\n" AND 
// 			msg_buf->length AND ret);
// 	}
	LEAVE_FUNC();
	return ret ;
}

/* 发送网络消息 
 * @net_handle 网络连接的句柄,指向struct nd_tcp_node(TCP连接)
 *		或者ndudt_socket(UDT)节点
 * @nd_msgui_buf 发送消息内容
 * @flag ref send_flag
 */
//int nd_connector_send(nd_netui_handle net_handle,nd_msgui_buf *msg_buf, int flag) 
int nd_connector_send(nd_netui_handle net_handle, nd_packhdr_t *msg_buf, int flag) 
{
	ENTER_FUNC()
	int ret ;
	nd_assert(net_handle) ;
	nd_assert(msg_buf) ;
	nd_assert(net_handle->write_entry) ;

	if(!nd_connector_valid(net_handle)|| nd_tryto_clear_err(net_handle)) {
		LEAVE_FUNC();
		return -1;
	}
    msg_buf->version = NDNETMSG_VERSION ;
	if(flag & ESF_ENCRYPT ) {
		nd_packetbuf_t crypt_buf ;
		memcpy(&crypt_buf, msg_buf, nd_pack_len(msg_buf)) ;
		nd_packet_encrypt(net_handle, &crypt_buf) ;
		nd_atomic_inc(&net_handle->send_pack_times) ;
        
        packet_hton(&crypt_buf) ;
		ret =net_handle->write_entry(net_handle,&crypt_buf.hdr, flag) ;
	}
	else {
		nd_atomic_inc(&net_handle->send_pack_times) ;
        
        packet_hton(msg_buf) ;
		ret =net_handle->write_entry(net_handle,msg_buf, flag) ;
        packet_ntoh(msg_buf) ;
	}
    
	
	LEAVE_FUNC();
	return ret ;
}


int nd_connector_raw_write(nd_handle net_handle , void *data, size_t size) 
{
	int ret ;
	ENTER_FUNC() 
	nd_assert(net_handle->sock_write) ;
	ret = net_handle->sock_write(net_handle , data,  size);
	//ret = net_handle->write_entry(net_handle , (nd_packhdr_t *)data,  size);
	LEAVE_FUNC() ;
	return ret ;
}

/*
 * 重新连接到一个新的服务器
 */
int nd_reconnect(nd_netui_handle net_handle, ndip_t ip, int port, struct nd_proxy_info *proxy)
{
	ENTER_FUNC() 
	int ret = 0 ;
	ndtime_t starttime = net_handle->start_time ;
	nd_connector_close(net_handle, 0) ;


	ret = nd_connector_open( net_handle,(char*)nd_inet_ntoa(ip,NULL), port, proxy) ;
	if (ret == 0){
		net_handle->start_time = starttime;
		nd_object_seterror(net_handle,NDERR_SUCCESS);
	}
	LEAVE_FUNC();
	return ret ;
}

/*
 * 重新连接到一个新的服务器
 */
int nd_reconnectex(nd_netui_handle net_handle, const char *host, int port, struct nd_proxy_info *proxy)
{
	ENTER_FUNC() 

	int ret = 0 ;
	ndtime_t starttime = net_handle->start_time ;
	nd_connector_close(net_handle, 0) ;


	ret = nd_connector_open( net_handle,host, port, proxy) ;	
	if (ret == 0){
		net_handle->start_time = starttime;
		nd_object_seterror(net_handle,NDERR_SUCCESS);
	}
	LEAVE_FUNC();
	return 0 ;

// 	nd_assert(net_handle ) ;
// 	nd_connector_reset(net_handle) ;
// 	return nd_connector_open( net_handle,host, port, proxy) ;	
}


int nd_connector_open(nd_netui_handle net_handle,const char *host, int port,struct nd_proxy_info *proxy)
{	
	ENTER_FUNC()
	int ret =0;

	if (net_handle->fd){
		nd_connector_reset(net_handle) ;
	}
	else {
		ndlbuf_reset(&(net_handle->recv_buffer)) ;		/* buffer store data recv from net */
		ndlbuf_reset(&(net_handle->send_buffer)) ;		/* buffer store data send from net */
	}
	
	if(net_handle->type==NDHANDLE_TCPNODE){
		struct nd_tcp_node* socket_addr=(struct nd_tcp_node*)net_handle ;
		socket_addr->myerrno = NDERR_SUCCESS ;
		ret = nd_tcpnode_connect(host, port,socket_addr, proxy ) ;
		if(-1==ret  ) {
			socket_addr->myerrno = NDERR_OPENFILE ;
		}else {
			net_init_sendlock(net_handle) ;
			socket_addr->write_entry = (packet_write_entry)_tcp_connector_send ;
		}
	}
	else if(net_handle->protocol==PROTOCOL_UDT) {
		nd_udt_node* socket_addr =(nd_udt_node*)net_handle ;
		nd_netui_handle n_handle =(nd_netui_handle) udt_connect(socket_addr,host, (short)port,proxy) ;

		if(n_handle){				
			net_init_sendlock(net_handle) ;		//client socket need send lock
		}
		else {
			ret = socket_addr->status == NETSTAT_ESTABLISHED ;
		}
	}
	else {
		ret = -1 ;
	}

	if (0==ret ){
		net_handle->last_push = net_handle->last_recv = nd_time() ;
	}

	LEAVE_FUNC();
	return ret ;
}

int nd_connector_valid(nd_netui_handle net_handle)
{
	ENTER_FUNC()
	nd_assert(net_handle) ;
	if (!net_handle->fd || nd_object_check_error(net_handle)) {
		LEAVE_FUNC();
		return 0 ;
	}
	//nd_msgtable_destroy(net_handle) ;
	if(net_handle->type==NDHANDLE_TCPNODE){
		if(TCPNODE_CHECK_OK(net_handle)) {
			LEAVE_FUNC();
			return 1 ;
		}
	}
	else if(net_handle->type==NDHANDLE_UDPNODE) {
		if(net_handle->protocol==PROTOCOL_UDT) {
			LEAVE_FUNC();
			return net_handle->status == NETSTAT_ESTABLISHED ;
		}
		LEAVE_FUNC();
		return 1;
	}
	LEAVE_FUNC();
	return 0;
}

int nd_connector_close(nd_netui_handle net_handle, int flag)
{	
	ENTER_FUNC()
	//struct net_handle_header *h_header =(struct net_handle_header *)net_handle ;
	nd_assert(net_handle) ;
	//nd_msgtable_destroy(net_handle) ;
	if(net_handle->type==NDHANDLE_TCPNODE){

		if (((struct nd_tcp_node*)net_handle)->status != ETS_DEAD )	{
			if (net_handle->data_entry) {
				net_handle->data_entry(net_handle,NULL,0,0) ;
			}
			nd_tcpnode_close((struct nd_tcp_node*)net_handle,flag);
			net_release_sendlock(net_handle) ;
		}
		LEAVE_FUNC();
		return 0 ;
	}
	else if(net_handle->type==NDHANDLE_UDPNODE) {
		if(net_handle->protocol==PROTOCOL_UDT) {
			nd_udt_node *socket_addr = (nd_udt_node*)net_handle ;

			if(socket_addr->fd>0) {
				udt_close(socket_addr,1);
				net_release_sendlock(net_handle) ;		//client socket need send lock
			}
			LEAVE_FUNC();
			return 0;
		}
		else {
			if (net_handle->fd) {
				nd_socket_close(net_handle->fd);
				net_handle->fd = 0;
			}
		}

	}
	LEAVE_FUNC();
	return -1;
}

/* reset connector
 * 关闭网络连接并重新初始化连接状态,但保留用户设置信息(消息处理函数,加密密钥)
 */
int nd_connector_reset(nd_handle net_handle) 
{
	ENTER_FUNC()
	nd_assert(net_handle) ;
	//nd_connector_close( net_handle, 0 ) ;

	if(net_handle->type==NDHANDLE_TCPNODE){
		nd_tcpnode_reset((struct nd_tcp_node *)net_handle) ;
	}
	else if(net_handle->type==NDHANDLE_UDPNODE){
		nd_udtnode_reset((nd_udt_node*)net_handle) ;
	}

	LEAVE_FUNC();
	return 0 ;

}

ndsocket_t nd_connector_fd(nd_handle net_handle)
{
	nd_assert(net_handle) ;
	return  ((struct nd_tcp_node*)net_handle)->fd ;
	
}

/* 销毁连接器
 * 通过nd_object_destroy调用此函数
 */
int _connector_destroy(nd_handle net_handle, int force) 
{
	int ret = -1;
	ENTER_FUNC()
	nd_assert(net_handle) ;
	//nd_msgtable_destroy(net_handle) ;
	if(net_handle->type==NDHANDLE_TCPNODE){
		struct nd_tcp_node *socket_addr = (struct nd_tcp_node*)net_handle ;
		
		nd_tcpnode_close(socket_addr,force);
		nd_msgtable_destroy(net_handle,0) ;
		nd_tcpnode_deinit((struct nd_tcp_node*)net_handle) ;
		ret = 0 ;
	}
	else if(net_handle->type==NDHANDLE_UDPNODE) {
		if(net_handle->protocol==PROTOCOL_UDT) {
			nd_udt_node *socket_addr = (nd_udt_node*)net_handle ;

			if(socket_addr->fd>0) {
				udt_close(socket_addr,1);
				net_release_sendlock(net_handle) ;		//client socket need send lock
				_deinit_udt_socket(socket_addr) ;		//client socket need send lock
			}
		}
		else {
			if (net_handle->fd) {
				nd_socket_close(net_handle->fd);
				net_handle->fd = 0;
			}
		}

		nd_msgtable_destroy(net_handle,0) ;
		ret = 0;
		
	}
	LEAVE_FUNC();
	return ret;
}


//data handle function
static int fetch_udp_message(nd_handle  node, nd_packhdr_t *msg , nd_handle listener)
{
	ENTER_FUNC() 
	nd_packhdr_t *buf = (nd_packhdr_t*) node->user_data ;
	if(nd_pack_len(buf) > 0) {
		//already read data 
		LEAVE_FUNC();
		return 0 ;
	}
	else if(nd_pack_len(msg)>=ND_PACKET_DATA_SIZE) {
		LEAVE_FUNC();
		return -1;
	}
	memcpy(buf, msg, nd_pack_len(msg)) ;
	LEAVE_FUNC();
	return nd_pack_len(msg) ;
}


/* 等待网络消息,功能同nd_connect_waitmsg,
 * 使用这个函数的好处是直接把消息放到msgbuf中,
 * 无需再使用nd_connect_getmsg和nd_connect_delmsg
 * 出错时返回-1 网络需要关闭
 * 否则返回等待来的数据长度(0表示没有数据)
 */
int nd_connector_waitmsg(nd_netui_handle net_handle, nd_packetbuf_t *msgbuf, ndtime_t tmout)
{
	ENTER_FUNC()
	int ret =0;

	if(!nd_connector_valid(net_handle)|| nd_tryto_clear_err(net_handle)) {
		LEAVE_FUNC();
		return -1;
	}

	ret =nd_net_fetch_msg(net_handle, (nd_packhdr_t *)msgbuf) ;
	if(ret>0){
		LEAVE_FUNC();
		return ret;
	}

	if(net_handle->type==NDHANDLE_TCPNODE){
		ndtime_t waittime = 0;
		struct nd_tcp_node *socket_node = (struct nd_tcp_node*)net_handle ;

TCP_REWAIT:
		if (tmout > 1000) {
			waittime = 1000 ;
		}
		else {
			waittime = tmout ;
		}
		
		ret = tcpnode_wait_msg(socket_node, waittime) ;
		if(ret <= 0) {
			TCPNODE_TRY_CALLBACK_WRITE(socket_node) ;
			if(socket_node->update_entry((nd_handle)socket_node)==-1) {
				LEAVE_FUNC();
				return -1;
			}

			tmout -= waittime ;
			if (tmout > 0)
				goto TCP_REWAIT ;

			LEAVE_FUNC();
			return ret ;
		}
		ret =nd_net_fetch_msg(net_handle, (nd_packhdr_t *)msgbuf) ; 

		TCPNODE_TRY_CALLBACK_WRITE(socket_node) ;
		if(socket_node->update_entry((nd_handle)socket_node)==-1) {
			ret = -1;
		}
	}
	else {
		ndtime_t now = nd_time() ;
		nd_udp_node *socket_node = (nd_udp_node *)net_handle ;
		void *param_old = socket_node->user_data ;
		net_msg_entry data_func = socket_node->msg_entry ;
		char read_buf[ND_UDP_PACKET_SIZE] ;

		if(-1==socket_node->update_entry((nd_handle)socket_node) ){
			LEAVE_FUNC();
			return -1 ;	
		}

		socket_node->user_data = (void*)msgbuf ;
		socket_node->msg_entry  = fetch_udp_message ;

RE_WAIT:
		ret = socket_node->sock_read((nd_handle)socket_node, read_buf,sizeof(read_buf),tmout)  ;
		if(ret > 0 ) {
			int v ;
			nd_pack_len(&msgbuf->hdr) = 0 ;

			v = nd_udp_parse((nd_handle)socket_node, read_buf, ret) ;
			if(-1==v) {
				socket_node->user_data = param_old;
				socket_node->msg_entry  = data_func ;
				LEAVE_FUNC();
				return -1 ;
			} 
			socket_node->update_entry((nd_handle)socket_node);
			
			ret = nd_pack_len(&msgbuf->hdr);
			if(0==ret) {
				tmout -= nd_time() - now ;
				if((int)tmout > 0) {
					goto RE_WAIT ;
				}
			}
		}
		socket_node->user_data = param_old;
		socket_node->msg_entry  = data_func ;

	}
	/*
	if(ret>0 ) {
		nd_assert(ret == nd_pack_len(&msgbuf->hdr)) ;
		if(msgbuf->hdr.encrypt) {
			int new_len ;
			new_len = nd_packet_decrypt(net_handle, msgbuf) ;
			if(new_len==0) {
				net_handle->myerrno = NDERR_BADPACKET ;
				LEAVE_FUNC();
				return -1;
			}
			nd_pack_len(&msgbuf->hdr)  = (NDUINT16)new_len ;			
		}
	}*/
	LEAVE_FUNC();
	return ret ;

}

//设置数据处理完毕
//@size 被处理的数据长度
int nd_connector_handled_data(nd_netui_handle net_handle, size_t size) 
{
	
	struct nd_tcp_node *socket_addr = (struct nd_tcp_node*)net_handle ;

	nd_assert(net_handle) ;
	ndlbuf_sub_data(&(socket_addr->recv_buffer),size) ;
	return 0 ;
	
}

//data handle function
static int fetch_udp_data(nd_netui_handle node,void *data , size_t len,nd_handle listen_h) 
{
	size_t buf_size = *((size_t*) node->user_data) ;
	buf_size = min(len, buf_size) ;
	if(buf_size==0) {
		return 0 ;
	}
	memcpy(node->user_data, data, buf_size) ;
	node->user_data =(void*) buf_size ;
	return (int) buf_size ;
}

//
int nd_connector_raw_waitdata(nd_netui_handle net_handle, void *buf, size_t size, ndtime_t timeout) 
{
	ENTER_FUNC()
	int ret = 0;
	if(!nd_connector_valid(net_handle)|| nd_tryto_clear_err(net_handle)) {
		LEAVE_FUNC();
		return -1;
	}

	if(net_handle->type==NDHANDLE_TCPNODE){
		struct nd_tcp_node *socket_node = (struct nd_tcp_node*)net_handle ;
		
		if (timeout)	{
			if(nd_socket_wait_read(socket_node->fd, timeout) <= 0) {
				socket_node->myerrno = NDERR_WUOLD_BLOCK ;
				LEAVE_FUNC();
				return -1 ;
			}

		}		
		ret = nd_socket_tcp_read(socket_node->fd, buf, size) ;
		if(ret==0){
			LEAVE_FUNC();
			return 0;
		}
		if (ret== -1 ){
			if (nd_last_errno() == ESOCKETTIMEOUT)	{
				socket_node->myerrno= NDERR_WUOLD_BLOCK ;
			}
			else {
				socket_node->myerrno= NDERR_BADSOCKET ;
			}
		}
		LEAVE_FUNC();
		return ret ;
	}
	else {

		nd_udp_node *socket_node = (nd_udp_node *)net_handle ;

		char read_buf[ND_UDP_PACKET_SIZE] ;

		ret = socket_node->sock_read((nd_handle)socket_node, read_buf,sizeof(read_buf),timeout)  ;
		if(ret > 0 ) {
			int v ;
			void *param_old = socket_node->user_data ;
			data_in_entry data_func = socket_node->data_entry ;

			*((size_t *)buf) = size ;
			socket_node->user_data = buf ;
			socket_node->data_entry  = fetch_udp_data ;

			v = nd_udp_parse((nd_handle)socket_node, read_buf, ret)  ;

			ret = (int)socket_node->user_data;
			socket_node->user_data = param_old;
			socket_node->data_entry  = data_func ;

			if (socket_node->update_entry)
				socket_node->update_entry((nd_handle)socket_node);

			if(-1==v) {
				LEAVE_FUNC();
				return -1 ;
			} 
		}
	}
	LEAVE_FUNC();
	return ret ;
}

void nd_net_set_crypt(nd_netcrypt encrypt_func, nd_netcrypt decrypt_func,int crypt_unit) 
{
	ENTER_FUNC() 
	__net_encrypt = encrypt_func ;
	__net_decrypt = decrypt_func ;
	_crypt_unit_len = crypt_unit ;
	LEAVE_FUNC();
}

/*加密数据包,返回加密后的实际长度,并且修改了封包的实际长度*/
int nd_packet_encrypt(nd_netui_handle net_handle, nd_packetbuf_t *msgbuf)
{
	ENTER_FUNC() 
	int datalen ;
	nd_cryptkey *pcrypt_key ;

	nd_assert(net_handle) ;
	nd_assert(msgbuf);

	datalen = (int) nd_pack_len(&msgbuf->hdr) - ND_PACKET_HDR_SIZE;	
	if(datalen<=0 || datalen> (ND_PACKET_DATA_SIZE - _crypt_unit_len)) {
		LEAVE_FUNC();
		return 0;
	}
	//get net handle
	pcrypt_key = &(net_handle->crypt_key );
	
	//crypt 
	if(__net_encrypt && is_valid_crypt(pcrypt_key)) {

		int new_len  ;
		
		new_len = __net_encrypt(msgbuf->data, datalen, pcrypt_key->key) ;
		if(0==new_len) {
			LEAVE_FUNC();
			return 0 ;
		}

		msgbuf->hdr.encrypt = 1 ;
		if(new_len> datalen) {
			msgbuf->hdr.stuff =1 ;
			msgbuf->hdr.stuff_len = (new_len -datalen) ; 
			msgbuf->hdr.length += msgbuf->hdr.stuff_len ;
		}
		LEAVE_FUNC();
		return new_len ;
	}
	LEAVE_FUNC();
	return 0 ;
}

/*解密数据包,返回解密后的实际长度,但是不修改封包的实际长度*/
int nd_packet_decrypt(nd_netui_handle net_handle, nd_packetbuf_t *msgbuf)
{
	ENTER_FUNC()
	int datalen ;
	nd_cryptkey *pcrypt_key ;

	nd_assert(net_handle) ;
	nd_assert(msgbuf);
	nd_assert(msgbuf->hdr.encrypt);

	datalen = (int) nd_pack_len(&msgbuf->hdr) - ND_PACKET_HDR_SIZE;	
	if(datalen<=0 || datalen> (ND_PACKET_DATA_SIZE - _crypt_unit_len)){
		LEAVE_FUNC();
		return 0;
	}
	//get net handle
	pcrypt_key = &(net_handle->crypt_key );
	//decrypt 
	if(__net_decrypt && is_valid_crypt(pcrypt_key)) {

		int new_len = __net_decrypt(msgbuf->data, datalen, pcrypt_key->key) ;
		if(new_len <= 0 || new_len!=datalen) {
			char buf[20] ;
			SOCKADDR_IN *addr =& (net_handle->remote_addr );
			nd_logdebug("[%s] send data error :unknow crypt data\n" AND nd_inet_ntoa( addr->sin_addr.s_addr, buf )) ;
			LEAVE_FUNC();
			return 0 ;
		}
		msgbuf->hdr.encrypt = 0 ;

		if(msgbuf->hdr.stuff) {
			LEAVE_FUNC();
			nd_assert(msgbuf->hdr.stuff_len==msgbuf->data[datalen-1]) ;
			if(nd_pack_len(&msgbuf->hdr) > msgbuf->data[datalen-1]) 
				return (nd_pack_len(&msgbuf->hdr) - msgbuf->data[datalen-1]) ;
			else 
				return 0;
		}
	}
	LEAVE_FUNC();
	return nd_pack_len(&msgbuf->hdr) ;
}

size_t nd_net_getpack_size(nd_handle  handle, void *data) 
{
    NDUINT16 len = ntohs (nd_pack_len((nd_packhdr_t*)data) ) ;
	return (size_t) len;
}

int nd_net_set_packet_minsize(int minsize)
{
	int ret = _min_packet_len;
	_min_packet_len = minsize ;
	return ret;
}


int nd_net_sysmsg_hander(nd_netui_handle node, nd_sysresv_pack_t *pack)
{
	NDUINT16 cksum = pack->checksum ;
	
	nd_packet_ntoh(&pack->hdr) ;
	
	pack->checksum = 0 ;
	pack->checksum = nd_checksum((NDUINT16 *)pack, pack->hdr.length ) ;
	if (cksum != pack->checksum){
		node->myerrno= NDERR_BADPACKET ; 
		nd_logerror("nd-system-msg error checkSum error send=%x, calc=%x\n", cksum,pack->checksum ) ;
		return -1 ;
	}
	//msg handle
	if (pack->msgid == ERSV_ALIVE_ID) {
		if (pack->hdr.length != sizeof(nd_sysresv_pack_t) || pack->hdr.stuff_len != 5 ){
			node->myerrno= NDERR_BADPACKET ; 
			nd_logerror("nd-system-msg input data error \n" ) ;
			return -1 ;
		}
		else {			
			nd_sysresv_pack_t alive ;
			nd_make_alive_pack(&alive) ;
			nd_connector_send(node, &alive.hdr, ESF_URGENCY) ;	
		}
		
	}
	else if(ERSV_VERSION_ERROR==pack->msgid) {
		nd_logerror("nd-system-msg message id error \n" ) ;
		node->myerrno = NDERR_VERSION ;
		return -1;
	}
	return 0;	
}

int nd_net_message_version_error(nd_netui_handle node)
{
	nd_sysresv_pack_t packdata ;
	nd_sysresv_pack_t *pack = &packdata ;
	nd_hdr_init(&pack->hdr) ;
	pack->hdr.length = sizeof(nd_sysresv_pack_t) ;
	pack->hdr.ndsys_msg = 1;
	pack->hdr.stuff_len = 5 ;
	pack->msgid = ERSV_VERSION_ERROR ;
	pack->checksum = 0;
	pack->checksum = nd_checksum((NDUINT16 *)pack,sizeof(nd_sysresv_pack_t) ) ;
	nd_connector_send(node, &pack->hdr, ESF_URGENCY) ;

	node->myerrno = NDERR_VERSION ;

	return 0;
}

#undef  ND_IMPLEMENT_HANDLE
