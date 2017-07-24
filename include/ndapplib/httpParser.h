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

class NDHttpRequest
{
public:

	NDHttpRequest() ;
	virtual ~NDHttpRequest() ;
	
	void InData(const char *data, int size) ;
	int ParseData() ;
	
	virtual int OnEnd() ;
	
	const char *getHeader(const char *) ;
	int getStatus() ;
	void setStatus(int stat) {m_status = stat ;} 
	const char *getBody() ;
	int getBodySize();
	void Reset() ;
	bool CheckRecvOk();

	bool addHeader(const char *name, const char *value);

	const char* getRequestVal(const char *name);

	size_t HeaderToBuf(char *buf, size_t size);
	size_t RequestValueTobuf(char *buf, size_t size);


	enum eAction{E_ACTION_GET, E_ACTION_POST};

	eAction getAction() { return m_action; }
	void setAction(NDHttpRequest::eAction act) { m_action = act; }
	const char *getPath() { return m_path.c_str(); }
	int _dumRequestHeader();
protected:

	typedef std::vector<httpHeaderNode>HttpHeader_t;
	int _parseInit() ;
	int _parseHeader() ;
	int _parseBody() ;
	int _parsePathInfo(const char *path);
	char *_getCurParseAddr() ;
	int _getDataSize() ;
	int _findBodySize() ;
	
	std::string *_getHeader(const char *name, HttpHeader_t &header);

	
	int m_status ;
	int m_bodySize ;
	
	nd_linebuf m_recvBuf ;
	int m_parseStat ; // 0 empty , 1 start parse ,2 parse body, 3 parse ok
	int m_parseProgress ; // 0 head, 1 body

	eAction m_action;
public:

	HttpHeader_t m_header;
	HttpHeader_t m_requestForms;
	std::string m_path;
	std::string m_body;
};


class HttpConnector
{
public:
	HttpConnector(NDHttpRequest *request);
	virtual ~HttpConnector();
	int Create(const char *host, int port);
	void Destroy();
	int SendRequest(NDHttpRequest &request, const char *host, int port, const char *path);

	int Recv(char *buf, int size, int timeout);
	int Update(int timeout);
	bool CheckValid();

protected:

	//NDIConn *m_conn ;
	nd_handle m_conn;
	int m_port;
	std::string m_host;
	NDHttpRequest *m_recvRequest;
};

class NDHttpSession;
typedef NDHttpRequest NDHttpResponse;

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
