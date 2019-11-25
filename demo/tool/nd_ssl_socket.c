/* file nd_ssl_socket.c
 *
 * implemention of ssl socket
 *
 * create by duan 
 * 2019.9.27
 */


#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#define ND_IMPLEMENT_HANDLE
typedef struct netui_info *nd_handle;
#include "nd_net/nd_netlib.h"
#include "nd_net/nd_netui.h"
#include "nd_ssl_socket.h"

static SSL_CTX *g_ssl_ctx;
void ShowCerts(SSL * ssl)
{
	X509 *cert;
	char *line;

	cert = SSL_get_peer_certificate(ssl);
	if (cert != NULL)
	{
		printf("certificate info:\n");
		line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
		printf("CERT: %s\n", line);
		free(line);
		line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
		printf("Publisher: %s\n", line);
		free(line);
		X509_free(cert);
	}
	else
		printf("No cert\n");
}
int nd_ssl_root_init()
{
	SSL_CTX *ctx;

	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
	ctx = SSL_CTX_new(SSLv23_client_method());
	if (ctx == NULL){
		ERR_print_errors_fp(stdout);
		return -1;
	}
	g_ssl_ctx = ctx;
	return 0;
}

void nd_ssl_root_destroy()
{
	if (g_ssl_ctx) {
		SSL_CTX_free(g_ssl_ctx);
	}
	g_ssl_ctx = 0;
}

int nd_ssl_load_cert(const char*cert_file, const char *priv_key)
{
	SSL_CTX *ctx = g_ssl_ctx;
	if (!SSL_CTX_use_certificate_file(ctx, cert_file, SSL_FILETYPE_PEM)){
		ERR_print_errors_fp(stdout);
		return -1;
	}
	if (!SSL_CTX_use_PrivateKey_file(ctx, priv_key, SSL_FILETYPE_PEM)){
		ERR_print_errors_fp(stdout);
		return -1;
	}
	if (!SSL_CTX_check_private_key(ctx)){
		ERR_print_errors_fp(stdout);
		return -1;
	}
	return 0;
}

static int nd_ssl_close(nd_handle conn, int flag)
{
	if (conn->ssl) {
		SSL_shutdown((SSL *)conn->ssl);
		SSL_free((SSL *)conn->ssl);
	}
	conn->ssl = 0;
	return nd_tcpnode_close((struct nd_tcp_node *)conn, flag);
}

static int nd_ssl_read(nd_handle node, void *buf, size_t len)
{
	int ret;
	ENTER_FUNC();
	nd_assert(node->ssl);
	ret = (int) SSL_read(node->ssl, buf, len);
	if (ret > 0) {
		node->recv_len += ret;
		node->last_recv = nd_time();
	}
	else if (-1 == ret) {
		node->sys_error = nd_socket_last_error();
		if (node->sys_error == ESOCKETTIMEOUT) {
			node->myerrno = NDERR_WOULD_BLOCK;
			ret = 0;
		}
		else {
			node->myerrno = NDERR_IO;
		}
	}
	else if (ret == 0) {
		node->myerrno = NDERR_CLOSED;
		ret = -1;
	}
	LEAVE_FUNC();
	return ret;
};
static int nd_ssl_write(nd_handle node, void *data, size_t len)
{
	ENTER_FUNC();
	int ret;
	nd_assert(node->ssl);
	ret = (int)SSL_write(node->ssl, data, len);
	if (ret > 0) {
		node->send_len += ret;
		node->last_push = nd_time();
	}
	else if (-1 == ret) {
		node->sys_error = nd_socket_last_error();
		if (node->sys_error == ESOCKETTIMEOUT) {
			node->myerrno = NDERR_WOULD_BLOCK;
			ret = 0;
		}
		else {
			node->myerrno = NDERR_IO;
		}
	}
	else if (ret == 0) {
		node->myerrno = NDERR_WOULD_BLOCK;
	}
	LEAVE_FUNC();
	return ret;
}

int nd_ssl_connect(nd_handle conn)
{
	SSL *ssl = SSL_new(g_ssl_ctx);

	nd_socket_nonblock(conn->fd, 0);
	SSL_set_fd(ssl, conn->fd);

	if (SSL_connect(ssl) == -1) {
		ERR_print_errors_fp(stderr);
		SSL_free(ssl);
		return -1;
	}
	conn->ssl = ssl;
	conn->sys_sock_read = nd_ssl_read;
	conn->sys_sock_write = nd_ssl_write;
	conn->sys_sock_close = nd_ssl_close;
	return 0;
}
int nd_ssl_accept(nd_handle conn)
{
	SSL *ssl = SSL_new(g_ssl_ctx);
	nd_socket_nonblock(conn->fd, 0);
	SSL_set_fd(ssl, conn->fd);
	if (SSL_accept(ssl) == -1) {
		ERR_print_errors_fp(stderr);
		SSL_free(ssl);
		return -1;
	}
	conn->ssl = ssl;
	conn->sys_sock_read = nd_ssl_read;
	conn->sys_sock_write = nd_ssl_write;
	conn->sys_sock_close = nd_ssl_close;
	return 0;
}
