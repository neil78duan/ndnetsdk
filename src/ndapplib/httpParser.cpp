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
// 
// int parser_fetch_data(char *src, char *outstr, int size)
// {
// 	unsigned char a ;
// 	char *_init = src ;
// 	while(*src && --size >=0) {
// 		a = (unsigned char)*src ;
// 		if(a > 0x20 ){
// 			*outstr++ = *src++ ;
// 		}
// 		else {
// 			break ;
// 		}
// 	}
// 	*outstr = 0 ;
// 	return (int)(src - _init);
// }

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

static int _sendHttpRequest(nd_handle h, NDHttpRequest *reques, const char *path, const char *host, int port,bool bLongConnect)
{
	int ret = 0;
	int len, bodySize;
	char *p;
	char buf[4096];
	char bodyBuf[0x10000];
	char *formBuf = bodyBuf;

	bodyBuf[0] = 0;
	buf[0] = 0;

	
	bodySize = (int)reques->RequestValueTobuf(bodyBuf, sizeof(bodyBuf));
	if (bodySize == 0 && reques->m_body.size() > 0 && NDHttpRequest::E_ACTION_POST){
		bodySize = (int) reques->m_body.size();
		formBuf = (char*) reques->m_body.c_str();
	}
	
	p = buf;

	if (NDHttpRequest::E_ACTION_POST == reques->getAction()) {
		len = snprintf(p, sizeof(buf), "POST /%s HTTP/1.1\r\nHost: %s:%d\r\n", path, host, port);
		p += len;

		len = snprintf(p, sizeof(buf) - (p - buf), "Accept: */*\r\n");
		p += len;
	}

	else {
		if (bodySize > 0){
			len = snprintf(p, sizeof(buf), "GET /%s?%s HTTP/1.1\r\nHost: %s:%d\r\n", path, formBuf, host, port);
			p += len;
		}
		else {
			len = snprintf(p, sizeof(buf), "GET /%s HTTP/1.1\r\nHost: %s:%d\r\n", path, host, port);
			p += len;
		}
		bodySize = 0;
	}
	
	len =(int) reques->HeaderToBuf(p, sizeof(buf) - (p - buf));
	p += len; 

	len = snprintf(p, sizeof(buf) - (p - buf), "Content-Type:application/x-www-form-urlencoded;charset=utf-8\r\n"
		"Content-Length:%d\r\nConnection:Keep-Alive\r\n\r\n", bodySize);
	p += len;

	//write header to send buf
	len = nd_connector_send_stream(h, buf, p - buf,0);
	if (len <= 0) {
		return len;
	}
	ret += len;
	
	if (NDHttpRequest::E_ACTION_POST == reques->getAction() && bodySize) {
		//len = snprintf(p, sizeof(buf) - (p - buf), "%s\r\n\r\n", formBuf);
		//p += len;

		len = nd_connector_send_stream(h, formBuf, bodySize, 0);
		if (len <= 0) {
			return len;
		}
		ret += len;


		len = nd_connector_send_stream(h, "\r\n\r\n",4, 0);
		if (len <= 0) {
			return len;
		}
		ret += len;
	}

	nd_tcpnode_flush_sendbuf_force((nd_netui_handle)h);
	return 0;
}

int _sendHttpResponse(nd_handle h, NDHttpResponse *reques, const char *errorDesc)
{
	int ret = 0;
	int bodySize = reques->getBodySize();;
	int len;
	char *p;
	char buf[0x10000];

	p = buf;

	len = snprintf(p, sizeof(buf), "HTTP/1.1 %d %s \r\n", reques->getStatus(), errorDesc);
	p += len;

	len =(int) reques->HeaderToBuf(p, sizeof(buf) -(p - buf));
	p += len;

	len = snprintf(p, sizeof(buf) - (p - buf),"Server:userDefine\r\nContent-Length:%d\r\nConnection: Keep-Alive\r\n\r\n", bodySize);
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
	unsigned char y;
	if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
	else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
	else if (x >= '0' && x <= '9') y = x - '0';
	else {}
	return y;
}


