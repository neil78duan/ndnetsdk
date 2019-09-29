//
//  httpParser.h
//  roborts
//
//  Created by duanxiuyun on 15-3-19.
//  Copyright (c) 2015 duanxiuyun. All rights reserved.
//

#ifndef __roborts__httpParser__
#define __roborts__httpParser__


//#include "nd_net/nd_netlib.h"
#include "nd_common/nd_common.h"
//#include "ndapplib/applib.h"
#include "nd_common/nd_export_def.h"
#include "ndapplib/nd_object.h"
#include <string>
#include <vector>
#include <map>

#define ND_DFT_SESSION_ID_NAME "apoSessionId"
template<class keyT> struct httpStringComp
{
	bool operator()(const keyT &l, const keyT &r) const
	{
		return ndstricmp(l.c_str(), r.c_str()) < 0;
	}
};
typedef std::map<std::string, std::string, httpStringComp<std::string> >sessionIdVal;

struct httpHeaderNode{
	std::string name;
	std::string value ;
};

class  ND_COMMON_CLASS  NDHttpParser : public NDIBaseObj
{
public:
	NDHttpParser();
	virtual ~NDHttpParser();
	
	virtual void Reset();
	virtual int dump();
	bool CheckRecvOk();
	bool isLongConnect();
	void InData(const char *data, int size);

	int getStatus()const{ return m_status; }
	void setStatus(int stat) { m_status = stat; }

	void setBody(const char*body);
	const char *getBody();
	int getBodySize();

	const char *getHeader(const char *) const;
	int getHeaderSize() const {	return (int)m_header.size();	}
	const char* getHeaderVal(int index)const;
	const char* getHeaderName(int index)const;

	bool addHeader(const char *name, const char *value);

	size_t HeaderToBuf(char *buf, size_t size);

	enum eAction{ E_ACTION_GET, E_ACTION_POST, E_ACTION_RESPONSE };
	eAction getAction() { return m_action; }
	void setAction(NDHttpParser::eAction act) { m_action = act; }
	bool CheckParseError() { return m_parseError; }

	static std::string textToURLcode(const char *text,bool isLittle= false);

	static std::string URLcodeTotext(const char *urlCode);

	static bool getFileRange(const char *range, size_t &offset, size_t &length, size_t fileSize);

	typedef std::vector<httpHeaderNode>HttpHeader_t;
	void setParseStepIndex(int stepIndex) { m_parseStat = stepIndex; }
protected:

	virtual int Create(const char *name = 0);
	virtual void Destroy(int flag = 0);

	virtual int ParseProtocol();
	virtual void onParseEnd();

	void _setParseEnd();
	int _parseHeader();
	int _parseBody();
	int ParseData();

	httpHeaderNode *_getNode(const char *name, HttpHeader_t &headers);
	std::string *_getHeader(const char *name);
	void _adNode(const char *name,const char *value, HttpHeader_t &headers);

	char *_getCurParseAddr();
	int _getDataSize();
	int _findBodySize();

	bool m_parseError;
	int m_status;		//200 is ok ,404 not found
	//nd_linebuf *m_recvBuf;
	int m_parseStat; // 0 empty , 1 start parse ,2 parse body, 3 parse ok
	int m_parseProgress; // 0 head, 1 body

	eAction m_action;
	nd_linebuf m_bodyBuf;
public:
	//std::string m_body;
	HttpHeader_t m_header;
};


class ND_COMMON_CLASS  NDHttpRequest : public NDHttpParser
{
public:
	NDHttpRequest() ;
	virtual ~NDHttpRequest() ;
	
	const char *getPath() { return m_path.c_str(); }
	const char* getRequestVal(const char *name); //get header value
	//size_t getRequestBodySize();
	bool addRequestFormVal(const char *name, const char *value);
	size_t RequestValueTobuf(char *buf, size_t size);

	int dump();
	virtual void Reset();
	int _postBodyToJson();

	struct fileCacheInfo
	{
		std::string fileName;
		size_t size;
		char *dataAddr;
		fileCacheInfo() :size(0), dataAddr(0) {}
	};

	const fileCacheInfo *getUploadFile(const char *filename)const;

protected:
	int _parseMultipart(const char *pHeaderText);
	int _parseOnePart(const char *partStart, size_t len);
	int _parse_x_form();
	virtual int ParseProtocol();
	virtual void onParseEnd();
	int _parsePathInfo(const char *path);
	int _parseCookies();

	bool _insertFile(const char *varname, const char*filePath, void *data, size_t length);
	void _destroyUpFile();
public:


	typedef std::map<std::string, fileCacheInfo > fileCacheMap_t;
	typedef std::vector<httpHeaderNode>HttpHeader_t;
	fileCacheMap_t m_upfiles;

	HttpHeader_t m_requestForms;
	std::string m_path;
	void *m_userData;
	sessionIdVal m_cookies;
};


class ND_COMMON_CLASS  NDHttpResponse : public NDHttpParser
{
public:
	NDHttpResponse() : NDHttpParser()
	{
		m_action = NDHttpParser::E_ACTION_RESPONSE;
	}
	~NDHttpResponse()
	{

	}

protected:
	virtual int ParseProtocol();

};

class  ND_COMMON_CLASS  HttpConnector : public NDObject
{
public:
	HttpConnector(bool bLongConnect=false);
	virtual ~HttpConnector();

	int Create(const char *instantName);
	int Open(const char *host, int port);
	int Close(int flag = 0);
	void Destroy(int flag = 0);
	int SendRequest(NDHttpRequest &request, const char *host, int port, const char *path);

	int onDataRecv(char *buf, int size);
	int Recv(char *buf, int size, int timeout);
	int Update(int timeout);
	void setLongConnect(bool bLongConnect){ m_bLongConnection = bLongConnect; }
	bool CheckValid();
	virtual void onResponse(NDHttpResponse *response);
	void setLast(const char *path){ m_lastRequestPath = path; }
	bool CheckResponsed() {	return m_responseOk;}
	void setHttps(bool enable = false) { m_bSSL = enable; }
	bool getHttps() { return m_bSSL; }
	//nd_handle getHandle(){ return m_conn; }
protected:
	void setResponseSuccess() { m_responseOk = true; }
	bool m_responseOk;
	bool m_bLongConnection;
	bool m_bSSL;
	int m_port;
	std::string m_host;
	std::string m_lastRequestPath;
	NDHttpResponse m_response;
};
int _sendHttpResponse(nd_handle h, NDHttpResponse *reques, const char *errorDesc,const char *serverInfo=NULL);

ND_APPLIB_API bool NDHttpGet(HttpConnector &conn, const char *url);

#endif /* defined(__roborts__httpParser__) */
