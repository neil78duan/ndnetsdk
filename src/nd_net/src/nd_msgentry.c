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
#define NET_MSG_NAME_SIZE 64
 
//��Ϣ��ں����ڵ�
struct msg_entry_node
{
	NDUINT32			level:16 ;	//Ȩ�޵ȼ�
	NDUINT32			sys_msg:1 ;	//�Ƿ���ϵͳ��Ϣ
    NDUINT32            is_log:1;   //log current message
	NDUINT32			is_script : 1;// handle message with script
	nd_usermsg_func		entry ;	//��ں���
	//char* name;
	char name[NET_MSG_NAME_SIZE];
};

/*����Ϣ���*/
struct sub_msgentry 
{
	struct msg_entry_node   msg_buf[SUB_MSG_NUM] ;
};

/* ��Ϣ����ṹ��ڽڵ� */
struct msgentry_root
{
	ND_OBJ_BASE;
	NDUINT16	main_num ;			//�������ٸ���Ϣ���
	NDUINT16	msgid_base ;		//����Ϣ����ʼ��ַ
	NDUINT32	msg_node_size;
	void		*script_engine;
	nd_msg_script_entry script_entry;
	nd_usermsg_func		def_entry ;	//Ĭ����ں���
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
		root->msg_node_size = sizeof(struct msg_entry_node);
		root->type = NDHANDLE_NETMSG ;
		root->size = (NDUINT32) size ;
		
		root->close_entry=(nd_close_callback)nd_msgtable_destroy ;
	}
	return root ;	
};

static void destroy_msgroot(struct msgentry_root *root)
{
	int i = 0;
	for (i = 0; i < root->main_num;i++) {
		int j = 0;
		struct sub_msgentry *psub = &root->sub_buf[i];
		for (j = 0; j < SUB_MSG_NUM; j++) {
			struct  msg_entry_node *node = &psub->msg_buf[j];
// 			if (node->name)	{
// 				free(node->name);
// 				node->name = 0;
// 			}
			if (node->is_script)	{
				free((void*)node->entry);
				node->entry = NULL;
			}
		}
	}
	free(root);
}
/* Ϊ���Ӿ��������Ϣ��ڱ�
 * @mainmsg_num ����Ϣ�ĸ���(�ж�������Ϣ
 * @base_msgid ����Ϣ��ʼ��
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
			destroy_msgroot(((struct nd_tcp_node*)handle)->msg_handle);
			((struct nd_tcp_node*)handle)->msg_handle = 0 ;
		}
	}
	else if(handle->type==NDHANDLE_UDPNODE) {
		if(((nd_udt_node*)handle)->msg_handle) {
			destroy_msgroot(((nd_udt_node*)handle)->msg_handle);
			((nd_udt_node*)handle)->msg_handle = 0 ;
		}
	}
	else {
		struct nd_srv_node* srv_node = (struct nd_srv_node* )handle ;
		if(srv_node->msg_handle) {
			destroy_msgroot(srv_node->msg_handle);
			srv_node->msg_handle = 0 ;
		}
	}
}


nd_handle nd_get_msg_hadle(nd_netui_handle handle)
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
        return NULL;
    }
    return (nd_handle) root_entry ;
}

int nd_msgentry_def_handler(nd_netui_handle handle, nd_usermsg_func func) 
{
	struct msgentry_root *root_entry = (struct msgentry_root *) nd_get_msg_hadle(handle);

	if(root_entry) {
		root_entry->def_entry = func ;
		return 0 ;
	}
	return -1;
}

static struct msg_entry_node *_nd_msgentry_get_node(nd_netui_handle handle, ndmsgid_t maxid, ndmsgid_t minid) 
{
	struct msgentry_root *root_entry = (struct msgentry_root *) nd_get_msg_hadle(handle);

	if(root_entry) {
		ndmsgid_t main_index =(ndmsgid_t) (maxid - root_entry->msgid_base );
		if(main_index >= root_entry->main_num ) {
			nd_logerror("MAIN MESSAGE ERROR input %d  limited %d\n"AND main_index AND root_entry->main_num ) ;
			return NULL ;
		}
		if(minid>=SUB_MSG_NUM ){
			nd_logerror("MIN MESSAGE ERROR input %d  limited %d\n"AND minid AND SUB_MSG_NUM ) ;
			return NULL;
		}
		return  &(root_entry->sub_buf[main_index].msg_buf[minid]) ;
	}
	return  NULL ;
}

nd_usermsg_func nd_msgentry_get_func(nd_netui_handle handle, ndmsgid_t maxid, ndmsgid_t minid)
{
	struct msg_entry_node * node ;
	ENTER_FUNC() ;
	
	node = _nd_msgentry_get_node(handle,   maxid,  minid) ;
	LEAVE_FUNC();
	if (node->is_script)	{
		return NULL;
	}
	
	return node ? node->entry : NULL;
}

const char * nd_msgentry_get_name(nd_netui_handle handle, ndmsgid_t maxid, ndmsgid_t minid)
{
#if 1
	struct msg_entry_node * node ;
	ENTER_FUNC() ;
	
	node = _nd_msgentry_get_node(handle,   maxid,  minid) ;
	LEAVE_FUNC();
	
	return node ? node->name : NULL;
#else
	return NULL ;
#endif
}

NDUINT32 nd_msgentry_get_id(nd_handle handle, const char *msgname)
{
	int i, x;
	struct msgentry_root *root_entry = nd_get_msg_hadle(handle);
	if (!root_entry){
		return (NDUINT32)-1;
	}
	if (!msgname || !msgname[0]){
		return (NDUINT32)-1;
	}

	for ( i = 0; i < root_entry->main_num; i++)	{
		for (x = 0; x < SUB_MSG_NUM; x++){
			struct msg_entry_node*node = &(root_entry->sub_buf[i].msg_buf[x]);
			if (!node->name || !node->name[0]){
				continue;
			}
			if (ndstricmp((char*)msgname, node->name)==0 ) {
				NDUINT32 maxid = root_entry->msgid_base + i;
				return ND_MAKE_DWORD(maxid, x);
			}
		}
	}

	return (NDUINT32)-1;
}


nd_usermsg_func nd_msgentry_get_def_func(nd_netui_handle handle) 
{
	struct msgentry_root *root_entry = (struct msgentry_root *) nd_get_msg_hadle( handle) ;
	return root_entry->def_entry ;
	
}
/*��handle���Ӿ���ϰ�װ��Ϣ������*/
int nd_msgentry_install(nd_netui_handle handle, nd_usermsg_func func, ndmsgid_t maxid, ndmsgid_t minid,int level, const char *name) 
{
	struct msg_entry_node * node ;
	int ret = -1;
	ENTER_FUNC() ;
	
	node = _nd_msgentry_get_node(handle,   maxid,  minid) ;
	if (node) {	
		if (node->is_script && node->entry)	{
			free(node->entry);
		}
		node->entry = func ;
		node->level = level ;
		node->is_script = 0;
#if 1
		if (name && name[0]) {
			int len = strlen(name) + 1;
			if (len > (int) sizeof(node->name)) {
				len = sizeof(node->name);
			}
			strncpy(node->name, name, len);
		}
#endif
		ret = 0 ;
	}
	else {
		nd_object_seterror(handle, NDERR_NOSOURCE) ;
	}
	
	LEAVE_FUNC();	
	return  ret ;

}

