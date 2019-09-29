/* file ndhttpcli.cpp
 *
 * http client for get action
 *
 *create by duan 
 */

#include "ndcli/nd_iconn.h"
#include "ndapplib/httpParser.h"
#include "nd_ssl_socket.h"


#if defined(_MSC_VER) 

#pragma comment(lib,"libcrypto-41.lib")
#pragma comment(lib,"libssl-43.lib")
#endif 

//#include "nd_common/nd_common.h"
// 
// 
// static bool requestHttp(HttpConnector &conn, const char *host, int port, const char*path, const char*reqData)
// {
// 	if (-1 == conn.Open(host, port)) {
// 		nd_logerror("open %s:%d\n", host, port);
// 		return false;
// 	}
// 	NDHttpRequest req;
// 	if (reqData) {
// 		req.setBody(reqData);
// 	}
// 
// 	if (conn.SendRequest(req, host, port, path) < 0) {
// 		nd_logerror("send request error\n");
// 		return false;
// 	}
// 	return true;
// }
// 
// static bool httpGet(HttpConnector &conn,const char *url)
// {
// 	int port = 80;
// 	const char *p = url;
// 	char buf[1024];
// 	char * paim = (char*) ndstristr(p, "http://");
// 	if (paim) {
// 		p = paim + 7;
// 	}
// 	//get host 
// 	p = ndstr_nstr_end(p, buf, '/', sizeof(buf));
// 
// 	paim = ndstrchr(buf, ':');
// 	if (paim) {
// 		++paim;
// 		port = atoi(paim);
// 		*--paim = 0;
// 	}
// 
// 	if (!p || !*p) {
// 		return requestHttp(conn, buf, port, NULL, NULL);
// 	}
// 
// 	if (*p == '/') {
// 		++p;
// 	}
// 
// 	std::string host = buf;
// 	std::string path = "";
// 
// 	paim = (char*)ndstrchr(p, '?');
// 	if (paim ) {
// 		buf[0] = 0;
// 		p = ndstr_nstr_end(p, buf, '?', sizeof(buf));
// 		path = buf;
// 		if (*p && *p == '?') {
// 			++p;
// 		}
// 	}
// 	return requestHttp(conn, host.c_str(), port,path.c_str(), p);
//
class MyHttp : public HttpConnector
{
public:
	MyHttp() :HttpConnector()
	{

	}
	virtual ~MyHttp()
	{

	}

	virtual void OnInitilize()
	{
		if (getHttps())	{
			if (-1 == nd_ssl_connect(m_objhandle)) {
				nd_logerror("ssl_connect error\n");
			}
		}
	}
};
int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr,"usage: ndhttp_cli url\n");
		exit(1);
	}

	ndInitNet();
	nd_ssl_root_init();

	MyHttp httpConn;

	httpConn.Create("httpConnector");
	
	NDHttpGet(httpConn, argv[1]);

	do {
		httpConn.Update(50);
	} while (! httpConn.CheckResponsed() );

	httpConn.Close(0);
	httpConn.Destroy(0);

	nd_ssl_root_destroy();
	ndDeinitNet();
	getch();
	return 0;
}