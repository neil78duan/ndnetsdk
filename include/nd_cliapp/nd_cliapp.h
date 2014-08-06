/* file nd_cliapp.h
 * nd-engine client applition 

 * neil duan
 * 2008-6
 */

#ifndef _ND_CLIAPP_H_
#define _ND_CLIAPP_H_

#include "nd_common/nd_common.h"
#include "nd_net/nd_netlib.h"
#include "nd_crypt/nd_crypt.h"
#include "nd_app/app_msgid.h"

#ifdef __cplusplus
#include "nd_appcpp/nd_object.h"
#include "nd_appcpp/nd_msgpacket.h"
#include "nd_appcpp/nd_connector.h"
#else 
#endif 

struct connect_config
{
	int port ;
	char protocol_name[64] ;
	char host[256] ;
	struct nd_proxy_info proxy_info ;
	ndchar_t callstack_file[256] ;
};

CPPAPI  int read_config(char *file, struct connect_config *readcfg) ;

CPPAPI nd_handle get_connect_handle();

CPPAPI int nd_cliapp_init(int argc, char *argv[]);

CPPAPI int nd_cliapp_end(int force);
CPPAPI int start_encrypt(nd_handle connect_handle);

CPPAPI int end_session(nd_handle connect_handle) ;
CPPAPI int start_session(nd_handle connect_handle);

CPPAPI nd_handle create_connector() ;
CPPAPI void destroy_connect(nd_handle h_connect) ;
CPPAPI void install_msg(nd_handle connector) ;
CPPAPI struct connect_config * get_config_info() ;
#endif
