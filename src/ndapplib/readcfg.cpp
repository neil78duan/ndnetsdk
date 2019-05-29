/* file readcfg.cpp
 * read config from xml file
 *
 * 2010-12-29
 * create by duan
 */

#include "nd_net/nd_netlib.h"
#include "ndapplib/readcfg.h"

#define T_ERROR(msg) do {				\
	nd_msgbox( msg, "error" ) ;	\
	return -1 ;							\
} while(0)

#define  XML_READ_SUB_BUF(_xml,_subname, _buf)		\
	do 	{													\
		ndxml *_xml_sub = ndxml_refsub(_xml,_subname) ;		\
		_buf[0] = 0 ;										\
		if(_xml_sub) {										\
			ndxml_getval_buf(_xml_sub, _buf, sizeof(_buf)) ;\
		}													\
	} while (0)	

#define  XML_READ_SUB_INT(_xml,_subname, _val)			\
	do 	{												\
		ndxml *_xml_sub = ndxml_refsub(_xml,_subname) ;		\
		_val = 0 ;											\
		if(_xml_sub) {										\
			_val = ndxml_getval_int(_xml_sub) ;				\
		}													\
	} while (0)	

//read listener info
int read_listen_cfg(ndxml *xmlroot, int base_port, struct listen_config *lcfg)
{	
	//read port
	//XML_READ_SUB_INT(xmlroot,"port",lcfg->port) ;
	
	lcfg->port = 0;
	ndxml *xmlPort = ndxml_refsub(xmlroot, "port");
	if (xmlPort) {
		lcfg->port = ndxml_getval_int(xmlPort);
		const char *pUseBase = ndxml_getattr_val(xmlPort, "not_use_base");
		if (!pUseBase || *pUseBase == '0') {
			lcfg->port += base_port;
		}
	}
	

	XML_READ_SUB_INT(xmlroot, "is_ipv6", lcfg->is_ipv6);
	//read ip
	XML_READ_SUB_BUF(xmlroot,"bindip",lcfg->bind_ip) ;

	//read listen_mod
	XML_READ_SUB_BUF(xmlroot,"listen_mod",lcfg->listen_name) ;

	XML_READ_SUB_INT(xmlroot,"max_connect",lcfg->max_connect) ;

	XML_READ_SUB_INT(xmlroot,"connected_tmout",lcfg->connected_tmout) ;
	
    
    XML_READ_SUB_INT(xmlroot,"empty_connected_timeout",lcfg->empty_close_tmout) ;
    XML_READ_SUB_INT(xmlroot,"closed_unknown_msg",lcfg->closed_unknown) ;
    XML_READ_SUB_INT(xmlroot,"closed_unauthorize_msg",lcfg->cloase_unauthorize) ;

	XML_READ_SUB_INT(xmlroot,"listen_thread_number",lcfg->thread_pool_num) ;

	return 0 ;

}
//read connector info
int read_connect_cfg(ndxml *xmlroot, int base_port, struct connect_config *ccfg)
{
	ndxml *xml_sub ;
	//read port
	XML_READ_SUB_INT(xmlroot,"remote_port",ccfg->port) ;
	ccfg->port += base_port ;

	XML_READ_SUB_INT(xmlroot,"connected_tmout",ccfg->tmout) ;
	
	//read remote host
	XML_READ_SUB_BUF(xmlroot,"host",ccfg->host) ;

	//read listen_mod
	XML_READ_SUB_BUF(xmlroot,"connect_protocol",ccfg->protocol_name) ;




	//read proxy info 
	xml_sub = ndxml_refsub(xmlroot,"proxy") ;
	if(xml_sub) {
		XML_READ_SUB_INT(xml_sub,"proxy_type",ccfg->proxy_info.proxy_type) ;
		if (ccfg->proxy_info.proxy_type==0)
			return 0;
		XML_READ_SUB_INT(xml_sub,"proxy_port",ccfg->proxy_info.proxy_port) ;
		XML_READ_SUB_BUF(xml_sub,"proxy_host",ccfg->proxy_info.proxy_host) ;
		XML_READ_SUB_BUF(xml_sub,"proxy_user",ccfg->proxy_info.user) ;
		XML_READ_SUB_BUF(xml_sub,"proxy_password",ccfg->proxy_info.password) ;
		
	}
	return 0 ;
}

int read_instance_info(ndxml *xmlroot, struct instance_config *icfg)
{
	XML_READ_SUB_BUF(xmlroot,"inet_ip",icfg->inet_ip) ;
	XML_READ_SUB_BUF(xmlroot,"logfile",icfg->log_file) ;
	XML_READ_SUB_BUF(xmlroot,"callstack",icfg->callstack_file) ;
	XML_READ_SUB_INT(xmlroot,"outputdump", icfg->open_dump) ;
	XML_READ_SUB_INT(xmlroot,"single_thread", icfg->single_thread) ;
	XML_READ_SUB_BUF(xmlroot,"data_dir", icfg->data_dir) ;
	XML_READ_SUB_BUF(xmlroot,"domain_name", icfg->domain_name) ;

	XML_READ_SUB_INT(xmlroot,"logfilesize", icfg->log_file_size) ;
	XML_READ_SUB_INT(xmlroot, "logFileNoDate", icfg->log_filename_nodate);
	

	if (icfg->data_dir[0]){
		int len = (int) ndstrlen(icfg->data_dir) ;
		--len ;
		if (icfg->data_dir[len] != '\\' || icfg->data_dir[len] != '/'){
			icfg->data_dir[++len]= '/' ;
			icfg->data_dir[++len] = 0 ;
		}
	}
	return 0;
};