int nd_msgentry_script_install(nd_handle handle, const char*script, ndmsgid_t maxid, ndmsgid_t minid, int level)
{
	struct msg_entry_node * node;
	int ret = -1;
	ENTER_FUNC();
	node = _nd_msgentry_get_node(handle, maxid, minid);
	if (script && script[0] && node) {
		int len = strlen(script) + 1;
		if (node->is_script && node->entry)	{
			free(node->entry);
		}
		node->entry = (nd_usermsg_func)malloc(len);
		strncpy((char*)node->entry, script, len);
		node->level = level;
		node->is_script = 1;
		ret = 0;
	}
	else {
		nd_object_seterror(handle, NDERR_NOSOURCE);
	}

	LEAVE_FUNC();
	return  ret;
}

int nd_message_set_script_engine(nd_handle handle, void *script_engine, nd_msg_script_entry entry)
{
	struct msgentry_root *root_entry = nd_get_msg_hadle(handle);
	if (root_entry) {
		root_entry->script_engine = script_engine;
		root_entry->script_entry = entry;
		return 0;
	}
	return -1;
}

static int _call_message_func(struct msgentry_root *root,struct msg_entry_node * node, nd_usermsgbuf_t*msg, nd_netui_handle conn, nd_handle lh)
{
	int ret = 0;
	if (!node || !node->entry) {
		if (root->def_entry){
			ret = root->def_entry(conn, msg, lh);
		}
		else{
			nd_logmsg("received message (%d,%d) UNHANDLED\n" AND msg->msg_hdr.maxid AND msg->msg_hdr.minid);
		}
	}
	else {
		if (!node->is_script){
			ret = node->entry(conn, msg, lh);
		}
		else {
			ret = root->script_entry(root->script_engine, conn, msg, (const char*)node->entry);
		}
	}
	return ret;
}
int nd_translate_message(nd_netui_handle connect_handle, nd_packhdr_t *msg ,nd_handle listen_handle) 
{
	ENTER_FUNC()	
	
	int ret = 0 ;
	int data_len = nd_pack_len(msg);
	struct msgentry_root *root_entry= (struct msgentry_root *) connect_handle->msg_handle;
	nd_usermsghdr_t *usermsg = (nd_usermsghdr_t *) (msg) ;	
	//nd_usermsg_func func ;
	struct msg_entry_node * node;

	nd_netmsg_ntoh(usermsg);
	node = _nd_msgentry_get_node(connect_handle, usermsg->maxid, usermsg->minid);
	ret = _call_message_func(root_entry, node, (nd_usermsgbuf_t*)msg, connect_handle, listen_handle);
	
	LEAVE_FUNC();
	return  ret ==-1? -1: data_len ;
	
}


