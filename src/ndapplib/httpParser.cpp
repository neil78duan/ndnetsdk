//
//  httpParser.cpp
//  roborts
//
//  Created by duanxiuyun on 15-3-19.
//  Copyright (c) 2015年 duanxiuyun. All rights reserved.
//

#include "ndapplib/httpParser.h"
#include "ndapplib/nd_listener.h"

char *parser_valid( char *src, int size)
{
	unsigned char *tmp = (unsigned char *)src ;
	while(*tmp <=(unsigned char) 0x20) {
		if(*tmp==0 || --size <=0 ) {
			return NULL;
		}
		tmp++ ;
	}
	return (char*)tmp ;
	
}

int parser_fetch_data(char *src, char *outstr, int size)
{
	unsigned char a ;
	char *_init = src ;
	while(*src && --size >=0) {
		a = (unsigned char)*src ;
		if(a > 0x20 ){
			*outstr++ = *src++ ;
		}
		else {
			break ;
		}
	}
	*outstr = 0 ;
	return (int)(src - _init);
}

bool parser_check_end(char *src, char **next, int size)
{
	unsigned char a ;
	
	while(*src && --size >= 0) {
		a = (unsigned char)*src++ ;
		
		if (a==' ' || a=='\t') {
			continue ;
		}
		if (a=='\r' && *src=='\n') {
			(*next) = src + 1  ;
			return true ;
		}
	}
	return false ;
}

static int _sendHttpRequest(nd_handle h, NDHttpRequest *reques, const char *path, const char *host, int port)
{
	int bodySize = reques->getBodySize();;
	int len;
	char *p;
	char buf[0x10000];

	p = buf;

	if (NDHttpRequest::E_ACTION_POST == reques->getAction()) {
		len = snprintf(p, sizeof(buf), "POST /%s HTTP/1.1\r\nHost: %s:%d\r\n", path, host, port);
		p += len;

		len = snprintf(p, sizeof(buf) - (p - buf), "Accept: */*\r\n");
		p += len;
	}
	else {

		len = snprintf(p, sizeof(buf), "GET /%s HTTP/1.1\r\nHost: %s:%d\r\n", path, host, port);
		p += len;
	}
	

	len = reques->HeaderToBuf(p, sizeof(buf) - (p - buf));
	p += len; 

	len = snprintf(p, sizeof(buf) - (p - buf), "Content-Type:application/x-user-define;charset=utf-8\r\n"
		"Content-Length:%d\r\nConnection: Keep-Alive\r\n\r\n", bodySize);
	p += len;

	if (bodySize) {
		len = snprintf(p, sizeof(buf) - (p - buf), "%s\r\n\r\n", reques->getBody());
		p += len;
	}

	return nd_connector_raw_write(h, buf, p - buf);
}

static int _sendHttpResponse(nd_handle h, NDHttpResponse *reques, const char *errorDesc)
{
	int bodySize = reques->getBodySize();;
	int len;
	char *p;
	char buf[0x10000];

	p = buf;


	len = snprintf(p, sizeof(buf), "HTTP/1.1 %d %s \r\n", reques->getStatus(), errorDesc);
	p += len;

	len = reques->HeaderToBuf(p, sizeof(buf) -(p - buf));
	p += len;

	len = snprintf(p, sizeof(buf) - (p - buf),"Server:userDefine\r\nContent-Length:%d\r\nConnection: Keep-Alive\r\n\r\n", bodySize);
	p += len;

	if (bodySize) {
		len = snprintf(p, sizeof(buf) - (p - buf), "%s\r\n\r\n", reques->getBody());
		p += len;
	}

	return nd_connector_raw_write(h, buf, p - buf);
}


////////////////////
//http parser

NDHttpRequest::NDHttpRequest() :m_status(0), m_parseStat(0), m_parseProgress(0)
{
	ndlbuf_init(&m_recvBuf, 32 * 1024);
	m_bodySize = -1;
	m_action = E_ACTION_GET;
}
NDHttpRequest:: ~NDHttpRequest()
{

}

void NDHttpRequest::InData(const char *data, int size)
{
	ndlbuf_write(&m_recvBuf, (char*)data, size, 0);
	while (m_parseStat < 3 && ParseData() > 0) {
		if (_getDataSize() == 0) {
			break;
		}
	}
}

int NDHttpRequest::ParseData()
{
	if (0 == m_parseStat) {
		return _parseInit();
	}
	else if (1 == m_parseStat) {
		return _parseHeader();
	}
	else if (2 == m_parseStat){
		if (m_action == E_ACTION_GET){

			m_parseStat = 3;
			return 0;
		}
		return _parseBody();
	}
	return 0;
}

