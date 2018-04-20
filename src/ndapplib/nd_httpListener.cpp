/* file nd_httpListener.cpp
 *
 * listener of http server
 *
 * create by duan 
 * 
 * 2018.4.10
 */
 
#include "ndapplib/nd_httpListener.h"

NDHttpListener::NDHttpListener(nd_fectory_base *sf ) : NDSafeListener(sf)
{
}

NDHttpListener::~NDHttpListener()
{
	Destroy(1);
}

void NDHttpListener::Destroy(int flag)
{
	m_entrys.clear();
}
bool NDHttpListener::installRequest_c(const char *pathName, http_reqeust_func func)
{
	if (!pathName || !*pathName) {
		return false;
	}
	m_entrys[pathName] = ndHttpHandler(func, NULL);
	return true;
}
bool NDHttpListener::installRequest_script(const char *pathName, const char *script)
{
	if (!pathName || !*pathName || !script) {
		nd_logerror("input pathname or script name error\n");
		return false;
	}
	m_entrys[pathName] = ndHttpHandler(NULL, script);
	return true;
}

int NDHttpListener::onRequest(const char *reqPath, NDHttpSession *session, const NDHttpRequest &request)
{
	requestEntry_map::iterator it = m_entrys.find(reqPath);
	if (it == m_entrys.end() ) {
		return 404;
	}

	if (it->second.c_func) {
		return it->second.c_func(session, request, this);
	}
	else if (it->second.script_name.size()) {
		//call script handler
		return onRequestScript(it->second.script_name.c_str(),session, request) ;
	}
	return 404;
}

int NDHttpListener::onRequestScript(const char* script, NDHttpSession *session, const NDHttpRequest &request)
{
	return 200;
}
////////////////////////////

//////////////////////////////////////////////////////////////////////////

static int _session_data_handler(nd_handle sessionHandler, void *data, size_t len, nd_handle listen_h)
{
	NDHttpSession *pSession = (NDHttpSession*)NDGetSession(sessionHandler);
	if (!pSession) {
		return -1;
	}
	return pSession->onDataRecv((char*)data, len,(NDHttpListener*) NDGetListener(listen_h));
}


NDHttpSession::NDHttpSession() : m_bLongConnect(false)
{
}

NDHttpSession ::~NDHttpSession()
{
}



int NDHttpSession::SendResponse(NDHttpResponse &response, const char *errorDesc)
{
	return _sendHttpResponse(GetHandle(), &response, errorDesc);
}


int NDHttpSession::sendErrorResponse(int errorCdoe, const char *desc)
{
	int len;
	char *p;
	char buf[0x10000];
	
	p = buf;

	if (desc && *desc) {
		len = snprintf(p, sizeof(buf), "HTTP/1.1 %d %s \r\n", errorCdoe, desc );
	}
	else {
		len = snprintf(p, sizeof(buf), "HTTP/1.1 %d errorCode=%d \r\n", errorCdoe, errorCdoe);
	}
	p += len;

	if (desc && *desc) {
		len = snprintf(p, sizeof(buf) - (p - buf), "Server:userDefine\r\nContent-Length:%d\r\nConnection: close\r\n\r\n", (int)strlen(desc));
		p += len;  
		len = snprintf(p, sizeof(buf) - (p - buf), "%s\r\n\r\n", desc);
	}
	else {
		len = snprintf(p, sizeof(buf) - (p - buf), "Server:userDefine\r\nContent-Length:0\r\nConnection: close\r\n\r\n" );
	}
	p += len;


	return nd_connector_raw_write(GetHandle(), buf, p - buf);
}

void NDHttpSession::OnCreate()
{
	nd_hook_data(GetHandle(), _session_data_handler);
}

int NDHttpSession::onDataRecv(char *buf, int size, NDHttpListener *pListener)
{
	ndprintf("%s\n", buf);
	m_request.InData(buf, size);

	if (m_request.CheckRecvOk()) {
		//setLongConnect();
		m_bLongConnect = m_request.isLongConnect();
		m_request.dump();
		m_request.m_userData = pListener;
		pListener->onRequest(m_request.getPath(), this, m_request);
		m_request.Reset();
		if (!m_bLongConnect) {
			return -1; //CLOSE after on request
		}
	}
	return size;
}
// 
// void NDHttpSession::setLongConnect()
// {
// 	const char *connSt = m_request.getHeader("Connection");
// 	if (connSt) {
// 		if (0 == ndstricmp(connSt, "Keep-Alive")) {
// 			m_bLongConnect = true;
// 		}
// 	}
// 
// }