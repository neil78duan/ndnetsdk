/* file : nd_msgentry.c
 * define net message entry 
 *
 * author : neil
 * all right reserved by neil 2008 
 * luckduan@msn.com
 * 
 */
#define ND_IMPLEMENT_HANDLE
typedef struct netui_info *nd_handle;

#include "nd_net/nd_netlib.h"
 
//消息入口函数节点
struct msg_entry_node
{
	NDUINT32			level:16 ;	//权限等级
	NDUINT32			sys_msg:1 ;	//是否是系统消息
	nd_usermsg_func		entry ;	//入口函数
};

/*主消息结果*/
struct sub_msgentry 
{
	struct msg_entry_node   msg_buf[SUB_MSG_NUM] ;
};

/* 消息处理结构入口节点 */
struct msgentry_root
{
	ND_OBJ_BASE;
	int	main_num ;			//包含多少个消息类别
	int msgid_base ;		//主消息号起始地址
	nd_usermsg_func		def_entry ;	//默认入口函数
	struct sub_msgentry sub_buf[0] ;
};


int srv_default_handler(nd_handle session_handle , nd_usermsgbuf_t *msg);
int connector_default_handler(nd_handle session_handle , nd_usermsgbuf_t *msg,nd_handle hlisten);
int connector_default_handler1(NDUINT16 session_id , nd_usermsgbuf_t *msg,nd_handle hlisten);

static struct msgentry_root *create_msgroot(int max_mainmsg, int base) 
{
	size_t size ;
	struct msgentry_root * root ;

	size = sizeof(struct sub_msgentry ) * max_mainmsg + sizeof(struct msgentry_root) ;
	root = (struct msgentry_root *)malloc(size) ;
	if(root) {
		memset(root, 0, size) ;
		root->main_num = max_mainmsg ;	
		root->msgid_base = base ;
		root->type = NDHANDLE_NETMSG ;
		root->size = (NDUINT32) size ;
		root->close_entry=(nd_close_callback)nd_msgtable_destroy ;
	}
	return root ;	
};

/* 为连接句柄创建消息入口表
 * @mainmsg_num 主消息的个数(有多数类消息
 * @base_msgid 主消息开始号
 * return value : 0 success on error return -1
 */
int nd_msgtable_create(nd_handle handle, int mainmsg_num, int base_msgid) 
{
	void **p = NULL ;
	nd_assert(handle) ;
	
	if(mainmsg_num>MAX_MAIN_NUM || mainmsg_num <= 0) {
		return -1;
	}
	
	if(handle->type==NDHANDLE_TCPNODE){
		p= (void **) & (((struct nd_tcp_node*)handle)->msg_handle ) ; 
	}
	else if(handle->type==NDHANDLE_UDPNODE) {
		p = (void **) & (((nd_udt_node*)handle)->msg_handle ) ; 
	}
	else {
		p = (void **) & ((struct nd_srv_node*) handle)->msg_handle ;
	}
	if (*p){
		nd_msgtable_destroy(handle,0) ;
		*p = 0 ;
	}

	*p = create_msgroot(mainmsg_num, base_msgid)  ;
	if(*p) {
		return 0 ;
	}
	else {
		return -1;
	}
}



void nd_msgtable_destroy(nd_handle handle, int flag) 
{	
	nd_assert(handle) ;
		
	if(handle->type==NDHANDLE_TCPNODE){
		if(((struct nd_tcp_node*)handle)->msg_handle) {
			free(((struct nd_tcp_node*)handle)->msg_handle) ;
			((struct nd_tcp_node*)handle)->msg_handle = 0 ;
		}
	}
	else if(handle->type==NDHANDLE_UDPNODE) {
		if(((nd_udt_node*)handle)->msg_handle) {
			free(((nd_udt_node*)handle)->msg_handle) ;
			((nd_udt_node*)handle)->msg_handle = 0 ;
		}
	}
	else {

		struct nd_srv_node* srv_node = (struct nd_srv_node* )handle ;

		if(srv_node->msg_handle) {
			free(srv_node->msg_handle) ;
			srv_node->msg_handle = 0 ;

		}

	}

}


int nd_msgentry_def_handler(nd_netui_handle handle, nd_usermsg_func func) 
{
	struct msgentry_root *root_entry= NULL;
	nd_assert(handle) ;

	if(handle->type==NDHANDLE_TCPNODE){
		root_entry = (struct msgentry_root *) (((struct nd_tcp_node*)handle)->msg_handle ) ; 
	}
	else if(handle->type==NDHANDLE_UDPNODE){
		root_entry = (struct msgentry_root *) (((nd_udt_node*)handle)->msg_handle ) ; 
	}
	else if(handle->type==NDHANDLE_LISTEN){
		root_entry = (struct msgentry_root *) (((struct nd_srv_node* )handle )->msg_handle ) ; 
	}
	else {
		return -1;
	}
	if(root_entry) {
		root_entry->def_entry = func ;
		return 0 ;
	}
	return -1;
}