std::string NDHttpParser::textToURLcode(const char *text, bool isLittle )
{
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

//////////////////////////////////////////////////////////////////////////

NDHttpParser::NDHttpParser()
{
	m_recvBuf = NULL;
	Reset();
}

NDHttpParser::~NDHttpParser()
{

	if (m_recvBuf){
		ndlbuf_destroy(m_recvBuf);
		delete m_recvBuf;
		m_recvBuf = NULL;
	}
	Reset();

}

void NDHttpParser::Reset()
{
	//ndlbuf_reset(&m_recvBuf);
	if (m_recvBuf){
		ndlbuf_reset(m_recvBuf);
	}
	
	m_parseStat = 0;
	m_parseProgress = 0;
	m_header.clear();
	m_body.clear();
	
	//m_requestForms.clear();
	//m_path.clear();

	m_action = E_ACTION_GET;
	m_status = 200;
	//m_bodySize = 0;
}
bool NDHttpParser::CheckRecvOk()
{
	return (3 == m_parseStat || (2 == m_parseStat && m_action == E_ACTION_GET));
}

bool NDHttpParser::isLongConnect()
{
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
	if (m_recvBuf==NULL){
		m_recvBuf = new nd_linebuf;
		ndlbuf_init(m_recvBuf, 0x10000);
	}

	ndlbuf_write(m_recvBuf, (char*)data, size, 0);
	while (m_parseStat < 3 && ParseData() > 0) {
		if (_getDataSize() == 0) {
			break;
		}
	}
}


int NDHttpParser::dump()
{
	int ret = 0;
	HttpHeader_t::iterator it;
	for (it = m_header.begin(); it != m_header.end(); ++it) {
		++ret;
		fprintf(stderr, "[header] %s : %s \n", it->name.c_str(), it->value.c_str());
	}
	
	if (m_body.size())	{
		fprintf(stderr, "%s\n", m_body.c_str());
	}
	return  ret;
}

const char *NDHttpParser::getHeader(const char *name)const
{
	httpHeaderNode *pNode = ((NDHttpParser*)this)->_getNode(name, (HttpHeader_t&)m_header);
	if (pNode)	{
		return pNode->value.c_str();
	}
	return NULL;
}

const char* NDHttpParser::getHeaderVal(int index)const
{
	if (index < m_header.size()) {
		return m_header[index].value.c_str();
	}
	return NULL;
}
const char* NDHttpParser::getHeaderName(int index)const
{
	if (index < m_header.size()) {
		return m_header[index].name.c_str();
	}
	return NULL;
}

bool NDHttpParser::addHeader(const char *name, const char *value)
{
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
	if (0 == m_parseStat) {
		return ParseProtocol();
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


char *NDHttpParser::_getCurParseAddr()
{
	if (!m_recvBuf)	{
		return NULL;
	}
	if (ndlbuf_datalen(m_recvBuf)) {
		return (char*)ndlbuf_data(m_recvBuf);
	}
	return NULL;

}

int NDHttpParser::_getDataSize()
{
	if (!m_recvBuf)	{
		return 0;
	}
	return (int)ndlbuf_datalen(m_recvBuf);
}


std::string *NDHttpParser::_getHeader(const char *name)
{
	httpHeaderNode *pNode = _getNode(name, m_header);
	if (pNode)	{
		return &(pNode->value);
	}
	return NULL;
}

httpHeaderNode *NDHttpParser::_getNode(const char *name, HttpHeader_t &headers)
{
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
	const char *p = getHeader("content-length");
	if (p) {
		return atoi(p);
	}
	return 0;
}

int NDHttpParser::_parseHeader()
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


		if (*p == '\r' && *(p + 1) == '\n') {
			//ndlbuf_sub_data(&m_recvBuf, 2);
			p += 2;
			m_parseStat = 2;
			//_dumRequestHeader() ;
			if (_findBodySize() == 0) {
				m_parseProgress = 3;//without body

			}
			if (m_action == E_ACTION_GET ){
				m_parseStat = 3;
			}
		}

		len = (int)(p - p_start);
		ndlbuf_sub_data(m_recvBuf, len);

		return len;
	}

	return 0;


}
int NDHttpParser::_parseBody()
{

	int contentSize = _findBodySize();
	int datasize = _getDataSize();
	char *p_start = _getCurParseAddr();

	if (!p_start) {
		return 0;
	}
	char *p = p_start;

	if (contentSize > datasize)		{
		return 0;
	}
	if (contentSize){
		for (int i = 0; i < contentSize; i++){
			m_body.push_back(*p);
			++p;
		}
	}
	else {
		p = strstr(p_start, "\r\n\r\n");
		if (!p)	{
			return 0;
		}

		char ch = *p;
		*p = 0;

		m_body = p_start;
		*p = ch;

	}
	//p += 4;

	ndlbuf_sub_data(m_recvBuf, (p - p_start));

	//m_bodySize = m_body.size();
	m_parseStat = 3;

	onParseEnd();
// 	if (m_action == E_ACTION_POST && m_body.size()){
// 		_postBodyToJson();
// 	}
	return (int)(p - p_start);

}


size_t NDHttpParser::HeaderToBuf(char *buf, size_t size)
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
////////////////////
//http parser

NDHttpRequest::NDHttpRequest() :NDHttpParser(), m_userData(0)
{
}
NDHttpRequest:: ~NDHttpRequest()
{

}
// 
// void NDHttpRequest::InData(const char *data, int size)
// {
// 	ndlbuf_write(&m_recvBuf, (char*)data, size, 0);
// 	while (m_parseStat < 3 && ParseData() > 0) {
// 		if (_getDataSize() == 0) {
// 			break;
// 		}
// 	}
// }
// 
// int NDHttpRequest::ParseData()
// {
// 	if (0 == m_parseStat) {
// 		return _parseInit();
// 	}
// 	else if (1 == m_parseStat) {
// 		return _parseHeader();
// 	}
// 	else if (2 == m_parseStat){
// 		if (m_action == E_ACTION_GET){
// 
// 			m_parseStat = 3;
// 			return 0;
// 		}
// 		return _parseBody();
// 	}
// 	return 0;
// }
// 
// int NDHttpRequest::OnEnd()
// {
// 
// 	return 0;
// }



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

	return (int)(p - path );
}



