/* file readcfg.h
 * header file of read config from xml 
 *
 * create by duan 
 * 2010-12-29
 */

#ifndef _READCFG_H_
#define _READCFG_H_

#include "nd_common/nd_export_def.h"

#define ND_HOST_NAME_SIZE 256 
//#define ND_FILE_PATH_SIZE 256
#define ND_IP_TEXT_SIZE 64
#define ND_CONNECT_OTHER_HOSTR_NUM 8
#define ND_DOMAIN_SIZE 64
struct listen_config
{
	int port ;
	int is_ipv6;
	int max_connect ;
	int connected_tmout ; //connect timeout (s)
    int empty_close_tmout;
    int closed_unknown ;
    int cloase_unauthorize ;
	int thread_pool_num;
	char bind_ip[ND_IP_TEXT_SIZE] ;
	char listen_name[32] ;
};

//connection info
struct connect_config
{
	int port ;
	int tmout ;
	char protocol_name[32] ;
	char host[ND_HOST_NAME_SIZE];
	struct nd_proxy_info proxy_info ;
	char connector_name[32] ;
};

struct nd_db_config
{
	int port;
	int special_read_port;
	char db_host[ND_HOST_NAME_SIZE] ;
	char db_database[64] ;
	char db_user[32] ;
	char db_password[32] ;
};

//instant config info
struct instance_config
{
	NDUINT8 open_dump ;				//is open dump
	NDUINT8 single_thread;			//is single thread 
	NDUINT8 log_filename_nodate;
	NDUINT32 log_file_size ;
	char inet_ip[ND_IP_TEXT_SIZE];
	char callstack_file[256] ;
	char log_file[256] ;
	char data_dir[256] ;
	char domain_name[ND_DOMAIN_SIZE];
	
	struct connect_config connectors[ND_CONNECT_OTHER_HOSTR_NUM] ;
};

#define MAX_RELIABLE_HOST 8
//server listen- instant config 
struct server_config
{
	NDUINT8 m_un_develop;	//is develop version 
	NDUINT8 reliable_num;
	struct listen_config   l_cfg ;
	struct instance_config i_cfg ;

	ndip_t reliable_hosts[MAX_RELIABLE_HOST] ;
	NDUINT32 reliable_ipmask[MAX_RELIABLE_HOST] ;
};
ND_APPLIB_API int read_base_port(ndxml *xmlroot) ;
ND_APPLIB_API int read_instance_info(ndxml *xmlroot, struct instance_config *icfg);
ND_APPLIB_API int read_connect_cfg(ndxml *xmlroot, int base_port, struct connect_config *ccfg);
ND_APPLIB_API int read_listen_cfg(ndxml *xmlroot, int base_port, struct listen_config *lcfg);
ND_APPLIB_API int read_config(ndxml *xmlroot, const char *name, struct server_config *scfg) ;
ND_APPLIB_API int read_dbconfig(const char *fileName, const char *dbCfgname ,struct nd_db_config *db_cfg) ;
#endif
