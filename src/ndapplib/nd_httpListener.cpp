/* file nd_httpListener.cpp
 *
 * listener of http server
 *
 * create by duan 
 * 
 * 2018.4.10
 */
 
#include "ndapplib/nd_httpListener.h"

NDHttpListener::NDHttpListener(nd_fectory_base *sf ) : NDSafeListener(sf), m_cookieSessionIds(NULL), m_sessionAge(-1)
{
}

NDHttpListener::~NDHttpListener()
{
	Destroy(1);
}

void NDHttpListener::Destroy(int flag)
{
	ND_TRACE_FUNC();
	m_entrys.clear();
	delete m_cookieSessionIds;
	m_cookieSessionIds = 0;
}

void NDHttpListener::OnInitilize()
{
	ND_TRACE_FUNC();
	m_cookieSessionIds = new SessionIdMgr;
}

void NDHttpListener::updateSessionIds()
{
	ND_TRACE_FUNC();
	if (m_cookieSessionIds) {
		m_cookieSessionIds->Update();
	}
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
	NDHttpSession *pSession = dynamic_cast<NDHttpSession*>(NDObject::FromHandle(sessionHandler));
	if (!pSession) {
		return -1;
	}
	return pSession->onDataRecv((char*)data, (int)len,(NDHttpListener*) NDObject::FromHandle(listen_h));
}


std::string NDHttpSession::s_serverInfo = "UserDefined" ;

const char *NDHttpSession::getServerInfo()
{
	return s_serverInfo.c_str();
}
void NDHttpSession::setServerInfo(const char *info)
{
	s_serverInfo = info;
}

NDHttpSession::NDHttpSession() 
{
	m_closedTime = 0;
	m_sessionAge = -1;
	//m_trytoClose = false;
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
	return 0;
}

void NDHttpSession::setDelayClosed(bool bRightnow)
{
	ND_TRACE_FUNC();
	if (bRightnow) {
		m_closedTime = nd_time() + 5000;
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
			return nd_listener_get_empty_timeout(hlisten);
		}
	}
	return 60;
}

NDHttpSession ::~NDHttpSession()
{
}


int NDHttpSession::_getMyCookie(char *buf, size_t size)
{
	ND_TRACE_FUNC();
	int age = getSessionAge();
	if (age == -1) {
		return ndsnprintf(buf, size, "%s=%s;path=/", ND_DFT_SESSION_ID_NAME,
			m_cookieSessionId.c_str());
	}
	else {
		return ndsnprintf(buf, size, "%s=%s;Max-Age=%d;path=/", ND_DFT_SESSION_ID_NAME,
			m_cookieSessionId.c_str(), age);
	}
	
}

int NDHttpSession::SendResponse(NDHttpResponse &response, const char *errorDesc)
{
	ND_TRACE_FUNC();
	//setDelayClosed(!response.isLongConnect());
	sessionValInfo sInfo;
	if (sessionIdGetInfo(sInfo)) {
		char buf[4096];
		_getMyCookie(buf, sizeof(buf));
		response.addHeader("Set-Cookie", buf);
	}
	return _sendHttpResponse(GetHandle(), &response, errorDesc,getServerInfo());
}

int NDHttpSession::SendRedirect(const char *newUrl)
{
	ND_TRACE_FUNC();
	int len;
	//char *p;
	char cookie[1024];
	char buf[4096];
	if (!newUrl || !*newUrl) {
		return -1;
	}
	//setDelayClosed(true);
	
	sessionValInfo sInfo;
	if (sessionIdGetInfo(sInfo)){
		_getMyCookie(cookie, sizeof(cookie));

		len = ndsnprintf(buf, sizeof(buf), "HTTP/1.1 302 Found \r\n"
			"Content-Type:text/html;charset=UTF-8\r\nServer:%s\r\n"
			"Location:%s\r\n"
			"Set-Cookie:%s\r\n"
			"Content-Length:0\r\nConnection: close\r\n\r\n", getServerInfo(), newUrl, cookie);
	}
	else {
		len = ndsnprintf(buf, sizeof(buf), "HTTP/1.1 302 Found \r\n"
			"Content-Type:text/html;charset=UTF-8\r\nServer:%s\r\n"
			"Location:%s\r\n"
			"Content-Length:0\r\nConnection: close\r\n\r\n",  getServerInfo(),newUrl);
	}
	
	len = nd_connector_send_stream(GetHandle(), buf,len, 0);

	nd_tcpnode_flush_sendbuf_force((nd_netui_handle)GetHandle());
	return len;
}


