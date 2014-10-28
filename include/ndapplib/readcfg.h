/* file readcfg.h
 * header file of read config from xml 
 *
 * create by duan 
 * 2010-12-29
 */

#ifndef _READCFG_H_
#define _READCFG_H_

#define HOST_NAME_SIZE 256 
#define FILE_PATH_SIZE 256
struct listen_config
{
	int port ;
	int max_connect ;
	int connected_tmout ; //connect timeout (s)
    int empty_close_tmout;
    int closed_unknown ;
    int cloase_unauthorize ;
	char bind_ip[32] ;
	char listen_name[32] ;
};

//������Ϣ
struct connect_config
{
	int port ;
	char protocol_name[32] ;
	char host[HOST_NAME_SIZE] ;
	struct nd_proxy_info proxy_info ;
};

//ʵ��������Ϣ
struct instance_config
{
	int open_dump ;				//�Ƿ��dump
	int single_thread;			//�Ƿ�ʹ�õ��߳��߼�
	char inet_ip[HOST_NAME_SIZE] ;
	char callstack_file[256] ;
	char log_file[256] ;
	char data_dir[256] ;
};

#define MAX_RELIABLE_HOST 8
//������������Ϣ
struct server_config
{
	NDUINT8 m_un_develop;	//�Ƿ��ǿ����汾
	struct listen_config   l_cfg ;
	struct instance_config i_cfg ;

	ndip_t reliable_hosts[MAX_RELIABLE_HOST] ;
	ndip_t reliable_ipmask[MAX_RELIABLE_HOST] ;
};
// 
// struct connect_in_server
// {
// 	int port ;
// 	char host[HOST_NAME_SIZE] ;
// };

//��ȡ�˿ڻ���
int read_base_port(ndxml *xmlroot) ;
//��ȡʵ����Ϣ
int read_instance_info(ndxml *xmlroot, struct instance_config *icfg);
//��ȡ������Ϣ
int read_connect_cfg(ndxml *xmlroot, int base_port, struct connect_config *ccfg);
//��ȡ������Ϣ
int read_listen_cfg(ndxml *xmlroot, int base_port, struct listen_config *lcfg);

//��xml��ȡ������Ϣ
int read_config(ndxml *xmlroot, const char *name, struct server_config *scfg) ;
#endif