int NDHttpRequest::dump()
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
// 
// char *NDHttpRequest::_getCurParseAddr()
// {
// 	if (ndlbuf_datalen(&m_recvBuf)) {
// 		return (char*)ndlbuf_data(&m_recvBuf);
// 	}
// 	return NULL;
// 
// }
// 
// int NDHttpRequest::_getDataSize()
// {
// 	return (int)ndlbuf_datalen(&m_recvBuf);
// }

void NDHttpRequest::Reset()
{
	NDHttpParser::Reset();
	m_path.clear();
	m_requestForms.clear();
	m_userData = NULL;
}

// 
// bool NDHttpRequest::CheckRecvOk()
// {
// 	return (3 == m_parseStat || (2 == m_parseStat && m_action == E_ACTION_GET));
// }

int NDHttpRequest::ParseProtocol()
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

		ndlbuf_sub_data(m_recvBuf, len);
		m_parseStat = 1;
		return len;
	}

	return 0;

// 	if (p) {
// 		len = parser_fetch_data(p, buf, datasize - (int)(p - p_start));
// 		if (len >= datasize) {
// 			return  0;
// 		}
// 		p += len;
// 		//get status
// 		//m_httpProtocol = buf ;
// 
// 		m_status = (int)strtol(p, &p, 10);
// 		if (!p) {
// 			return 0;
// 		}
// 
// 		if (parser_check_end(p, &p, datasize - (int)(p - p_start))) {
// 			//parse prorocol success
// 			len = (int)(p - p_start);
// 
// 			ndlbuf_sub_data(&m_recvBuf, len);
// 			m_parseStat = 1;
// 			return len;
// 		}
// 
// 	}
	return 0;

}


void NDHttpRequest::onParseEnd()
{
	if (m_action == E_ACTION_POST && m_body.size()){
		_postBodyToJson();
	}
}

const char* NDHttpRequest::getRequestVal(const char *name)
{
	httpHeaderNode *pNode = _getNode(name, m_requestForms);
	if (pNode)	{
		return pNode->value.c_str();
	}
	return NULL;
}

bool NDHttpRequest::addRequestFormVal(const char *name, const char *value)
{
	_adNode(name, value, m_requestForms);
// 	httpHeaderNode node1 = { name, value };
// 	m_requestForms.push_back(node1);
	return true;
}


size_t NDHttpRequest::RequestValueTobuf(char *buf, size_t size)
{
	char *p = buf;
	for (size_t i = 0; i < m_requestForms.size(); i++)	{
		if (m_requestForms[i].value.size() ==0)		{
			continue;
		}
		const httpHeaderNode &node = m_requestForms[i];
		int len = 0;

		if (i == m_requestForms.size() - 1)	{
			len = snprintf(p, size, "%s=%s", node.name.c_str(), node.value.c_str());
		}
		else {
			len = snprintf(p, size, "%s=%s&", node.name.c_str(), node.value.c_str());
		}
		p += len;
		size -= len;
	}
	return  p - buf;
}


int NDHttpRequest::_postBodyToJson()
{
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
	return -1;
}


