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
	ND_TRACE_FUNC();
	if (!pathName || !*pathName) {
		return false;
	}
	m_entrys[pathName] = ndHttpHandler(func, NULL);
	return true;
}
bool NDHttpListener::installRequest_script(const char *pathName, const char *script)
{
	ND_TRACE_FUNC();
	if (!pathName || !*pathName || !script) {
		nd_logerror("input pathname or script name error\n");
		return false;
	}
	m_entrys[pathName] = ndHttpHandler(NULL, script);
	return true;
}

int NDHttpListener::onRequest(const char *reqPath, NDHttpSession *session, const NDHttpRequest &request)
{
	ND_TRACE_FUNC();
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
	ND_TRACE_FUNC();
	if (!data || len == 0) {
		return 0;
	}
	NDHttpSession *pSession = (NDHttpSession*)NDGetSession(sessionHandler);
	if (!pSession) {
		return -1;
	}
	return pSession->onDataRecv((char*)data, (int)len,(NDHttpListener*) NDGetListener(listen_h));
}


NDHttpSession::NDHttpSession() 
{
	m_closedTime = 0;
}


int NDHttpSession::UpdateSecond()
{
	ND_TRACE_FUNC();
	if (m_closedTime) {
		int interval = (int)( nd_time() - m_closedTime);
		if (interval >= 0) {
			Close(0);
			nd_logdebug("time out closed %d session\n", GetSessionID());
			return 0;
		}
	}
}

void NDHttpSession::setDelayClosed(bool bRightnow)
{
	ND_TRACE_FUNC();
	if (bRightnow) {
		m_closedTime = nd_time();
	}
	else {
		m_closedTime =nd_time() + getWaitTimeout() * 1000;
	}
}

int NDHttpSession::getWaitTimeout()
{
	ND_TRACE_FUNC();
	NDInstanceBase *inst = nd_get_appinst();
	if (inst) {
		nd_handle hlisten =inst->GetDeftListener()->GetHandle();
		if (hlisten) {
			return nd_listensrv_get_empty_conntimeout(hlisten);
		}
	}
	return 60;
}

NDHttpSession ::~NDHttpSession()
{
}



int NDHttpSession::SendResponse(NDHttpResponse &response, const char *errorDesc)
{
	ND_TRACE_FUNC();
	setDelayClosed(!response.isLongConnect());
	return _sendHttpResponse(GetHandle(), &response, errorDesc);
}

int NDHttpSession::sendBinaryData(NDHttpResponse &response, void *data, size_t datalen, const char*errorDesc)
{
	ND_TRACE_FUNC();
	int ret = 0;
	int len;
	char *p;
	char buf[0x10000];

	setDelayClosed(!response.isLongConnect()); 

	p = buf;
	len = snprintf(p, sizeof(buf), "HTTP/1.1 %d %s \r\n", response.getStatus(), errorDesc);
	p += len;

	len = (int)response.HeaderToBuf(p, sizeof(buf) - (p - buf));
	p += len;

	len = snprintf(p, sizeof(buf) - (p - buf), "Server:userDefine\r\nContent-Length:%lld\r\n\r\n\r\n", datalen);
	p += len;

	len = nd_connector_send_stream(GetHandle(), buf, p - buf, 0);
	if (len <= 0) {
		return len;
	}
	ret += len;

	if (datalen) {
		len = nd_connector_send_stream(GetHandle(), data, datalen, 0);
		if (len <= 0) {
			return len;
		}
		ret += len;
		nd_connector_send_stream(GetHandle(), (void*)"\r\n\r\n", 4, 0);
		ret += 4;
	}

	nd_tcpnode_flush_sendbuf_force((nd_netui_handle)GetHandle());
	return ret;
}


int NDHttpSession::sendErrorResponse(int errorCdoe, const char *desc)
{
	ND_TRACE_FUNC();
	int len;
	char *p;
	char buf[4096];
	
	setDelayClosed(true);

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
	len = nd_connector_send_stream(GetHandle(), buf, p - buf,0);

	nd_tcpnode_flush_sendbuf_force((nd_netui_handle)GetHandle());
	return len;
}

void NDHttpSession::OnCreate()
{
	ND_TRACE_FUNC();
	nd_hook_data(GetHandle(), _session_data_handler);
}


int NDHttpSession::onDataRecv(char *buf, int size, NDHttpListener *pListener)
{
	ND_TRACE_FUNC();
	ndprintf("%s\n", buf);
	m_request.InData(buf, size);
	SetPrivilege(EPL_LOGIN);

	if (m_request.CheckRecvOk()) {
		m_request.dump();
		m_request.m_userData = pListener;
		pListener->onRequest(m_request.getPath(), this, m_request);
		m_request.Reset();
	}
	return size;
}
