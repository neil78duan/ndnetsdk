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


template<class keyT> struct httpStringComp
{
	bool operator()(const keyT &l, const keyT &r) const
	{
		return ndstricmp(l.c_str(), r.c_str()) < 0;
	}
};

class NDHttpListener : public NDSafeListener
{
public:
	NDHttpListener(nd_fectory_base *sf = NULL);
	~NDHttpListener();

	void Destroy(int flag);
	bool installRequest_c(const char *pathName, http_reqeust_func func);
	bool installRequest_script(const char *pathName, const char *script);
	int onRequest(const char *reqPath, NDHttpSession *session, const NDHttpRequest &request);
protected:
	virtual int onRequestScript(const char* script, NDHttpSession *session, const NDHttpRequest &request);
	typedef std::map<std::string, ndHttpHandler, httpStringComp<std::string> > requestEntry_map;
	requestEntry_map m_entrys;
};



class NDHttpSession : public NDBaseSession
{
public:
	NDHttpSession();
	virtual~NDHttpSession();
	int SendResponse(NDHttpResponse &response, const char *errorDesc);
	int sendErrorResponse(int errorCdoe, const char *desc);
	void OnCreate();
	int onDataRecv(char *buf, int size, NDHttpListener *pListener);
	
private:
	//void setLongConnect();
	NDHttpRequest m_request;
	bool m_bLongConnect;

};
#endif
