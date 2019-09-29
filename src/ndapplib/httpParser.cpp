//
//  httpParser.cpp
//  roborts
//
//  Created by duanxiuyun on 15-3-19.
//  Copyright (c) 2015 duanxiuyun. All rights reserved.
//

#include "nd_net/nd_netlib.h"
#include "nd_common/nd_common.h"
#include "ndapplib/httpParser.h"
#include "ndstl/nd_utility.h"

char *parser_valid( char *src, int size)
{
	ND_TRACE_FUNC();
	unsigned char *tmp = (unsigned char *)src ;
	while(*tmp <=(unsigned char) 0x20) {
		if(*tmp==0 || --size <=0 ) {
			return NULL;
		}
		tmp++ ;
	}
	return (char*)tmp ;
	
}

bool parser_check_end(char *src, char **next, int size)
{
	ND_TRACE_FUNC();
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

static int _sendHttpRequest(nd_handle h, NDHttpRequest *reques, const char *path, const char *host, int port,bool bLongConnect)
{
	ND_TRACE_FUNC();
	int ret = 0;
	int len, bodySize;
	char *p;
	char buf[4096];
	char bodyBuf[0x10000];
	char *formBuf = bodyBuf;

	bodyBuf[0] = 0;
	buf[0] = 0;

	
	bodySize = (int)reques->RequestValueTobuf(bodyBuf, sizeof(bodyBuf));
	if (bodySize == 0 && reques->getBodySize() > 0 && NDHttpRequest::E_ACTION_POST){
		bodySize = (int) reques->getBodySize();
		formBuf = (char*) reques->getBody();
	}
	
	p = buf;

	if (NDHttpRequest::E_ACTION_POST == reques->getAction()) {
		len = ndsnprintf(p, sizeof(buf), "POST /%s HTTP/1.1\r\nHost: %s:%d\r\n", path, host, port);
		p += len;

		len = ndsnprintf(p, sizeof(buf) - (p - buf), "Accept: */*\r\n");
		p += len;
	}

	else {
		if (bodySize > 0){
			len = ndsnprintf(p, sizeof(buf), "GET /%s?%s HTTP/1.1\r\nHost: %s:%d\r\n", path, formBuf, host, port);
			p += len;
		}
		else {
			len = ndsnprintf(p, sizeof(buf), "GET /%s HTTP/1.1\r\nHost: %s:%d\r\n", path, host, port);
			p += len;
		}
		bodySize = 0;
	}
	
	len =(int) reques->HeaderToBuf(p, sizeof(buf) - (p - buf));
	p += len; 

	len = ndsnprintf(p, sizeof(buf) - (p - buf), "Content-Type:application/x-www-form-urlencoded;charset=utf-8\r\n"
		"Content-Length:%d\r\nConnection:Keep-Alive\r\n\r\n", bodySize);
	p += len;

	//write header to send buf
	len = nd_connector_send_stream(h, buf, p - buf,0);
	if (len <= 0) {
		return len;
	}
	ret += len;
	
	if (NDHttpRequest::E_ACTION_POST == reques->getAction() && bodySize) {
		//len = ndsnprintf(p, sizeof(buf) - (p - buf), "%s\r\n\r\n", formBuf);
		//p += len;

		len = nd_connector_send_stream(h, formBuf, bodySize, 0);
		if (len <= 0) {
			return len;
		}
		ret += len;


		len = nd_connector_send_stream(h, (void*)"\r\n\r\n",4, 0);
		if (len <= 0) {
			return len;
		}
		ret += len;
	}

	nd_tcpnode_flush_sendbuf_force((nd_netui_handle)h);
	return 0;
}

int _sendHttpResponse(nd_handle h, NDHttpResponse *reques, const char *errorDesc,const char *serverInfo)
{
	ND_TRACE_FUNC();
	int ret = 0;
	int bodySize = reques->getBodySize();;
	int len;
	char *p;
	char buf[0x10000];

	p = buf;

	len = ndsnprintf(p, sizeof(buf), "HTTP/1.1 %d %s \r\n", reques->getStatus(), errorDesc);
	p += len;

	len =(int) reques->HeaderToBuf(p, sizeof(buf) -(p - buf));
	p += len;

	len = ndsnprintf(p, sizeof(buf) - (p - buf),"Server:%s\r\nContent-Length:%d\r\n\r\n",serverInfo, bodySize);
	p += len;

	len = nd_connector_send_stream(h, buf, p - buf, 0);
	if (len <= 0) {
		return len;
	}
	ret += len;


	if (bodySize) {
		len = nd_connector_send_stream(h, (void*)reques->getBody(), bodySize,0);
		if (len <= 0) {
			return len;
		}
		ret += len;
		nd_connector_send_stream(h, (void*)"\r\n\r\n",4, 0);
		ret += 4;
	}

	nd_tcpnode_flush_sendbuf_force((nd_netui_handle)h); 
	return ret;
}



static bool _getHttp(HttpConnector &conn, const char *host, int port, const char*path, const char*reqData)
{
	if (-1 == conn.Open(host, port)) {
		nd_logerror("open %s:%d\n", host, port);
		return false;
	}
	NDHttpRequest req;
	if (reqData) {
		req.setBody(reqData);
	}

	if (conn.SendRequest(req, host, port, path) < 0) {
		nd_logerror("send request error\n");
		return false;
	}
	return true;
}

bool NDHttpGet(HttpConnector &conn, const char *url)
{
	int port = 80;
	const char *p = url;
	char buf[1024];
	char * paim = (char*)ndstristr(p, "http://");
	if (paim) {
		p = paim + 7;
	}
	else {
		paim = (char*)ndstristr(p, "https://");
		if (paim) {
			p = paim + 8;
			conn.setHttps(true);
			port = 443;
		}
	}
	//get host 
	p = ndstr_nstr_end(p, buf, '/', sizeof(buf));

	paim = ndstrchr(buf, ':');
	if (paim) {
		++paim;
		port = atoi(paim);
		*--paim = 0;
	}

	if (!p || !*p) {
		return _getHttp(conn, buf, port, NULL, NULL);
	}

	if (*p == '/') {
		++p;
	}

	std::string host = buf;
	std::string path = "";

	paim = (char*)ndstrchr(p, '?');
	if (paim) {
		buf[0] = 0;
		p = ndstr_nstr_end(p, buf, '?', sizeof(buf));
		path = buf;
		if (*p && *p == '?') {
			++p;
		}
	}
	return _getHttp(conn, host.c_str(), port, path.c_str(), p);
}

///////////////////////////////////
static unsigned char bits4ToHex(unsigned char x)
{
	return  x > 9 ? x + 55 : x + 48;
}

static unsigned char bits4ToHexLittle(unsigned char x)
{
	return  x > 9 ? x + 87 : x + 48;
}

static unsigned char FromHexChar(unsigned char x)
{
	unsigned char y=0;
	if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
	else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
	else if (x >= '0' && x <= '9') y = x - '0';
	else {}
	return y;
}


std::string NDHttpParser::textToURLcode(const char *text, bool isLittle )
{
	ND_TRACE_FUNC();
	std::string strTemp = "";
	while (*text) {
		unsigned char chr1 = *text++;
		if (isalnum(chr1) || (chr1 == '-') || (chr1 == '_') ||(chr1 == '.') || (chr1 == '~')) {
			strTemp += chr1;
		}
		else if (chr1 == ' ') {
			strTemp += "+";
		}
		else {
			strTemp += '%';
			unsigned char t1 = isLittle? bits4ToHexLittle(chr1 >> 4): bits4ToHex(chr1 >> 4);
			strTemp += t1;
			unsigned char t2 = isLittle ? bits4ToHexLittle(chr1 % 16) :  bits4ToHex(chr1 % 16);
			strTemp += t2;
		}
	}	
	return strTemp;
}
std::string NDHttpParser::URLcodeTotext(const char *urlCode)
{
	ND_TRACE_FUNC();
	std::string strTemp = "";
	while (*urlCode) {
		unsigned char chr1 = *urlCode++;
		if (chr1 == '%') {
			char hiBits4 = *urlCode++;
			if (!hiBits4) {
				break;
			}
			char loBits4 = *urlCode++;
			if (!loBits4) {
				break;
			}
			unsigned char val = FromHexChar(hiBits4) << 4 | FromHexChar(loBits4);
			strTemp += val;
		}
		else if (chr1 == '+') {
			strTemp += " ";
		}
		else {
			strTemp += chr1;
		}
	}
	return strTemp;
}


bool NDHttpParser::getFileRange(const char *range, size_t &offset, size_t &length, size_t fileSize)
{
	ND_TRACE_FUNC();
	const char *rangeData = ndstristr(range, "bytes=");
	if (!rangeData) {
		return false;
	}
	
	char *p = (char*) rangeData + 6;
	NDINT64 start = strtoll(p, &p, 10);
	if (start < 0) {
		length = (size_t)(-start);
		offset = fileSize - length;
		return true;
	}
	else {
		offset = start;
	}

	size_t end = fileSize;

	p =(char*) ndstrchr(p, '-');
	if (p && *p == '-') {
		++p;
	}
	else {
		return false;
	}

	if (p && *p) {
		end = (size_t)strtoll(p, &p, 10);
		length = end - offset + 1;
	}
	else {
		length = fileSize - offset;
	}
	

	return true;
}

//////////////////////////////////////////////////////////////////////////

NDHttpParser::NDHttpParser()
{
	ND_TRACE_FUNC();
	ndlbuf_init(&m_bodyBuf, 4096);
	m_parseError = false;
	ndlbuf_auto_inc_enable(&m_bodyBuf);
	Reset();
}

NDHttpParser::~NDHttpParser()
{
	ND_TRACE_FUNC();
	ndlbuf_destroy(&m_bodyBuf);
	Reset();

}

void NDHttpParser::Reset()
{
	ND_TRACE_FUNC();
	ndlbuf_reset(&m_bodyBuf);
	
	m_parseStat = 0;
	m_parseProgress = 0;
	m_header.clear();
	
	m_action = E_ACTION_GET;
	m_status = 200;
	m_parseError = false;
}
bool NDHttpParser::CheckRecvOk()
{
	return (3 == m_parseStat || (2 == m_parseStat && m_action == E_ACTION_GET));
}

bool NDHttpParser::isLongConnect()
{
	ND_TRACE_FUNC();
	const char *pConnHeader = getHeader("Connection");
	if (pConnHeader && *pConnHeader){
		if (ndstristr(pConnHeader, "Keep-alive"))	{
			return true;
		}
	}
	return false;
}

void NDHttpParser::InData(const char *data, int size)
{
	ND_TRACE_FUNC();
	//ndprintf("%s\n", data);
	//char end = 0;
	ndlbuf_write(&m_bodyBuf, (void*)data, size, 0);
	//ndlbuf_write(&m_bodyBuf, (void*)&end, 1, 0); //write end char
	ndlbuf_set_zero_tail(&m_bodyBuf);

	while (m_parseStat < 3 && ParseData() > 0) {
		if (_getDataSize() == 0 || CheckParseError()) {
			break;
		}
	}
}


int NDHttpParser::dump()
{
	ND_TRACE_FUNC();
	int ret = 0;
	HttpHeader_t::iterator it;
	for (it = m_header.begin(); it != m_header.end(); ++it) {
		++ret;
		ndfprintf(stderr, "[header] %s : %s \n", it->name.c_str(), it->value.c_str());
	}
	
	if (getBodySize())	{
		ndfprintf(stderr, "%s\n", getBody());
	}
	return  ret;
}

void NDHttpParser::setBody(const char*body)
{
	ND_TRACE_FUNC();
	ndlbuf_write(&m_bodyBuf, (void*)body, ndstrlen(body), 0);
}
const char *NDHttpParser::getBody()
{
	ND_TRACE_FUNC();
	ndlbuf_set_zero_tail(&m_bodyBuf);
	return (const char*) ndlbuf_data(&m_bodyBuf);
}

int NDHttpParser::getBodySize()
{
	ND_TRACE_FUNC();
	return (int)ndlbuf_datalen(&m_bodyBuf);
}

const char *NDHttpParser::getHeader(const char *name)const
{
	ND_TRACE_FUNC();
	httpHeaderNode *pNode = ((NDHttpParser*)this)->_getNode(name, (HttpHeader_t&)m_header);
	if (pNode)	{
		return pNode->value.c_str();
	}
	return NULL;
}

const char* NDHttpParser::getHeaderVal(int index)const
{
	ND_TRACE_FUNC();
	if (index < m_header.size()) {
		return m_header[index].value.c_str();
	}
	return NULL;
}
const char* NDHttpParser::getHeaderName(int index)const
{
	ND_TRACE_FUNC();
	if (index < m_header.size()) {
		return m_header[index].name.c_str();
	}
	return NULL;
}

bool NDHttpParser::addHeader(const char *name, const char *value)
{
	ND_TRACE_FUNC();
	_adNode(name, value, m_header);
	return true;
}

int NDHttpParser::Create(const char *name)
{
	return 0;
}
void NDHttpParser::Destroy(int flag)
{
}

int NDHttpParser::ParseProtocol()
{
	return -1;
}

void NDHttpParser::onParseEnd()
{

}

int NDHttpParser::ParseData()
{
	ND_TRACE_FUNC();
	if (0 == m_parseStat) {
		return ParseProtocol();
	}
	else if (1 == m_parseStat) {
		return _parseHeader();
	}
	else if (2 == m_parseStat){
		if (m_action == E_ACTION_GET){
			_setParseEnd();
			return 0;
		}
		return _parseBody();
	}
	return 0;
}


void NDHttpParser::_setParseEnd()
{
	ND_TRACE_FUNC();
	if (m_parseStat != 3) {
		m_parseStat = 3;
		onParseEnd();
	}
}

char *NDHttpParser::_getCurParseAddr()
{
	ND_TRACE_FUNC();
	if (ndlbuf_datalen(&m_bodyBuf)) {
		ndlbuf_set_zero_tail(&m_bodyBuf);
		return (char*)ndlbuf_data(&m_bodyBuf);
	}
	return NULL;

}

int NDHttpParser::_getDataSize()
{
	ND_TRACE_FUNC();
	return (int)ndlbuf_datalen(&m_bodyBuf);
}


std::string *NDHttpParser::_getHeader(const char *name)
{
	ND_TRACE_FUNC();
	httpHeaderNode *pNode = _getNode(name, m_header);
	if (pNode)	{
		return &(pNode->value);
	}
	return NULL;
}

httpHeaderNode *NDHttpParser::_getNode(const char *name, HttpHeader_t &headers)
{
	ND_TRACE_FUNC();
	HttpHeader_t::iterator it;
	for (it = headers.begin(); it != headers.end(); ++it) {
		if (ndstricmp((char*)it->name.c_str(), (char*)name) == 0) {
			return &(*it);
		}
	}
	return  NULL;
}
void NDHttpParser::_adNode(const char *name, const char *value, HttpHeader_t &headers)
{
	ND_TRACE_FUNC();
	httpHeaderNode *pNode = _getNode(name, headers);
	if (pNode)	{
		pNode->value = value;
	}
	else {
		httpHeaderNode node1 = { name, value };
		headers.push_back(node1);
	}
}



int NDHttpParser::_findBodySize()
{
	ND_TRACE_FUNC();
	const char *p = getHeader("content-length");
	if (p) {
		return atoi(p);
	}
	return 0;
}

int NDHttpParser::_parseHeader()
{
	ND_TRACE_FUNC();
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
	if(!p || !*p) {
		return 0;
	}
	char *_st = p;
	if (parser_check_end(p, &p, datasize - (int)(p - p_start))) {
		int len = (int)(p - _st);
		memcpy(buf, _st, len - 2);
		buf[len - 2] = 0;

		std::string hdrVal = buf;

		httpHeaderNode node1 = { hdrName, hdrVal };
		m_header.push_back(node1);


		if (*p == '\r' && *(p + 1) == '\n') {
			//ndlbuf_sub_data(&m_recvBuf, 2);
			p += 2;
			m_parseStat = 2;
			//_dumRequestHeader() ;
			if (_findBodySize() == 0) {
				m_parseProgress = 3;//without body

			}
			if (m_action == E_ACTION_GET ){
				//m_parseStat = 3;
				_setParseEnd(); 
			}
		}

		len = (int)(p - p_start);
		ndlbuf_sub_data(&m_bodyBuf, len);
		return len;
	}

	return 0;


}
int NDHttpParser::_parseBody()
{
	ND_TRACE_FUNC();
	int contentSize = _findBodySize();
	int datasize = _getDataSize();

	if (contentSize == 0 || contentSize == -1) {
		char *p_start = _getCurParseAddr();
		if (!p_start) {
			return 0;
		}
		char *p = ndstrstr(p_start, "\r\n\r\n");
		if (!p) {
			return 0;
		}
		size_t realSize = p - p_start;
		char buf[32];
		ndsnprintf(buf, sizeof(buf), "%lld", (NDUINT64)realSize);
		addHeader("content-length", buf);

		_setParseEnd();
	}
	else if(datasize >= contentSize) {
		_setParseEnd();
	}
	else {
		return 0;	//data received not completed
	}
	return datasize;
}


size_t NDHttpParser::HeaderToBuf(char *buf, size_t size)
{
	ND_TRACE_FUNC();
	HttpHeader_t::iterator it;
	char *p = buf;
	for (it = m_header.begin(); it != m_header.end(); ++it) {
		if (it->value.size() > 0)	{
			int len = ndsnprintf(p, size, "%s:%s\r\n", it->name.c_str(), it->value.c_str());
			p += len;
			if (size >= len) {
				size -= len;
			}
		}
	}
	return  p - buf;
}
////////////////////
//http parser

NDHttpRequest::NDHttpRequest() :NDHttpParser(), m_userData(0)
{
}
NDHttpRequest:: ~NDHttpRequest()
{

}

int NDHttpRequest::_parsePathInfo(const char *path)
{
	ND_TRACE_FUNC();
	char val[4096];
	char buf[4096];
	size_t size = ndstrlen(path);
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

		if (buf[0] && val[0]) 	{
			httpHeaderNode node = { buf, URLcodeTotext(val) };
			m_requestForms.push_back(node);
		}

	} while (p && *p);

	return (int)(p - path );
}