int read_base_port(ndxml *xmlroot)
{
	ndxml *xml_sub = ndxml_refsub(xmlroot,"base_port") ;
	if (xml_sub) {
		return ndxml_getval_int(xml_sub);
	}
	return 0;
};

int read_iplist(ndxml *xmlnode, ndip_t *ipbuf, int num )
{
	int real_num = 0;
	for (int i=0; i<ndxml_getsub_num(xmlnode) && real_num<num; i++){
		ndip_t ip = ND_IP_INIT;
		ndxml *xmlip = ndxml_refsubi(xmlnode,i) ;
		const char *p = ndxml_getval(xmlip) ;
		if (p) {
			if (0 == ndstricmp(p, "localhost")) {
				ip.ip = 0x0100007f;
			}
			else {
				if (ndstrchr(p,':')) {
					ip = nd_inet_aton(p);
				}
				else {
					if (0 != ndstr_get_ip(p, &ip.ip)) {
						ip = nd_inet_aton(p);
					}
				}
			}
			ipbuf[real_num++] = ip;
		}
	}
	return real_num;

}

int read_config(ndxml *xmlroot, const char *name, struct server_config *scfg) 
{
	ndxml *xml_sub,*xml_listen ;
	
	int base_port = read_base_port(xmlroot) ;

	memset(scfg, 0, sizeof(*scfg)) ;
	if (-1==base_port) {
		return -1;
	}
	
	xml_sub = ndxml_refsub(xmlroot,name) ;
	if (!xml_sub){
		T_ERROR("read base port error") ;
	}

	if(-1== read_instance_info(xml_sub, &scfg->i_cfg)) {
		return -1 ;
	}


	xml_listen = ndxml_refsub(xml_sub,"listen") ;
	if (!xml_listen){
		T_ERROR("read base port error") ;
	}	
	if(-1== read_listen_cfg(xml_listen,  base_port,&scfg->l_cfg) ) {
		return -1 ;
	}
	//

	scfg->reliable_num = 0;
	xml_listen = ndxml_refsub(xml_sub,"reliable_host") ;
	if (xml_listen){
		int count = read_iplist(xml_listen, scfg->reliable_hosts, MAX_RELIABLE_HOST ) ;
		for(int i=0; i<count; i++) {
			if (scfg->reliable_hosts[i].sin_family == AF_INET6) {
				scfg->reliable_num++;
				continue;
			}
			union {
				NDUINT32 ip ;
				NDUINT8 buf[4] ;
			}readip,ipmask;

			readip.ip = scfg->reliable_hosts[i].ip ;
			if (readip.ip ==0){
				continue ;
			}
			ipmask.ip = 0xffffffff;
			for (int x=0; x<4; x++)	{
				if (0xff== readip.buf[x]){
					ipmask.buf[x] = 0 ;
				}
			}
			scfg->reliable_ipmask[i] = ipmask.ip;
			scfg->reliable_num++;
		}
		//get netmask
	}
	//read connectors
	
	xml_listen = ndxml_refsub(xml_sub,"connectors") ;
	if (xml_listen){
		for (int i=0; i< ND_CONNECT_OTHER_HOSTR_NUM && i<ndxml_num(xml_listen); i++) {
			ndxml *pnode = ndxml_getnodei(xml_listen, i) ;
			
			if(0== read_connect_cfg(pnode,  base_port, &scfg->i_cfg.connectors[i]) ) {
				const char *pname = ndxml_getattr_val(pnode, "name") ;
				if (pname && pname[0]) {
					ndstrncpy(scfg->i_cfg.connectors[i].connector_name, pname,sizeof(scfg->i_cfg.connectors[i].connector_name)) ;
				}
				else {
					pname = ndxml_getname(pnode) ;
					ndstrncpy(scfg->i_cfg.connectors[i].connector_name, pname,sizeof(scfg->i_cfg.connectors[i].connector_name)) ;
				}
			}
		}
	}
	
	return 0 ;
}

int read_dbconfig(const char *fileName, const char *dbCfgname ,struct nd_db_config *db_cfg)
{
	//ND_TRACE_FUNC() ;
	
	int ret = -1;
	ndxml *xmlroot, *xmlsub, *xmlnode ;
	ndxml_root xmlfile;
	
#define GET_VAL(name)                       \
	xmlnode = ndxml_refsub(xmlsub, #name) ; \
	if (!xmlnode){                          \
		ret = -1;                               \
		goto READ_EXIT ;                        \
	}                                       \
	ndstrncpy(db_cfg->db_##name, ndxml_getval(xmlnode), sizeof(db_cfg->db_##name) )
	
	
	if ( 0 != ndxml_load(fileName , &xmlfile ) ){
		nd_logfatal("read file %s error %s\n", fileName, nd_last_error() ) ;
		return -1;
	}
	
	xmlroot = ndxml_getnode( &xmlfile, "root" ) ;
	if ( !xmlroot )   {
		goto READ_EXIT;
	}
	//read connect config
	xmlsub = ndxml_refsub( xmlroot, dbCfgname ) ;
	if ( !xmlsub )   {
		goto READ_EXIT;
	}
	
	GET_VAL( host ) ;
	GET_VAL( database ) ;
	GET_VAL( user ) ;
	GET_VAL( password) ;

	db_cfg->port = 0;
	xmlnode = ndxml_getnode(xmlsub, "port");
	if (xmlnode)	{
		db_cfg->port = ndxml_getval_int(xmlnode);
	}

	db_cfg->special_read_port = -1;
	xmlnode = ndxml_getnode(xmlsub, "read_port");
	if (xmlnode)	{
		db_cfg->special_read_port = ndxml_getval_int(xmlnode);
	}

	ret = 0 ;
	
	
READ_EXIT:
	ndxml_destroy( &xmlfile );
	return ret ;
}