int NDHttpRequest::OnEnd()
{

	return 0;
}



int NDHttpRequest::_parsePathInfo(const char *path)
{
	char val[4096];
	char buf[4096];
	size_t size = strlen(path);
	const char *p = ndstr_nstr_ansi(path, buf, '?', sizeof(buf));
	m_path = buf;

	if (!p || !*p || *p != '?')	{
		return 0;
	}
	++p;
	do 	{
		val[0] = 0;
		buf[0] = 0;
		p = ndstr_nstr_ansi(p, buf, '=', sizeof(buf));
		if (!p || !*p || *p != '=')	{
			return 0;
		}
		++p;
		p = ndstr_nstr_ansi(p, val, '&', sizeof(val));
		if (p && *p == '&')	{
			++p;
		}

		if (buf[0]) 	{
			httpHeaderNode node = { buf, val };
			m_requestForms.push_back(node);
		}

	} while (p && *p);

	return p - path ;
}

std::string *NDHttpRequest::_getHeader(const char *name, HttpHeader_t &header)
{
	HttpHeader_t::iterator it;
	for (it = header.begin(); it != header.end(); ++it) {
		if (ndstricmp((char*)it->name.c_str(), (char*)name) == 0) {
			return &it->value;
		}
	}
	return  NULL;
}

const char *NDHttpRequest::getHeader(const char *hdrname)
{
	//	HttpHeader_t::iterator it = m_header.find(std::string(hdrname) ) ;
	//	if (it == m_header.end()) {
	//		return NULL ;
	//	}
	//	return (it->second.c_str() );

	HttpHeader_t::iterator it;
	for (it = m_header.begin(); it != m_header.end(); ++it) {
		if (ndstricmp((char*)it->name.c_str(), (char*)hdrname) == 0) {
			return it->value.c_str();
		}
	}
	return  NULL;

}
int NDHttpRequest::getStatus()
{
	return m_status;
}
const char *NDHttpRequest::getBody()
{
	if (m_body.size() > 0) {
		return m_body.c_str();
	}
	return  NULL;
}


int NDHttpRequest::getBodySize()
{
	return m_body.size();
}

int NDHttpRequest::_findBodySize()
{
	const char *p = getHeader("content-length");
	if (p) {
		m_bodySize = atoi(p);
		return m_bodySize;
	}
	return -1;
}

int NDHttpRequest::_dumRequestHeader()
{
	int ret = 0;
	HttpHeader_t::iterator it;
	for (it = m_header.begin(); it != m_header.end(); ++it) {
		++ret;
		fprintf(stderr, "[header] %s : %s \n", it->name.c_str(), it->value.c_str());
	}

	for (it = m_requestForms.begin(); it != m_requestForms.end(); ++it) {
		++ret;
		fprintf(stderr, "[forms] %s : %s \n", it->name.c_str(), it->value.c_str());
	}

	return  ret;
}

char *NDHttpRequest::_getCurParseAddr()
{
	if (ndlbuf_datalen(&m_recvBuf)) {
		return (char*)ndlbuf_data(&m_recvBuf);
	}
	return NULL;

}

int NDHttpRequest::_getDataSize()
{
	return (int)ndlbuf_datalen(&m_recvBuf);
}

void NDHttpRequest::Reset()
{
	ndlbuf_reset(&m_recvBuf);
	m_parseStat = 0;
	m_parseProgress = 0;
	m_header.clear();
	m_body.clear();
	m_requestForms.clear();
	m_path.clear();

	m_action = E_ACTION_GET;
	m_status = 200;
	m_bodySize = 0;

}


bool NDHttpRequest::CheckRecvOk()
{
	return (3 == m_parseStat || (2 == m_parseStat && m_action == E_ACTION_GET));
}