int NDHttpRequest::dump()
{
	ND_TRACE_FUNC();
	int ret = 0;
	HttpHeader_t::iterator it;
	for (it = m_header.begin(); it != m_header.end(); ++it) {
		++ret;
		ndfprintf(stderr, "[header] %s : %s \n", it->name.c_str(), it->value.c_str());
	}

	for (it = m_requestForms.begin(); it != m_requestForms.end(); ++it) {
		++ret;
		ndfprintf(stderr, "[forms] %s : %s \n", it->name.c_str(), it->value.c_str());
	}
	return  ret;
}


void NDHttpRequest::Reset()
{
	ND_TRACE_FUNC();
	NDHttpParser::Reset();
	m_path.clear();
	m_requestForms.clear();
	_destroyUpFile();
	m_userData = NULL;
	m_cookies.clear();
}

int NDHttpRequest::ParseProtocol()
{
	ND_TRACE_FUNC();
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

	p = ndstrchr(p, '/');
	if (!p || *p != '/'){
		return 0;
	}
	++p;
	if (*p != ' '){
		//parse path
		buf[0] = 0;
		p = (char*)ndstr_nstr_ansi(p, buf, ' ', sizeof(buf));
		if (buf[0]) {
			std::string myFullPath = URLcodeTotext(buf);
			httpHeaderNode node1 = { "PATH", myFullPath.c_str() };
			m_header.push_back(node1);
			_parsePathInfo(myFullPath.c_str());
		}
	}

	p = (char*)ndstristr(p, (char*)"HTTP");
	
	if (!p || !*p){
		return 0;
	}
	p += 4;
	if (parser_check_end(p, &p, datasize - (int)(p - p_start))) {
		//parse prorocol success
		len = (int)(p - p_start);

		ndlbuf_sub_data(&m_bodyBuf, len);
		m_parseStat = 1;
		return len;
	}

	return 0;


}


