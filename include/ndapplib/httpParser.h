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
#include <string>
#include <vector>

class NDHttpRequest ;

class HttpConnector
{
public:
	HttpConnector(NDHttpRequest *request) ;
	virtual ~HttpConnector() ;
	int Create(const char *host, int port) ;
	void Destroy() ;
	

	int SendPostRequest(const char *path, const char* application, const char *dataBody);
	int SendGetRequest(const char *path);
	
	int Recv(char *buf, int size, int timeout) ;
	int Update(int timeout) ;
	bool CheckValid() ;
	
protected:
	
	//NDIConn *m_conn ;
	nd_handle m_conn;
	int m_port ;
	std::string m_host ;
	NDHttpRequest *m_recvRequest ;
};

//
//struct textFound
//{
//public:
//	textFound(){};
//	bool operator ()( const std::string &s1, const std::string &s2 ) const
//	{
//		return  ndstricmp((char*) s1.c_str(), (char*) s2.c_str()) ;
//	}
//};

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
	void Reset() ;
	bool CheckRecvOk() {return 3==m_parseStat;}

protected:
	
	int _parseInit() ;
	int _parseHeader() ;
	int _parseBody() ;
	char *_getCurParseAddr() ;
	int _getDataSize() ;
	int _findBodySize() ;
	
	int _dumRequestHeader() ;
	
	int m_status ;
	int m_bodySize ;
	//typedef std::map<std::string, std::string, textFound> HttpHeader_t ;
	typedef std::vector<httpHeaderNode>HttpHeader_t ;

	HttpHeader_t m_header ;
	std::string m_body ;
	
	std::string m_httpProtocol ;
	
	nd_linebuf m_recvBuf ;
private:
	int m_parseStat ; // 0 empty , 1 start parse ,2 parse body, 3 parse ok
	int m_parseProgress ; // 0 head, 1 body
};

#endif /* defined(__roborts__httpParser__) */