/*在handle连接句柄上安装消息处理函数*/
int nd_msgentry_install(nd_netui_handle handle, nd_usermsg_func func, ndmsgid_t maxid, ndmsgid_t minid,int level) 
{
	struct msgentry_root *root_entry= NULL;
	nd_assert(handle) ;
		
	if(handle->type==NDHANDLE_TCPNODE){
		root_entry = (struct msgentry_root *) (((struct nd_tcp_node*)handle)->msg_handle ) ; 
	}
	else if(handle->type==NDHANDLE_UDPNODE){
		root_entry = (struct msgentry_root *) (((nd_udt_node*)handle)->msg_handle ) ; 
	}
	else if(handle->type==NDHANDLE_LISTEN){
		root_entry = (struct msgentry_root *) (((struct nd_srv_node* )handle )->msg_handle ) ; 
	}
	else {
		return -1;
	}
	if(root_entry) {
		ndmsgid_t main_index =(ndmsgid_t) (maxid - root_entry->msgid_base );
		if(main_index >= root_entry->main_num ) {
			nd_logerror("MAIN MESSAGE ERROR input %d  limited %d\n"AND main_index AND root_entry->main_num ) ;
			return -1 ;
		}
		if(minid>=SUB_MSG_NUM ){
			nd_logerror("MIN MESSAGE ERROR input %d  limited %d\n"AND minid AND SUB_MSG_NUM ) ;
			return -1 ;
		}

		root_entry->sub_buf[main_index].msg_buf[minid].entry = func ;
		root_entry->sub_buf[main_index].msg_buf[minid].level = level ;
		return 0 ;
	}
	nd_object_seterror(handle, NDERR_NOSOURCE) ;
	return -1;
}

int nd_translate_message(nd_netui_handle connect_handle, nd_packhdr_t *msg ,nd_handle listen_handle) 
{
	ENTER_FUNC()
	int ret = 0 ;
	int data_len = nd_pack_len(msg);
	struct msgentry_root *root_entry= NULL;
	nd_usermsg_func  func = NULL ;
	nd_usermsghdr_t *usermsg =  (nd_usermsghdr_t *) msg ;

	nd_assert(msg) ;
	nd_assert(connect_handle) ;

	root_entry =(struct msgentry_root *) connect_handle->msg_handle ;

	if(root_entry) {
		ndmsgid_t main_index , minid;
		int level = (int) nd_connect_level_get(connect_handle);
			nd_netmsg_ntoh(usermsg) ;
		main_index = usermsg->maxid - root_entry->msgid_base;
		minid = usermsg->minid ;
		if(main_index >= root_entry->main_num || minid>=SUB_MSG_NUM){
			nd_object_seterror(connect_handle, NDERR_INVALID_INPUT) ;
			LEAVE_FUNC();
			return -1 ;
		}

		func = root_entry->sub_buf[main_index].msg_buf[minid].entry ;

		if(func)
			ret = func(connect_handle,(nd_usermsgbuf_t*)usermsg,NULL) ;

		else if(root_entry->def_entry){
			ret = root_entry->def_entry(connect_handle,(nd_usermsgbuf_t*)usermsg,NULL) ;
		}

		if(-1==ret) {
			nd_object_seterror(connect_handle, NDERR_USER) ;
		}
	}

	LEAVE_FUNC();
	return  data_len ;
}