int NDHttpRequest::_parseMultipart(const char *pHeaderText)
{
	pHeaderText = ndstristr(pHeaderText, "boundary=");

	if (!pHeaderText ) {
		return -1;
	}

	char val[1024];
	char buf[1024];
	pHeaderText += 9;
	std::string WebKitFormName = pHeaderText;

	const char *p = m_body.c_str();
	p = ndstr_first_valid(p);

	while (p && *p) {
		buf[0] = 0;
		val[0] = 0;
		p = ndstristr(p, WebKitFormName.c_str());
		if (!p || !*p) {
			break;
		}

		p += WebKitFormName.size();
		p = ndstristr(p, "name=\"");
		if (!p) {
			break;
		}
		p += 6;

		p = ndstr_nstr_ansi(p, buf, '\"', sizeof(buf));
		if (!p || *p != '\"') {
			break;
		}
		++p;
		p = ndstr_first_valid(p);
		if (!p) {
			break;
		}
		p = ndstr_nstr_end(p, val, '\r', sizeof(val));


		if (buf[0] && val[0]) {
			httpHeaderNode node = { buf, val };
			m_requestForms.push_back(node);
		}
	}
	return (int)m_body.size();
}

int NDHttpRequest::_parse_x_form()
{

	char val[1024];
	char buf[1024];

	const char *p = m_body.c_str();
	p = ndstr_first_valid(p);

	do {
		val[0] = 0;
		buf[0] = 0;
		p = ndstr_nstr_ansi(p, buf, '=', sizeof(buf));
		if (!p || !*p || *p != '=') {
			return 0;
		}
		++p;
		p = ndstr_nstr_ansi(p, val, '&', sizeof(val));
		if (p && *p == '&') {
			++p;
		}

		if (buf[0]) {
			httpHeaderNode node = { buf, val };
			m_requestForms.push_back(node);
		}

	} while (p && *p);

	return (int)m_body.size();
}

//////////////////////////////////////////////////////////////////////////

int NDHttpResponse::ParseProtocol()
{
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
		p = strchr(p, ' ');
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

			ndlbuf_sub_data(m_recvBuf, len);
			m_parseStat = 1;
			return len;
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////

static int _http_connector_data_handler(nd_handle sessionHandler, void *data, size_t len, nd_handle )
{
	if (!data || len == 0) {
		return 0;
	}

	HttpConnector *pConn = dynamic_cast<HttpConnector *> (NDObject::FromHandle(sessionHandler));
	if (!pConn) {
		return -1;
	}
	return pConn->onDataRecv((char*)data, len);
}

HttpConnector::HttpConnector(bool bLongConnect) : m_port(0), m_bLongConnection(bLongConnect), m_responseOk(false)
{
	
}

HttpConnector:: ~HttpConnector()
{
	Destroy() ;
}

int HttpConnector::Create(const char *instantName)
{
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
	((nd_netui_handle)m_objhandle)->user_data = (void*)this;
	if (instantName && *instantName) {
		nd_object_set_instname(m_objhandle, instantName);
	}
	return 0;
}

int HttpConnector::Open(const char *host, int port)
{
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
	
	return 0 ;
	
}

int HttpConnector::Close(int flag)
{
	if (m_objhandle)	{
		nd_connector_close(m_objhandle, flag);
	}
	OnClose();
	return 0;
}

void HttpConnector::Destroy(int flag)
{
	if (m_objhandle) {
		nd_object_destroy(m_objhandle, flag) ;
		//DestroyConnectorObj(m_objhandle) ;
		m_objhandle = 0 ;
	}
	OnDestroy();
}


int HttpConnector::SendRequest(NDHttpRequest &request, const char *host, int port, const char *path)
{
	m_lastRequestPath = path;
	std::string pathUrl = NDHttpParser::textToURLcode(path);
	return _sendHttpRequest(m_objhandle, &request, pathUrl.c_str(), host, port,m_bLongConnection);
}

int HttpConnector::Recv(char *buf, int size, int timeout)
{
	return  nd_connector_raw_waitdata( m_objhandle,  buf, size, timeout) ;
}


int HttpConnector::onDataRecv(char *buf, int size)
{
#ifdef ND_DEBUG
	char reserved = buf[size];
	buf[size] = 0;
	nd_log_screen("%s\n", buf);
	buf[size] = reserved;
#endif
	m_response.InData(buf, size);

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
// 		m_response.InData(buf, size);
// 		
// 		if (m_response.CheckRecvOk()){
// 			onResponse(&m_response);
// 			m_response.Reset();
// 			//if (!m_bLongConnection)	{
// 			//	Close();
// 			//}
// 			break;
// 		}

	}while (lefttime > 0);
	
	return 0;
}

bool HttpConnector::CheckValid()
{
	
	if (m_objhandle) {
		return nd_connector_valid((nd_netui_handle)m_objhandle) ? true: false ;
	}
	return false;
}

void HttpConnector::onResponse(NDHttpResponse *response)
{

	nd_logdebug("on response success \n");

	response->dump();
	m_responseOk = true;
}


