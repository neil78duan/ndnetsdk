/* file nd_httpListener.h
 *
 * listener of http server
 *
 * create by duan 
 * 
 * 2018.4.10
 */

#ifndef _ND_HTTP_LISTENER_H_
#define _ND_HTTP_LISTENER_H_

#include "ndapplib/nd_listener.h"
#include "ndapplib/httpParser.h"
#include <string>
#include <map>

#define SESSION_DEFAULT_TIMEOUT 1800
#define SESSION_DEFAULT_PATH "/"

class NDHttpSession;
class NDHttpListener;
typedef int(*http_reqeust_func)(NDHttpSession *pSession, const NDHttpRequest &resuest, NDHttpListener *pListener);

struct ndHttpHandler
{
	http_reqeust_func c_func;
	std::string script_name;
	ndHttpHandler() : c_func(0), script_name()
	{

	}
	ndHttpHandler(http_reqeust_func func, const char*script) : c_func(func)
	{
		if (script && *script) {
			script_name = script;
		}
	}
	~ndHttpHandler() 
	{
// 		if (script_name) {
// 			free(script_name);
// 			script_name = 0;
// 		}
	}

	ndHttpHandler& operator = (const ndHttpHandler &r)
	{
		script_name = r.script_name;
		c_func = r.c_func;
		return *this;
	}
};



class SessionIdMgr;
class ND_COMMON_CLASS NDHttpListener : public NDSafeListener
{
	typedef  NDSafeListener myBase;
public:
	NDHttpListener(nd_fectory_base *sf = NULL);
	~NDHttpListener();

	void Destroy(int flag);
	bool installRequest_c(const char *pathName, http_reqeust_func func);
	bool installRequest_script(const char *pathName, const char *script);
	SessionIdMgr *getSessionIdMgr() { return m_cookieSessionIds; }
	virtual int onRequest(const char *reqPath, NDHttpSession *session, const NDHttpRequest &request);

	void setSessionAge(int seconds) { m_sessionAge = seconds; }
	int getSessionAge() { return m_sessionAge; }
	void updateSessionIds();
protected:
	void OnInitilize();		// call on open
	virtual int onRequestScript(const char* script, NDHttpSession *session, const NDHttpRequest &request);
	typedef std::map<std::string, ndHttpHandler, httpStringComp<std::string> > requestEntry_map;
	requestEntry_map m_entrys;

	SessionIdMgr *m_cookieSessionIds;

	NDINT32 m_sessionAge;
};


//typedef httpHeaderNode sessionIdVal;
typedef std::string sessionId_t;

struct sessionValInfo
{
	sessionIdVal val;
	time_t invalidTm;
	std::string path;
};


class ND_COMMON_CLASS  NDHttpSession : public NDBaseSession
{
public:
	NDHttpSession();
	virtual~NDHttpSession();
	int SendResponse(NDHttpResponse &response, const char *errorDesc);
	int SendRedirect(const char *newUrl);
	int sendErrorResponse(int errorCdoe, const char *desc);
	int sendBinaryData(NDHttpResponse &response, void *data, size_t datalen, const char*errorDesc);
	void OnCreate();
	int onDataRecv(char *buf, int size, NDHttpListener *pListener);

	void setDelayClosed(bool bRightnow=true);
	virtual int UpdateSecond();				//update per second

	//http session functions 
	bool sessionIdTrytoSet(const char *clientSendSid);
	bool sessionIdCreate(int lifeOfSeconds= SESSION_DEFAULT_TIMEOUT, const char *path=NULL);
	sessionId_t sessionIdGet();
	bool sessionIdSetValue(const char *name, const char *value);
	std::string sessionIdGetValue(const char*name);
	bool sessionIdGetInfo(sessionValInfo &info);
	void setSessionAge(int seconds) { m_sessionAge = seconds; }
	int getSessionAge();
	static const char *getServerInfo();
	static void setServerInfo(const char *info);

protected:
	static std::string s_serverInfo;
	void _preOnHandle();
	int _getMyCookie(char *buf, size_t size);
	SessionIdMgr * _getSessoinIdMgr();
	int getWaitTimeout();
	NDHttpRequest m_request;
	ndtime_t m_closedTime;
	sessionId_t m_cookieSessionId;
	NDINT32 m_sessionAge;		// 0 clear cookie right now , -1 cleared when close
};

//this is traditional session

class ND_COMMON_CLASS  SessionIdMgr
{
public:


	SessionIdMgr();
	~SessionIdMgr();

	sessionId_t CreateSessionId(const sessionIdVal &val, int lifeOfSeconds,const char *path);
	bool DestroySessionId(const sessionId_t &sid);
	bool GetSessionIdVal(const sessionId_t  &sid, sessionValInfo &outval);
	bool SaveSessionIdValue(const sessionId_t  &sid, const char *name, const char*val);
	void Update();
private:
	bool BuildSessionId(sessionId_t &sid);
	typedef std::map<sessionId_t, sessionValInfo> sessionData_t;

	sessionData_t m_data;
	nd_mutex m_lock;
};


#endif