int nd_srv_translate_message( nd_netui_handle connect_handle, nd_packhdr_t *msg ,nd_handle listen_handle) 
{
	ENTER_FUNC()
	int ret = 0 ;
	int data_len = nd_pack_len(msg);
	struct nd_srv_node* srv_node = (struct nd_srv_node* )listen_handle ;
	struct msgentry_root *root_entry= NULL;
	nd_usermsg_func  func = NULL ;
	nd_usermsghdr_t *usermsg =  (nd_usermsghdr_t *) msg ;

	nd_assert(data_len > 0) ;
	nd_assert(msg) ;
	nd_assert(connect_handle) ;
	
	nd_assert(srv_node) ;

	root_entry = (struct msgentry_root *) (srv_node->msg_handle ) ; 
		
	if(root_entry) {
		ndmsgid_t  main_index , minid;
		NDUINT16 level = (NDUINT16) nd_connect_level_get(connect_handle)
		nd_netmsg_ntoh(usermsg) ;
		main_index = usermsg->maxid - root_entry->msgid_base;
		minid = usermsg->minid ;
		if(main_index >= root_entry->main_num || minid>=SUB_MSG_NUM ){
			nd_logdebug(("received error message maxid =%d minid =%d\n") AND main_index AND minid) ;
			nd_object_seterror(connect_handle, NDERR_INVALID_INPUT) ;
			LEAVE_FUNC();
			return srv_node->unreg_msg_close ? -1 : data_len ;
		}

		if(level < (NDUINT16) root_entry->sub_buf[main_index].msg_buf[minid].level){
			nd_object_seterror(connect_handle, NDERR_NO_PRIVILAGE) ;
			nd_log_screen(("PRIVILAGE error message maxid =%d minid =%d\n") AND main_index AND minid) ;
			LEAVE_FUNC();
			return srv_node->error_privilage_close ? -1 : data_len ;
		}
		if (root_entry->sub_buf[main_index].msg_buf[minid].sys_msg){
			if (!msg->ndsys_msg){
				nd_log_screen(("STATE error message maxid =%d minid =%d is system message\n") AND main_index AND minid) ;
				LEAVE_FUNC();
				return srv_node->error_privilage_close ? -1 : data_len ;
			}
		}

		func = root_entry->sub_buf[main_index].msg_buf[minid].entry ;
		if(func) {
			ret = func(connect_handle,(nd_usermsgbuf_t*)usermsg,listen_handle) ;
		}

		else if(root_entry->def_entry){
			ret = root_entry->def_entry(connect_handle,(nd_usermsgbuf_t*)usermsg,listen_handle) ;
		}
		/*
#ifdef ND_OPEN_LOG_DEBUG
		else {
			char buf[20] ;
			SOCKADDR_IN *addr =& (connect_handle->remote_addr );
			nd_logdebug(("received message from [%s] maxid =%d minid =%d UNHANDLED\n") AND 
				nd_inet_ntoa( addr->sin_addr.s_addr, buf ) AND main_index AND minid) ;
			nd_object_seterror(connect_handle, NDERR_INVALID_INPUT) ;
			LEAVE_FUNC();
			return srv_node->unreg_msg_close ? -1 : data_len ;
		}
#endif
		*/
		if (ret == -1) {
			nd_object_seterror(connect_handle, NDERR_USER) ;
			nd_logwarn("run message (%d,%d)error connect would be close\n" AND   main_index AND minid);
			LEAVE_FUNC();
			return -1;
		}
	}

	LEAVE_FUNC();
	return  data_len ;
}

NDUINT32 nd_connect_level_get(nd_netui_handle handle) 
{
	return handle->level ;
}

//权限等级
void nd_connect_level_set(nd_netui_handle handle,NDUINT32 level) 
{
	handle->level = level ;
}


int connector_default_handler(nd_handle session_handle , nd_usermsgbuf_t *msg,nd_handle hlisten)
{
	nd_log_screen("received message maxid=%d minid=%d len=%d\n", 
		ND_USERMSG_MAXID(msg) ,ND_USERMSG_MINID(msg) , ND_USERMSG_LEN(msg) ) ;
	//nd_sessionmsg_send(session_handle,msg) ;
	return 0;
}

int connector_default_handler1(NDUINT16 session_id , nd_usermsgbuf_t *msg,nd_handle hlisten)
{
	nd_log_screen("received message maxid=%d minid=%d  len=%d\n", 
		ND_USERMSG_MAXID(msg) ,ND_USERMSG_MINID(msg) , ND_USERMSG_LEN(msg) ) ;
	//nd_sessionmsg_send(session_handle,msg) ;
	return 0;
}

int nd_message_set_system(nd_netui_handle handle,  ndmsgid_t maxid, ndmsgid_t minid,int issystem) 
{
	struct msgentry_root *root_entry= NULL;
	nd_assert(handle) ;

	if(handle->type==NDHANDLE_TCPNODE){
		root_entry = (struct msgentry_root *) (((struct nd_tcp_node*)handle)->msg_handle ) ; 
	}
	else if(handle->type==NDHANDLE_UDPNODE){
		root_entry = (struct msgentry_root *) (((nd_udt_node*)handle)->msg_handle ) ; 
	}
	else if(handle->type==NDHANDLE_LISTEN){
		root_entry = (struct msgentry_root *) (((struct nd_srv_node* )handle )->msg_handle ) ; 
	}
	else {
		return -1;
	}
	if(root_entry) {
		int i;
		ndmsgid_t main_index =(ndmsgid_t) (maxid - root_entry->msgid_base );
		if(main_index >= root_entry->main_num ) {
			nd_logerror("MAIN MESSAGE ERROR input %d  limited %d\n"AND main_index AND root_entry->main_num ) ;
			return -1 ;
		}
		if ((ndmsgid_t)-1==minid){
			for(i=0; i<SUB_MSG_NUM; i++)
				root_entry->sub_buf[main_index].msg_buf[i].sys_msg = issystem?1:0 ; 
		}

		else if(minid < SUB_MSG_NUM ){
			root_entry->sub_buf[main_index].msg_buf[minid].sys_msg = issystem?1:0;
		}
		return 0 ;
	}
	return -1;

}
#undef ND_IMPLEMENT_HANDLE