void NDHttpRequest::onParseEnd()
{
	ND_TRACE_FUNC();
	if (m_action == E_ACTION_POST && getBodySize()>0){
		_postBodyToJson();
	}
	_parseCookies();
}

const char* NDHttpRequest::getRequestVal(const char *name)
{
	ND_TRACE_FUNC();
	httpHeaderNode *pNode = _getNode(name, m_requestForms);
	if (pNode)	{
		return pNode->value.c_str();
	}
	return NULL;
}

bool NDHttpRequest::addRequestFormVal(const char *name, const char *value)
{
	ND_TRACE_FUNC();
	_adNode(name, value, m_requestForms);
	return true;
}


const NDHttpRequest::fileCacheInfo *NDHttpRequest::getUploadFile(const char *filename)const
{
	ND_TRACE_FUNC();
	fileCacheMap_t::const_iterator it = m_upfiles.find(filename);
	if (it == m_upfiles.end()) {
		return NULL;
	}
	return &it->second;
}

size_t NDHttpRequest::RequestValueTobuf(char *buf, size_t size)
{
	ND_TRACE_FUNC();
	char *p = buf;
	for (size_t i = 0; i < m_requestForms.size(); i++)	{
		if (m_requestForms[i].value.size() ==0)		{
			continue;
		}
		const httpHeaderNode &node = m_requestForms[i];
		int len = 0;

		if (i == m_requestForms.size() - 1)	{
			len = ndsnprintf(p, size, "%s=%s", node.name.c_str(), node.value.c_str());
		}
		else {
			len = ndsnprintf(p, size, "%s=%s&", node.name.c_str(), node.value.c_str());
		}
		p += len;
		size -= len;
	}
	return  p - buf;
}