int nd_srv_translate_message( nd_netui_handle connect_handle, nd_packhdr_t *msg ,nd_handle listen_handle) 
{
	ENTER_FUNC() ;
	struct msg_entry_node * node ;
	
	int ret = 0;
	int data_len = nd_pack_len(msg);
	struct nd_srv_node* srv_node = (struct nd_srv_node* )listen_handle ;
	nd_usermsghdr_t *usermsg =  (nd_usermsghdr_t *) msg ;	
	struct msgentry_root *root_entry= (struct msgentry_root *) (srv_node->msg_handle ) ; 
	
	nd_assert(data_len > 0) ;
	nd_assert(msg) ;
	nd_assert(connect_handle) ;	
	nd_assert(srv_node) ;
	
	nd_netmsg_ntoh(usermsg) ;
	
	node = _nd_msgentry_get_node(listen_handle, usermsg->maxid,  usermsg->minid) ;
	if (!node || (!node->entry && !root_entry->def_entry) ){
		if (srv_node->unreg_msg_close) {
			nd_object_seterror(connect_handle, NDERR_UNHANDLED_MSG) ; 
			ret = srv_node->unreg_msg_close ? -1 : data_len ;
		}
		nd_logwarn("receive UNHANDLED message(%d , %d) \n", usermsg->maxid,  usermsg->minid) ;
	}
	else {		
		if( nd_connect_level_get(connect_handle) < node->level ||  (node->sys_msg && !msg->ndsys_msg) ){			
			if (srv_node->error_privilage_close) {
				nd_object_seterror(connect_handle, NDERR_NO_PRIVILAGE) ;
				ret = srv_node->error_privilage_close ? -1 : data_len ;
			}
			
			nd_logwarn("receive un-privilege message(%d , %d) \n", usermsg->maxid,  usermsg->minid) ;
		}		
		else {
			ret = _call_message_func(root_entry, node, (nd_usermsgbuf_t*)msg, connect_handle, listen_handle);
			//nd_usermsg_func func = node->entry ? node->entry : root_entry->def_entry ;
			//ret = func(connect_handle,(nd_usermsgbuf_t*)usermsg,listen_handle) ; 
			if (-1==ret) {
				nd_object_seterror(connect_handle, NDERR_USER) ;
			}
		}		
	}
#if defined(ND_OPEN_TRACE)
    if (node && node->is_log) {
        nd_logmsg("recvived message(%d, %d)\n", usermsg->maxid,  usermsg->minid) ;
    }
#endif
		
	LEAVE_FUNC();	
	return  ret==-1 ? -1 : data_len ;
	
}

NDUINT32 nd_connect_level_get(nd_netui_handle handle) 
{
	return handle->level ;
}

//Ȩ�޵ȼ�
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

int nd_message_set_log(nd_handle handle,  ndmsgid_t maxid, ndmsgid_t minid,int is_log)
{
    
    ENTER_FUNC() ;
    struct msg_entry_node * node ;
    
    int ret = -1;
    node = _nd_msgentry_get_node(handle, maxid, minid) ;
    if (node) {
        ret = node->is_log ? 1 :0 ;
        node->is_log = is_log ? 1 : 0 ;
    }
    
    LEAVE_FUNC();
    return ret ;
}

int nd_message_set_system(nd_netui_handle handle,  ndmsgid_t maxid, ndmsgid_t minid,int issystem) 
{
	struct msgentry_root *root_entry = nd_get_msg_hadle(handle);

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
