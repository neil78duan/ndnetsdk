/* file nd_ssl_socket.h
 *
 * implement ssl api 
 *
 * create by duan 
 *
 * 2019-9
 */
 

#ifndef _ND_SSL_SOCKET_H_
#define _ND_SSL_SOCKET_H_

#include "nd_common/nd_common.h"
////////////////
CPPAPI int nd_ssl_root_init();
CPPAPI void nd_ssl_root_destroy();
CPPAPI int nd_ssl_connect(nd_handle conn);
CPPAPI int nd_ssl_accept(nd_handle conn);
CPPAPI int nd_ssl_load_cert(const char*cert_file, const char *priv_key);

#endif 