int NDHttpRequest::_postBodyToJson()
{
	ND_TRACE_FUNC();
	std::string *pContent = _getHeader("Content-Type");
	if (!pContent || pContent->size() == 0)	{
		return 0;
	}
	const char *content_text = pContent->c_str();
	const char *pHeaderText = ndstristr(content_text, "multipart/form-data");
	if (pHeaderText )	{
		return _parseMultipart(pHeaderText += 19);
	}

	pHeaderText = ndstristr(content_text, "application/x-www-form-urlencoded");
	if (pHeaderText) {
		return _parse_x_form();
	}
	m_parseError = true;
	return -1;
}


void NDHttpRequest::_destroyUpFile()
{
	ND_TRACE_FUNC();
	fileCacheMap_t m_upfiles;
	for (fileCacheMap_t::iterator it = m_upfiles.begin(); it != m_upfiles.end(); ++it) {
		delete[] it->second.dataAddr;
	}
	m_upfiles.clear();
}
bool NDHttpRequest::_insertFile(const char *varname, const char*filePath, void *data, size_t length)
{
	ND_TRACE_FUNC();
	fileCacheInfo cacheInfo;

	if (data && length > 0) {
		cacheInfo.dataAddr = new char[length + 1];
		memcpy(cacheInfo.dataAddr, data, length);
		cacheInfo.dataAddr[length] = 0;
	}
	cacheInfo.size = length;
	cacheInfo.fileName = filePath;
	
	std::pair<fileCacheMap_t::iterator, bool> ret = m_upfiles.insert(std::make_pair(varname, cacheInfo));
	if (ret.second == false) {
		delete cacheInfo.dataAddr;
		return false;
	}
	return true;
}