int NDHttpRequest::_parseInit()
{
	int len;
	int datasize = _getDataSize();
	char *p_start = _getCurParseAddr();
	char buf[1024];
	if (!p_start) {
		return 0;
	}
	char *p = p_start;

	if (0 == ndstricmp_n(p, "POST", 4)){
		m_action = E_ACTION_POST;
		p += 4;
	}
	else if (0 == ndstricmp_n(p, "GET", 3)) {
		m_action = E_ACTION_GET;
		p += 3;
	}

	p = strchr(p, '/');
	if (!p || *p != '/'){
		return 0;
	}
	++p;
	if (*p != ' '){
		//parse path
		buf[0] = 0;
		p = (char*)ndstr_nstr_ansi(p, buf, ' ', sizeof(buf));
		if (buf[0]) {
			httpHeaderNode node1 = { "PATH", buf };
			m_header.push_back(node1);
			_parsePathInfo(buf);
		}
	}


	p = (char*)ndstristr(p, (char*)"HTTP");

	if (p) {
		len = parser_fetch_data(p, buf, datasize - (int)(p - p_start));
		if (len >= datasize) {
			return  0;
		}
		p += len;
		//get status
		//m_httpProtocol = buf ;

		m_status = (int)strtol(p, &p, 10);
		if (!p) {
			return 0;
		}

		if (parser_check_end(p, &p, datasize - (int)(p - p_start))) {
			//parse prorocol success
			len = (int)(p - p_start);

			ndlbuf_sub_data(&m_recvBuf, len);
			m_parseStat = 1;
			return len;
		}

	}
	return 0;

}


bool NDHttpRequest::addHeader(const char *name, const char *value)
{
	std::string *pHeader = _getHeader(name, m_header);
	if (pHeader)	{
		*pHeader = value;
	}
	else {
		httpHeaderNode node1 = { name, value };
		m_header.push_back(node1);
	}
	return true;
}


const char* NDHttpRequest::getRequestVal(const char *name)
{
	std::string *pHeader = _getHeader(name, m_requestForms);
	if (pHeader)	{
		return pHeader->c_str();
	}
	else {
		return NULL;
	}
}


size_t NDHttpRequest::HeaderToBuf(char *buf, size_t size)
{
	HttpHeader_t::iterator it;
	char *p = buf;
	for (it = m_header.begin(); it != m_header.end(); ++it) {
		if (it->value.size() > 0)	{
			int len = snprintf(p, size, "%s:%s\r\n", it->name.c_str(), it->value.c_str());
			p += len;
			if (size >= len) {
				size -= len;
			}
		}
	}
	return  p - buf;
}

size_t NDHttpRequest::RequestValueTobuf(char *buf, size_t size)
{
	HttpHeader_t::iterator it;
	char *p = buf;
	for (it = m_requestForms.begin(); it != m_requestForms.end(); ++it) {
		if (it->value.size() > 0)	{
			int len = snprintf(p, size, "%s:%s\r\n", it->name.c_str(), it->value.c_str());
			p += len;
			if (size >= len) {
				size -= len;
			}
		}
	}
	return  p - buf;
}


int NDHttpRequest::_parseHeader()
{

	int datasize = _getDataSize();
	char *p_start = _getCurParseAddr();
	char buf[1024];

	if (!p_start) {
		return 0;
	}
	char *p = p_start;

	p = parser_valid(p, datasize);
	if (!p) {
		return  0;
	}

	p = (char*)ndstr_nstr_end(p, buf, ':', datasize - (int)(p - p_start));
	if (*p != ':') {
		return  0;
	}
	std::string hdrName = buf;

	++p;
	p = parser_valid(p, datasize - (int)(p - p_start));
	char *_st = p;
	if (parser_check_end(p, &p, datasize - (int)(p - p_start))) {
		int len = (int)(p - _st);
		memcpy(buf, _st, len - 2);
		buf[len - 2] = 0;

		std::string hdrVal = buf;

		httpHeaderNode node1 = { hdrName, hdrVal };
		m_header.push_back(node1);

		len = (int)(p - p_start);
		ndlbuf_sub_data(&m_recvBuf, len);

		if (*p == '\r' && *(p + 1) == '\n') {
			ndlbuf_sub_data(&m_recvBuf, 2);
			m_parseStat = 2;
			//_dumRequestHeader() ;
			if (_findBodySize() == 0) {
				m_parseProgress = 3;//without body

			}
		}

		return len;
	}

	return 0;


}
int NDHttpRequest::_parseBody()
{
	int datasize = _getDataSize();
	char *p_start = _getCurParseAddr();

	if (!p_start) {
		return 0;
	}
	char *p = p_start;

	for (int i = 0; i < datasize; ++i) {
		m_body.push_back(*p);
		++p;

		if (m_body.size() >= m_bodySize) {
			m_parseStat = 3;
			break;
		}
	}

	ndlbuf_sub_data(&m_recvBuf, (p - p_start));

	return (int)(p - p_start);

}

//////////////////////////////////////////////////////////////////////////
HttpConnector::HttpConnector(NDHttpRequest *request):m_recvRequest(request),m_conn(0),m_port(0)
{
	
}

HttpConnector:: ~HttpConnector()
{
	Destroy() ;
}

