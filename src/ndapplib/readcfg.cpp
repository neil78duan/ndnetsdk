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

//读取监听信息
int read_listen_cfg(ndxml *xmlroot, int base_port, struct listen_config *lcfg)
{	
	//read port
	XML_READ_SUB_INT(xmlroot,"port",lcfg->port) ;
	lcfg->port += base_port ;

	//read ip
	XML_READ_SUB_BUF(xmlroot,"bindip",lcfg->bind_ip) ;

	//read listen_mod
	XML_READ_SUB_BUF(xmlroot,"listen_mod",lcfg->listen_name) ;

	XML_READ_SUB_INT(xmlroot,"max_connect",lcfg->max_connect) ;

	XML_READ_SUB_INT(xmlroot,"connected_tmout",lcfg->connected_tmout) ;
	
	return 0 ;

}
//读取连接信息
int read_connect_cfg(ndxml *xmlroot, int base_port, struct connect_config *ccfg)
{
	ndxml *xml_sub ;
	//read port
	XML_READ_SUB_INT(xmlroot,"remote_port",ccfg->port) ;
	ccfg->port += base_port ;
	
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
	if (icfg->data_dir[0]){
		int len = (int) strlen(icfg->data_dir) ;
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
	if (!xml_sub){
		T_ERROR("read base port error") ;
	}
	return ndxml_getval_int(xml_sub) ;
};

int read_iplist(ndxml *xmlnode, ndip_t *ipbuf, int num )
{
	int real_num = 0;
	for (int i=0; i<ndxml_getsub_num(xmlnode) && real_num<num; i++){
		ndip_t ip =0;
		ndxml *xmlip = ndxml_refsubi(xmlnode,i) ;
		char *p = ndxml_getval(xmlip) ;
		if (p) {
			if(0!=ndstr_get_ip(p, &ip) ) {
				ip =nd_inet_aton(p) ;
			}
			if (ip){
				ipbuf[real_num++] = ip ;
			}
		}
	}
	return 0;

}

int read_config(ndxml *xmlroot, const char *name, struct server_config *scfg) 
{
	ndxml *xml_sub,*xml_listen ;
	
	int base_port = read_base_port(xmlroot) ;

	memset(scfg, 0, sizeof(scfg)) ;
	if (0==base_port) {
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

	xml_listen = ndxml_refsub(xml_sub,"reliable_host") ;
	if (xml_listen){
		read_iplist(xml_listen, scfg->reliable_hosts, MAX_RELIABLE_HOST ) ;
		for(int i=0; i<MAX_RELIABLE_HOST; i++) {
			union {
				ndip_t ip ;
				NDUINT8 buf[4] ;
			}readip,ipmask;

			readip.ip = scfg->reliable_hosts[i] ;
			if (readip.ip ==0){
				break ;
			}
			ipmask.ip = 0xffffffff;
			for (int x=0; x<4; x++)	{
				if (0xff== readip.buf[x]){
					ipmask.buf[x] = 0 ;
				}
			}
			scfg->reliable_ipmask[i] = ipmask.ip;
		}
		//get netmask
	}
	return 0 ;
}