int NDHttpRequest::_parseOnePart(const char *partStart, size_t len)
{
	ND_TRACE_FUNC();
	char buf[1024];
	NDHttpParser _tmpParser;

	_tmpParser.setAction(E_ACTION_POST);
	_tmpParser.setParseStepIndex(1);
	_tmpParser.InData(partStart, (int)len);
	
	
	//_tmpParser._parseHeader();
	//_tmpParser._parseBody();

	const char *p = _tmpParser.getHeader("Content-Disposition");
	if (!p) {
		return -1;
	}
	//get name 
	const char *pName = ndstristr(p, "name=\"");
	if (!pName) {
		return -1;
	}
	pName += 6;
	ndstr_nstr_ansi(pName, buf, '\"', sizeof(buf));

	std::string name = buf;

	std::string value ;

	const char *pFileName = ndstristr(p, "filename=\"");
	if (pFileName) {
		pFileName += 10;
		ndstr_nstr_ansi(pFileName, buf, '\"', sizeof(buf));
		value = buf;

		const char *body = _tmpParser.getBody();
		_insertFile(name.c_str(), value.c_str(), (void*)body, _tmpParser.getBodySize());
	}
	else {
		value = _tmpParser.getBody();
	}


	//httpHeaderNode node = { buf, URLcodeTotext(value.c_str()) };
	httpHeaderNode node = { buf, value.c_str() };
	m_requestForms.push_back(node);
	return (int)len;
}


