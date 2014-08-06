/* file nd_app.h
 * nd engine server application interface
 *
 * neil duan
 * 2008-6
 */

#pragma warning(disable: 4819)

#ifndef _ND_APP_H_
#define _ND_APP_H_

#include "nd_common/nd_common.h"
#include "nd_net/nd_netlib.h"
#include "nd_crypt/nd_crypt.h"
#include "nd_srvcore/nd_srvlib.h"

#include "nd_app/nd_appdef.h"
#include "nd_app/app_msgid.h"


struct srv_config
{
	int port ;
	int max_connect ;
	char listen_name[32] ;
	ndchar_t callstack_file[256] ;
};

#define ND_CUR_VERSION 0x1


typedef int (*exit_app_func)(int flag) ;

CPPAPI ndchar_t *srv_getconfig() ;
//��ȡ������Ϣ
CPPAPI int read_config(char *file, struct srv_config *readcfg) ;
//
CPPAPI int init_server_app(struct srv_config *run_config) ;

CPPAPI void destroy_rsa_key();

CPPAPI ND_RSA_CONTEX  *get_rsa_contex();

CPPAPI char *get_pubkey_digest() ;

CPPAPI int start_server(int argc, char *argv[]);

CPPAPI int end_server(int force);

CPPAPI int wait_services() ;


CPPAPI nd_handle get_listen_handle() ;


CPPAPI int create_rsa_key() ;

CPPAPI nd_thsrvid_t get_timer_thid() ;

/* set player size*/
CPPAPI void set_session_size(size_t size) ;
CPPAPI struct cm_manager * get_cm_manager() ;

static __INLINE__ void nd_pause()
{
	getch() ;
	ndprintf("press ANY key to continue\n") ;
}

MSG_ENTRY_DECLARE(srv_echo_handler) ;
//extern int srv_echo_handler(nd_handle session_handle , nd_usermsgbuf_t *msg, nd_handle h_listen) ;
MSG_ENTRY_DECLARE(srv_broadcast_handler) ;
//extern int srv_broadcast_handler(nd_handle session_handle , nd_usermsgbuf_t *msg, nd_handle h_listen) ;

//CPPAPI int srv_echo_handler(nd_handle session_handle , nd_usermsgbuf_t *msg, nd_handle h_listen) ;

//CPPAPI int srv_broadcast_handler(nd_handle session_handle , nd_usermsgbuf_t *msg, nd_handle h_listen) ;

CPPAPI void install_start_session(nd_handle listen_handle);

#ifdef ND_UNIX
CPPAPI int installed_sig_handle(void);
CPPAPI int wait_signal_entry();
CPPAPI int block_signal(void) ;
char *get_signal_desc(int signo);
CPPAPI int unblock_signal(void) ;
CPPAPI int ignore_all_signal(void) ;
#else
CPPAPI BOOL WINAPI winclose_entry(DWORD dwevent);
#endif //_WINDOWS

//set user appliction exit function
CPPAPI void app_exit_entry(exit_app_func func) ;

CPPAPI void install_app_msg(nd_handle listen_handle) ;

#endif
