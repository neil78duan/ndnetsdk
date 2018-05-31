/* file readcfg.h
 * header file of read config from xml 
 *
 * create by duan 
 * 2010-12-29
 */

#ifndef _READCFG_H_
#define _READCFG_H_

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

//连接信息
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

//实例配置信息
struct instance_config
{
	NDUINT8 open_dump ;				//是否打开dump
	NDUINT8 single_thread;			//是否使用单线程逻辑
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
//服务器配置信息
struct server_config
{
	NDUINT8 m_un_develop;	//是否是开发版本
	NDUINT8 reliable_num;
	struct listen_config   l_cfg ;
	struct instance_config i_cfg ;

	ndip_t reliable_hosts[MAX_RELIABLE_HOST] ;
	ndip_t reliable_ipmask[MAX_RELIABLE_HOST] ;
};
// 
// struct connect_in_server
// {
// 	int port ;
// 	char host[ND_HOST_NAME_SIZE] ;
// };

//读取端口基数
int read_base_port(ndxml *xmlroot) ;
//读取实例信息
int read_instance_info(ndxml *xmlroot, struct instance_config *icfg);
//读取连接信息
int read_connect_cfg(ndxml *xmlroot, int base_port, struct connect_config *ccfg);
//读取监听信息
int read_listen_cfg(ndxml *xmlroot, int base_port, struct listen_config *lcfg);

//从xml读取配置信息
int read_config(ndxml *xmlroot, const char *name, struct server_config *scfg) ;

int read_dbconfig(const char *fileName, const char *dbCfgname ,struct nd_db_config *db_cfg) ;
#endif