int NDHttpRequest::_parseMultipart(const char *pHeaderText)
{
	ND_TRACE_FUNC();
	size_t bodySize = _findBodySize();
	if (bodySize == 0) {
		return -1;
	}
	const char *pbound = ndstristr(pHeaderText, "boundary=");
	if (!pbound) {
		return -1;
	}
	pbound += 9;
	std::string boundary = "--";
	boundary += pbound;

	size_t boundarySize = ndstrlen(pbound) + 2;
	if (boundarySize == 0) {
		m_parseError = true; 
		return -1;
	}
	const char *p = getBody();
	const char *pEnd = p + bodySize;

	while (p < pEnd) {
		const char *partStart = ndstrstr(p, boundary.c_str());
		if (!partStart) {
			return (int)( p - getBody());
		}
		partStart += boundarySize;

		const char *partEnd = ndstrstr(partStart, boundary.c_str());
		if (!partEnd) {
			return p - getBody();
		}
		if (*(partEnd - 2) == '\r' && *(partEnd - 1) == '\n') {
			partEnd -= 2;
		}
		_parseOnePart(partStart, partEnd - partStart);
		p = partEnd + boundarySize;
	}
	return (int)(p - getBody());

}

int NDHttpRequest::_parse_x_form()
{

	ND_TRACE_FUNC();
	char val[4096];
	char buf[1024];

	const char *p = getBody();
	p = ndstr_first_valid(p);

	do {
		val[0] = 0;
		buf[0] = 0;
		p = ndstr_nstr_ansi(p, buf, '=', sizeof(buf)-1);
		if (!p || !*p || *p != '=') {
			m_parseError = true;
			return 0;
		}
		++p;
		p = ndstr_nstr_ansi(p, val, '&', sizeof(val)-1);
		if (p && *p == '&') {
			++p;
		}

		if (buf[0]) {
			httpHeaderNode node = { buf };
			node.value = URLcodeTotext(val);
			m_requestForms.push_back(node);
		}

	} while (p && *p);

	return (int) (p - getBody());
}

