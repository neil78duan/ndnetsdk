//
//  httpParser.h
//  roborts
//
//  Created by duanxiuyun on 15-3-19.
//  Copyright (c) 2015年 duanxiuyun. All rights reserved.
//

#ifndef __roborts__httpParser__
#define __roborts__httpParser__


#include "nd_net/nd_netlib.h"
#include "nd_common/nd_common.h"
#include "ndapplib/applib.h"
#include <string>
#include <vector>
#include <map>



struct httpHeaderNode{
	std::string name;
	std::string value ;
};

class NDHttpParser
{
public:
	NDHttpParser();
	virtual ~NDHttpParser();
	
	virtual void Reset();
	virtual int dump();
	bool CheckRecvOk();
	bool isLongConnect();
	void InData(const char *data, int size);

	int getStatus(){ return m_status; }
	void setStatus(int stat) { m_status = stat; }

	const char *getBody() { return m_body.c_str(); }
	int getBodySize() { return (int)m_body.size(); }
	const char *getHeader(const char *);	
	bool addHeader(const char *name, const char *value);


	size_t HeaderToBuf(char *buf, size_t size);

	enum eAction{ E_ACTION_GET, E_ACTION_POST, E_ACTION_RESPONSE };
	eAction getAction() { return m_action; }
	void setAction(NDHttpParser::eAction act) { m_action = act; }

	typedef std::vector<httpHeaderNode>HttpHeader_t;

protected:
	virtual int ParseProtocol();
	virtual void onParseEnd();
	int ParseData();

	int _parseHeader();
	int _parseBody();

	httpHeaderNode *_getNode(const char *name, HttpHeader_t &headers);
	std::string *_getHeader(const char *name);
	void _adNode(const char *name,const char *value, HttpHeader_t &headers);

	char *_getCurParseAddr();
	int _getDataSize();
	int _findBodySize();

	int m_status;

	nd_linebuf *m_recvBuf;
	int m_parseStat; // 0 empty , 1 start parse ,2 parse body, 3 parse ok
	int m_parseProgress; // 0 head, 1 body

	eAction m_action;
public:
	std::string m_body;
	HttpHeader_t m_header;
};


class NDHttpRequest : public NDHttpParser 
{
public:

	NDHttpRequest() ;
	virtual ~NDHttpRequest() ;
	
	const char *getPath() { return m_path.c_str(); }
	const char* getRequestVal(const char *name); //get header value
	bool addRequestFormVal(const char *name, const char *value);
	size_t RequestValueTobuf(char *buf, size_t size);

	int dump();
	virtual void Reset();
	int _postBodyToJson();
protected:

	typedef std::vector<httpHeaderNode>HttpHeader_t;

	virtual int ParseProtocol();
	virtual void onParseEnd();
	int _parsePathInfo(const char *path);

	
public:

	//HttpHeader_t m_header;
	HttpHeader_t m_requestForms;
	std::string m_path;
	//std::string m_body;
};


class NDHttpResponse : public NDHttpParser
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

class HttpConnector
{
public:
	HttpConnector(bool bLongConnect=false);
	virtual ~HttpConnector();
	int Create(const char *host, int port);
	int Close();
	void Destroy();
	int SendRequest(NDHttpRequest &request, const char *host, int port, const char *path);

	int Recv(char *buf, int size, int timeout);
	int Update(int timeout);
	void setLongConnect(bool bLongConnect){ m_bLongConnection = bLongConnect; }
	bool CheckValid();
	virtual void onResponse(NDHttpResponse *response);
	void setLast(const char *path){ m_lastRequestPath = path; }
	nd_handle getHandle(){ return m_conn; }
protected:

	//NDIConn *m_conn ;
	bool m_bLongConnection;
	int m_port;
	nd_handle m_conn;
	std::string m_host;
	std::string m_lastRequestPath;
	//NDHttpRequest *m_recvRequest;
	NDHttpResponse m_response;
};


class NDHttpSession;
//typedef NDHttpRequest NDHttpResponse;


typedef int(*http_reqeust_func)(NDHttpSession *pSession,const NDHttpRequest &resuest);

class NDHttpSession : public NDBaseSession
{
public:
	NDHttpSession();
	virtual~NDHttpSession();

	int SendResponse(NDHttpResponse &response, const char *errorDesc);

	void OnCreate();
	int onDataRecv(char *buf, int size);

	static bool installRequest(const char *pathName, http_reqeust_func func);

private:

	int sendErrorResponse(int errorCdoe);
	int onRequest(const char *path, const NDHttpRequest &request);
	NDHttpRequest m_request;

	typedef std::map<std::string, http_reqeust_func> requestEntry_map;

	static requestEntry_map m_request_entry;

};

#endif /* defined(__roborts__httpParser__) */