int NDHttpSession::sendBinaryData(NDHttpResponse &response, void *data, size_t datalen, const char*errorDesc)
{
	ND_TRACE_FUNC();
	int ret = 0;
	int len;
	char *p;
	char buf[0x10000];

	//setDelayClosed(!response.isLongConnect()); 

	p = buf;
	len = ndsnprintf(p, sizeof(buf), "HTTP/1.1 %d %s \r\n", response.getStatus(), errorDesc);
	p += len;

	len = (int)response.HeaderToBuf(p, sizeof(buf) - (p - buf));
	p += len;

	len = ndsnprintf(p, sizeof(buf) - (p - buf),
					 "Server:%s\r\nContent-Length:%lld\r\n\r\n", getServerInfo(), (NDUINT64)datalen);
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
	}
	nd_connector_send_stream(GetHandle(), (void*)"\r\n\r\n", 4, 0);
	ret += 4;

	//nd_tcpnode_flush_sendbuf_force((nd_netui_handle)GetHandle());
	return ret;
}


int NDHttpSession::sendErrorResponse(int errorCdoe, const char *desc)
{
	ND_TRACE_FUNC();
	int len;
	char *p;
	char buf[4096];
	
	//setDelayClosed(true);

	p = buf;

	if (desc && *desc) {
		len = ndsnprintf(p, sizeof(buf), "HTTP/1.1 %d %s \r\n", errorCdoe, desc );
	}
	else {
		len = ndsnprintf(p, sizeof(buf), "HTTP/1.1 %d errorCode=%d \r\n", errorCdoe, errorCdoe);
	}
	p += len;

	if (desc && *desc) {		
		len = ndsnprintf(p, sizeof(buf) - (p - buf), "Content-Type:text/html;charset=UTF-8\r\nServer:%s\r\n"
			"Content-Length:%d\r\nConnection: close\r\n\r\n", getServerInfo(), (int)ndstrlen(desc));
		p += len;  
		len = ndsnprintf(p, sizeof(buf) - (p - buf), "%s\r\n\r\n", desc);
	}
	else {
		len = ndsnprintf(p, sizeof(buf) - (p - buf), "Server:%s\r\nContent-Length:0\r\nConnection: close\r\n\r\n" , getServerInfo());
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

	
	NDUINT32 val = 1;
	int size = sizeof(val);
	Ioctl(NDIOCTL_UNLIMITED_SEND_WNDSIZE, &val, &size);
	val = 1;
	size = sizeof(val);
	Ioctl(NDIOCTL_UNLIMITED_RECV_WNDSIZE, &val, &size);	
	nd_netobj_close_keep_alive((nd_netui_handle)GetHandle());
}


int NDHttpSession::onDataRecv(char *buf, int size, NDHttpListener *pListener)
{
	ND_TRACE_FUNC();
	//ndprintf("%s\n", buf);
	m_request.InData(buf, size);
	if (m_request.CheckParseError()) {
		nd_logerror("parse received data error\n");
		sendErrorResponse(501, "input error");
		return -1;
	}
	SetPrivilege(EPL_LOGIN);

	if (m_request.CheckRecvOk()) {
		//m_request.dump();
		m_request.m_userData = pListener;
		//try to parse session id 
		_preOnHandle();

		pListener->onRequest(m_request.getPath(), this, m_request);
		m_request.Reset();
	}
	return size;
}

void NDHttpSession::_preOnHandle()
{
	ND_TRACE_FUNC();
	const char *pSessionId = m_request.getHeader(ND_DFT_SESSION_ID_NAME);
	if (pSessionId) {
		sessionIdTrytoSet(pSessionId);
	}
	else {
		sessionIdVal::iterator it = m_request.m_cookies.find(ND_DFT_SESSION_ID_NAME);
		if (it != m_request.m_cookies.end()) {
			sessionIdTrytoSet(it->second.c_str());
		}
	}
	
}

// sessionId manage functions 

bool NDHttpSession::sessionIdTrytoSet(const char *clientSendSid)
{
	ND_TRACE_FUNC();
	SessionIdMgr *pMgr = _getSessoinIdMgr();
	if (!pMgr) {
		return false;
	}
	sessionValInfo vals;

	m_cookieSessionId = clientSendSid;
	if (pMgr->GetSessionIdVal(m_cookieSessionId, vals)) {
		return true;
	}
	m_cookieSessionId.clear();
	return false;
}

bool NDHttpSession::sessionIdCreate(int lifeOfSeconds , const char *path )
{
	ND_TRACE_FUNC();
	SessionIdMgr *pMgr = _getSessoinIdMgr();
	if (!pMgr) {
		return false;
	}
	m_cookieSessionId = pMgr->CreateSessionId(sessionIdVal(), lifeOfSeconds, path);
	return !m_cookieSessionId.empty();
}

sessionId_t NDHttpSession::sessionIdGet()
{
	return m_cookieSessionId;
}

bool NDHttpSession::sessionIdSetValue(const char *name, const char *value)
{
	ND_TRACE_FUNC();
	SessionIdMgr *pMgr = _getSessoinIdMgr();
	if (!pMgr) {
		return false;
	}

	if (m_cookieSessionId.empty()) {
		if (!name || !value) {
			return true;
		}
		sessionIdVal valMaps;
		valMaps[name] = value;
		m_cookieSessionId = pMgr->CreateSessionId(valMaps, SESSION_DEFAULT_TIMEOUT, SESSION_DEFAULT_PATH);
		if (m_cookieSessionId.empty()) {
			return false;
		}
	}

	return pMgr->SaveSessionIdValue(m_cookieSessionId, name, value);
}

std::string NDHttpSession::sessionIdGetValue(const char*name)
{
	ND_TRACE_FUNC();
	sessionValInfo info;
	bool ret = sessionIdGetInfo(info);
	if (!ret) {
		return std::string();
	}
	if (app_inst_time(NULL)>=info.invalidTm) {
		return std::string();
	}
	sessionIdVal::const_iterator it =info.val.find(name);
	if (it != info.val.end()) {
		return it->second;
	}
	return std::string();
}

bool NDHttpSession::sessionIdGetInfo(sessionValInfo &info)
{
	ND_TRACE_FUNC();
	if (m_cookieSessionId.empty()) {
		return false;
	}
	SessionIdMgr *pMgr = _getSessoinIdMgr();
	if (!pMgr) {
		return false;
	}

	return pMgr->GetSessionIdVal(m_cookieSessionId, info);
}

int NDHttpSession::getSessionAge()
{
	ND_TRACE_FUNC();
	int ret = m_sessionAge;
	if(ret ==0)	{
		NDHttpListener *pListener = dynamic_cast<NDHttpListener *> (GetParent());
		if (pListener) {
			ret = pListener->getSessionAge();
		}
	}
	return ret;
}

SessionIdMgr * NDHttpSession::_getSessoinIdMgr()
{
	ND_TRACE_FUNC();
	NDHttpListener *pListener = dynamic_cast<NDHttpListener *> (GetParent());
	if (!pListener) {
		return NULL;
	}
	return pListener->getSessionIdMgr();
}

///////////////////////////////////////
//li
SessionIdMgr::SessionIdMgr()
{
	ND_TRACE_FUNC();
	nd_mutex_init(&m_lock);
}

SessionIdMgr::~SessionIdMgr()
{
	ND_TRACE_FUNC();
	nd_mutex_destroy(&m_lock);
}

void SessionIdMgr::Update()
{
	ND_TRACE_FUNC();
	LockHelper tmplock(&m_lock);

	time_t now = app_inst_time(NULL);

	for (sessionData_t::iterator it = m_data.begin(); it != m_data.end(); ) {
		if (now >= it->second.invalidTm) {
			nd_logdebug("http sessionid %d is timeout \n", it->first.c_str()) ;
			m_data.erase(it++);
		}
		else {
			++it;
		}
	}

}

sessionId_t SessionIdMgr::CreateSessionId(const sessionIdVal &val, int lifeOfSeconds, const char *path)
{
	ND_TRACE_FUNC();
	int reTrytimes = 100;
	sessionId_t sid;
	while (--reTrytimes > 0) {
		if (BuildSessionId(sid)) {
			break;
		}
		if (reTrytimes == 0) {
			return sessionId_t();
		}
	}

	sessionValInfo valInfo;
	if (path && *path) {
		valInfo.path = path;
	}
	else {
		valInfo.path = SESSION_DEFAULT_PATH;
	}

	if (lifeOfSeconds == 0) {
		valInfo.invalidTm = app_inst_time(NULL) + SESSION_DEFAULT_TIMEOUT;
	}
	else {
		valInfo.invalidTm = app_inst_time(NULL) + lifeOfSeconds;
	}
	valInfo.val = val;


	nd_mutex_lock(&m_lock);
	m_data[sid] = valInfo;
	nd_mutex_unlock(&m_lock);
	return sid;
}

bool SessionIdMgr::BuildSessionId(sessionId_t &sid)
{
	ND_TRACE_FUNC();
	static NDUINT32 _s_index = 0;
	ndtime_t tmnow = nd_time();
	tmnow *= rand();
	tmnow = ~tmnow;

	++_s_index;
	char session_sed[128];
	int len =ndsnprintf(session_sed, sizeof(session_sed), "%d_%d_apollohttp", _s_index, tmnow);

	char key_buf[33];
	sid = MD5Crypt32(session_sed, len, key_buf);
	

	nd_mutex_lock(&m_lock);
	sessionData_t::iterator it= m_data.find(sid) ;
	if (it == m_data.end()) {
		nd_mutex_unlock(&m_lock);
		return true;
	}
	nd_mutex_unlock(&m_lock);
	return false;
}
bool SessionIdMgr::DestroySessionId(const sessionId_t &sid)
{
	ND_TRACE_FUNC();

	nd_mutex_lock(&m_lock);
	m_data.erase(sid);
	nd_mutex_unlock(&m_lock);
	return true;
}

bool SessionIdMgr::GetSessionIdVal(const sessionId_t  &sid, sessionValInfo &outval)
{
	ND_TRACE_FUNC();

	nd_mutex_lock(&m_lock);

	sessionData_t::iterator it = m_data.find(sid);
	if (it == m_data.end()) {
		nd_mutex_unlock(&m_lock);
		return false;
	}
	outval = it->second;
	nd_mutex_unlock(&m_lock);
	return true;
}

bool SessionIdMgr::SaveSessionIdValue(const sessionId_t  &sid, const char *name, const char*val)
{
	ND_TRACE_FUNC();
	if (!name || !*name) {
		return false;
	}

	nd_mutex_lock(&m_lock);

	sessionData_t::iterator it = m_data.find(sid);
	if (it == m_data.end()) {
		nd_mutex_unlock(&m_lock);
		return false;
	}
	sessionIdVal &valmaps  = it->second.val;
	if (!val || !*val) {
		valmaps.erase(name);
	}
	else {
		valmaps[name] = val;
	}
	nd_mutex_unlock(&m_lock);
	return true;
}