int NDHttpRequest::_parseCookies()
{
	ND_TRACE_FUNC();
	for (HttpHeader_t::const_iterator it = m_header.begin(); it != m_header.end(); ++it) {
		if (ndstricmp(it->name.c_str(), "Cookie")) {
			continue;
		}

		char buf[4096];
		const char *p = ndstr_first_valid(it->value.c_str());

		p = ndstr_nstr_end(p, buf, '=', sizeof(buf));
		if (!p || *p != '=') {
			continue;
		}
		++p;
		std::string name = buf;

		std::string val = NDHttpParser::URLcodeTotext(p);
		if (val.empty()) {
			return -1;
		}
		m_cookies[name] = val;
	}	
	return 0;

}

//////////////////////////////////////////////////////////////////////////

int NDHttpResponse::ParseProtocol()
{
	ND_TRACE_FUNC();
	int len;
	int datasize = _getDataSize();
	char *p_start = _getCurParseAddr();

	if (datasize <= 0 || !p_start)	{
		return 0;
	}


	//char buf[1024];
	if (!p_start) {
		return 0;
	}
	char *p = p_start;

	p = (char*)ndstristr(p, (char*)"HTTP");

	if (p) {
// 		len = parser_fetch_data(p, buf, datasize - (int)(p - p_start));
// 		if (len >= datasize) {
// 			return  0;
// 		}
// 		p += len;
		p += 4;//skip http ;
		p = ndstrchr(p, ' ');
		if (!p)	{
			return 0;
		}

		m_status = (int)strtol(p, &p, 10);
		if (!p) {
			return 0;
		}

		if (parser_check_end(p, &p, datasize - (int)(p - p_start))) {
			//parse prorocol success
			len = (int)(p - p_start);

			ndlbuf_sub_data(&m_bodyBuf, len);
			m_parseStat = 1;
			return len;
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////

static int _http_connector_data_handler(nd_handle sessionHandler, void *data, size_t len, nd_handle )
{
	ND_TRACE_FUNC();
	if (!data || len == 0) {
		return 0;
	}

	HttpConnector *pConn = dynamic_cast<HttpConnector *> (NDObject::FromHandle(sessionHandler));
	if (!pConn) {
		return -1;
	}
	return pConn->onDataRecv((char*)data, len);
}

HttpConnector::HttpConnector(bool bLongConnect) : m_port(0), m_bLongConnection(bLongConnect), m_responseOk(false), m_bSSL(false)
{
	
}

HttpConnector:: ~HttpConnector()
{
	Destroy() ;
}

int HttpConnector::Create(const char *instantName)
{
	ND_TRACE_FUNC();
	if (m_objhandle) {
		nd_object_destroy(m_objhandle, 0);
		//DestroyConnectorObj(m_objhandle) ;
		m_objhandle = 0;
	}

	m_objhandle = nd_object_create("tcp-connector");

	if (!m_objhandle) {
		nd_logerror((char*)"connect error :%s!", nd_last_error());
		return -1;
	}
	//((nd_netui_handle)m_objhandle)->user_data = (void*)this;
	((nd_netui_handle)m_objhandle)->msg_caller = (void*)this;
	if (instantName && *instantName) {
		nd_object_set_instname(m_objhandle, instantName);
	}
	return 0;
}

int HttpConnector::Open(const char *host, int port)
{
	ND_TRACE_FUNC();
	if (!m_objhandle) {
		if (-1 == Create(NULL)) {
			return -1;
		}
	}
	int ret = nd_connector_open(m_objhandle,host, port, NULL);
	
	if(ret == 0 ) {
		m_host = host ;
		m_port = port ;
	}
	else {
		nd_object_destroy(m_objhandle, 0) ;
		m_objhandle = 0 ;
		return -1;
	}
	nd_hook_data(m_objhandle, _http_connector_data_handler);

	OnInitilize();
	return 0 ;
	
}

int HttpConnector::Close(int flag)
{
	ND_TRACE_FUNC();
	if (m_objhandle)	{
		nd_connector_close(m_objhandle, flag);
	}
	OnClose();
	return 0;
}

void HttpConnector::Destroy(int flag)
{
	ND_TRACE_FUNC();
	if (m_objhandle) {
		nd_object_destroy(m_objhandle, flag) ;
		//DestroyConnectorObj(m_objhandle) ;
		m_objhandle = 0 ;
	}
	OnDestroy();
}


int HttpConnector::SendRequest(NDHttpRequest &request, const char *host, int port, const char *path)
{
	ND_TRACE_FUNC();
	if (path && *path) {
		m_lastRequestPath = path;
		std::string pathUrl = NDHttpParser::textToURLcode(path);
		return _sendHttpRequest(m_objhandle, &request, pathUrl.c_str(), host, port, m_bLongConnection);
	}
	else {
		return _sendHttpRequest(m_objhandle, &request, "", host, port, m_bLongConnection);
	}
}

int HttpConnector::Recv(char *buf, int size, int timeout)
{
	ND_TRACE_FUNC();
	return  nd_connector_raw_waitdata( m_objhandle,  buf, size, timeout) ;
}


int HttpConnector::onDataRecv(char *buf, int size)
{
	ND_TRACE_FUNC();
#ifdef ND_DEBUG
	char reserved = buf[size];
	buf[size] = 0;
	nd_log_screen("%s\n", buf);
	buf[size] = reserved;
#endif
	m_response.InData(buf, size);
	if (m_response.CheckParseError()) {
		return -1;
	}

	if (m_response.CheckRecvOk()) {
		onResponse(&m_response);
		if (m_responseOk) {
			m_response.Reset();
			return -1;
		}
	}
	return size;
}

int HttpConnector::Update(int timeout)
{
	ND_TRACE_FUNC();
	char buf[0x10000] ;
	
	ndtime_t start = nd_time() ;
	int lefttime = timeout ;
	
	do {
		int size = Recv(buf, sizeof(buf), lefttime) ;
		if (size <=0){
			return size;
		}
		buf[size] = 0;

		nd_log_screen( "%s\n", buf);
		if (-1 == onDataRecv(buf,size)) {
			return -1;
		}

	}while (lefttime > 0);
	
	return 0;
}

bool HttpConnector::CheckValid()
{
	ND_TRACE_FUNC();
	if (m_objhandle) {
		return nd_connector_valid((nd_netui_handle)m_objhandle) ? true: false ;
	}
	return false;
}

void HttpConnector::onResponse(NDHttpResponse *response)
{
	ND_TRACE_FUNC();

	nd_logdebug("on response success \n");

	//response->dump();
	m_responseOk = true;
}