int HttpConnector::Create(const char *host, int port)
{
	if (m_conn) {
		nd_object_destroy(m_conn, 0) ;
		//DestroyConnectorObj(m_conn) ;
		m_conn = 0 ;
	}
	
	m_conn = nd_object_create("tcp-connector"  ) ;
	
	if(!m_conn){
		nd_logerror((char*)"connect error :%s!" , nd_last_error()) ;
		return -1;
	}
	((nd_netui_handle)m_conn)->user_data =(void*) this ;
	
	int ret = nd_connector_open(m_conn,host, port, NULL);
	
	if(ret == 0 ) {
		m_host = host ;
		m_port = port ;
	}
	else {
		nd_object_destroy(m_conn, 0) ;
		m_conn = 0 ;
		return -1;
	}
	
	return 0 ;
	
}
void HttpConnector::Destroy()
{
	
	if (m_conn) {
		nd_object_destroy(m_conn, 0) ;
		//DestroyConnectorObj(m_conn) ;
		m_conn = 0 ;
	}

}


int HttpConnector::SendRequest(NDHttpRequest &request, const char *host, int port, const char *path)
{
	return _sendHttpRequest(m_conn, &request, path, host, port);
}

int HttpConnector::Recv(char *buf, int size, int timeout)
{
	return  nd_connector_raw_waitdata( m_conn,  buf, size, timeout) ;
}

int HttpConnector::Update(int timeout)
{
	int ret = -1 ;
	char buf[1024*32] ;
	if (!CheckValid() ) {
		m_recvRequest->setStatus(-1) ;
		m_recvRequest->OnEnd() ;
		return -1 ;
	}
	ndtime_t start = nd_time() ;
	int lefttime = timeout ;
	
	do {
		int size = Recv(buf, sizeof(buf), lefttime) ;
		if (size <=0){
			break ;
		}
		m_recvRequest->InData(buf, size) ;
		if (m_recvRequest->CheckRecvOk()) {
			ret = 0;
			break ;
		}
		lefttime  -= nd_time() - start ;
		start = nd_time() ;

	}while (lefttime > 0);
	
	if (ret ==-1 ) {
		m_recvRequest->setStatus(-1) ;
	}
	m_recvRequest->OnEnd() ;
	
	return ret;
}

bool HttpConnector::CheckValid()
{
	
	if (m_conn) {
		return nd_connector_valid((nd_netui_handle)m_conn) ? true: false ;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////

static int _session_data_handler(nd_handle sessionHandler, void *data, size_t len, nd_handle listen_h)
{
	NDHttpSession *pSession = (NDHttpSession*) NDGetSession(sessionHandler);
	if (!pSession){
		return -1;
	}
	return pSession->onDataRecv((char*)data, len);
}

static int testRequest1(NDHttpSession *pSession, const NDHttpRequest &resuest);
static int testRequest2(NDHttpSession *pSession, const NDHttpRequest &resuest);

NDHttpSession::requestEntry_map NDHttpSession::m_request_entry;

bool NDHttpSession::installRequest(const char *pathName, http_reqeust_func func)
{
	if (!pathName || !*pathName)	{
		return false;
	}
	m_request_entry[pathName] = func;
	return true;
}


NDHttpSession::NDHttpSession()
{
	//installRequest("test1", testRequest1);
	//installRequest("test2", testRequest2);
}

NDHttpSession ::~NDHttpSession()
{
}


int NDHttpSession::SendResponse(NDHttpResponse &response, const char *errorDesc)
{
	return _sendHttpResponse(GetHandle(),&response, errorDesc);
}


int NDHttpSession::sendErrorResponse(int errorCdoe)
{
	int len;
	char *p;
	char buf[0x10000];

	p = buf;

	len = snprintf(p, sizeof(buf), "HTTP/1.1 %d error%d \r\n", errorCdoe, errorCdoe);
	p += len;


	return nd_connector_raw_write(GetHandle(), buf, p - buf);
}

void NDHttpSession::OnCreate()
{
	nd_hook_data(GetHandle(), _session_data_handler);
}

int NDHttpSession::onDataRecv(char *buf, int size)
{
	m_request.InData(buf, size);

	if (m_request.CheckRecvOk()){
		m_request._dumRequestHeader();
		onRequest(m_request.getPath(), m_request);
		m_request.Reset();
	}
	//ndprintf("%s\n", buf);
	return size;
}


int NDHttpSession::onRequest(const char *path, const NDHttpRequest &request)
{
	requestEntry_map::iterator it =m_request_entry.find(path);
	if (it == m_request_entry.end()  || !it->second)	{
		return 404;
	}

	return it->second(this, request);
}



