/* file nd_instance.cpp
 * define class ndCInstance
 * 
 * all right reserved by neil duan
 *
 * 2009-4-24 22:41
 */

#if defined(_MSC_VER) 

#pragma comment(lib,"Psapi.lib")

#pragma comment(lib,"Ws2_32.lib")
#include <conio.h>
#include <stdarg.h>
int nd_unhandler_except(struct _EXCEPTION_POINTERS *lpExceptionInfo) ;
#endif
#include "nd_common/nd_common.h"
#include "nd_net/nd_netlib.h"
#include "nd_srvcore/nd_srvlib.h"

#include "ndapplib/nd_instance.h"

#include "ndapplib/nd_cmmgr.h"
#include "ndapplib/nd_listener.h"
#include "ndapplib/nd_session.h"

extern int create_stl_allocator() ;
extern  void destroy_stl_allocator() ;
static int applib_exit_callback(int flag) ;

int instance_tick_entry(void *param) ;
static NDInstanceBase *g_base_inst = NULL ;

// 
// class NDStaticInitHelper{
// public:
// 	NDStaticInitHelper() 
// 	{
// 		nd_common_init() ;
// 		nd_net_init() ;
// 		nd_srvcore_init() ;
// 
// 		//_init_pool_for_new() ;
// 		//create_stl_allocator() ;
// 		nd_log_screen("init common lib end\n") ;
// 		isInited = true;
// 	}
// 
// 	~NDStaticInitHelper()
// 	{
// 		nd_srvcore_destroy() ;
// 		nd_net_destroy() ;
// 
// 		//destroy_stl_allocator() ;
// 		//_destroy_pool_for_new() ;
// 		nd_common_release_ex(1) ;
// 		nd_log_screen("RELEASE common lib end\n") ;
// 		isInited = false;
// 	}
// private:
// 	bool isInited;
// };
// 
// #ifdef _MSC_VER
// #pragma data_seg(push, stack_nd, ".CRT$XIA")
// #define __initdata__ 
// #elif defined(__ND_LINUX__)
// #define __initdata__ __attribute__ (( section(".init.data")))
// #else
// #define __initdata__
// #endif
//
//__initdata__ NDStaticInitHelper _g_static_init_helper ;
// 
// #ifdef _MSC_VER
// #pragma data_seg(pop, stack_nd)
// #end

extern const char * __g_version_desc ;
extern int __g_version_id ; 

NDInstanceBase::NDInstanceBase() 
{
	tminterval = 100 ;
	pListen = 0;
	m_config_name = 0;
	config_file = 0 ;
	m_un_develop = 0 ;
	memset(&m_config,0, sizeof(m_config));	
	m_bNormalExit = 1;
	g_base_inst = this ;
    m_alarm_id = -1 ;
	m_bDaemon = 0 ;
	m_bCreated = 0;
	//char *configname ;
}

NDInstanceBase::~NDInstanceBase() 
{
	g_base_inst = NULL ;
}


int NDInstanceBase::CreateEx(int argc, ...)
{
	int i = 0;
	const char *p[100];
	va_list arg;
	va_start(arg, argc);
	while (argc-- > 0 && i<100) {
		p[i++] = va_arg(arg, const char*);
	}
	va_end(arg);
	return Create(i, p) ;
}
int NDInstanceBase::Create(int argc, const char *argv[])
{
	//get config file 	
	for (int i = 1; i < argc; i++) {
		if (0 == ndstrcmp(argv[i], "-f") && i < argc - 1) {
			NDInstanceBase::config_file = argv[++i];
		}
		else if (0 == ndstrcmp(argv[i], "-c") && i < argc - 1) {
			m_config_name = argv[++i];
		}
		else if (0 == ndstrcmp(argv[i], "-nodev")) {
			m_un_develop = 1;
		}

		else if (0 == ndstrcmp(argv[i], "-log") && i < argc - 1) {
			const char*logfileName = argv[++i];
			ndstrncpy(m_config.i_cfg.log_file, logfileName, sizeof(m_config.i_cfg.log_file));
		}
		else if (0 == ndstrcmp(argv[i], "-workdir") && i < argc - 1) {
			nd_chdir(argv[++i]);
		}
		else if (0 == ndstrcmp(argv[i], "-v") || 0 == ndstrcmp(argv[i], "--version")) {
			ndfprintf(stdout, "version : %s \n", __g_version_desc);
			exit(0);
		}
		else if (0 == ndstrcmp(argv[i], "-l") || 0 == ndstrcmp(argv[i], "--rlimit")) {
			char buf[1024];
			ndfprintf(stdout, "rlimit: \n %s\n", get_rlimit_info(buf, sizeof(buf)));
			exit(0);
		}

		else if (0 == ndstrcmp(argv[i], "-pid") && i < argc - 1) {
			FILE *pf = fopen(argv[i + 1], "w");
			if (pf) {
				ndfprintf(pf, "%d", nd_processid());
				fclose(pf);
			}
			else {
				ndfprintf(stderr, "write pid error %s file not exist\n", argv[i + 1]);
				exit(1);
			}
		}
	}
	nd_arg(argc, argv);
	return _create();
}
int NDInstanceBase::_create()
{
	//int i ;
	const char *logfileName = NULL ;
	if (m_bCreated)	{
		return 0;
	}
    system_signals_init() ;	

	if(!config_file|| !m_config_name) {
		ndprintf("usage: -f config-file -c config-name\n press ANY key to continue\n") ;
		exit(1) ;
	}

	if(-1==ReadConfig(m_config_name) ) {
		ndprintf("Read config %s file error \n press ANY key to continue\n", m_config_name) ;
		exit(1) ;
	}
	
	nd_set_exit_callback(applib_exit_callback) ;

	if (m_config.i_cfg.open_dump){
#ifndef ND_UNIX
		::SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)nd_unhandler_except);
#else
		enable_core_dump() ;
#endif
	}
	size_t max_connector = m_config.l_cfg.max_connect ;
	if ( max_connector > get_maxopen_fd() ) {
		size_t new_fd_num = set_maxopen_fd(max_connector) ;
		nd_logmsg("set max connect %d real max connect =%d\n",m_config.l_cfg.max_connect, (int) new_fd_num);
	}
	
	
	if (m_config.i_cfg.log_file[0]){
		nd_set_logpath_without_date(m_config.i_cfg.log_filename_nodate);
		set_log_file(m_config.i_cfg.log_file) ;
		if (m_config.i_cfg.log_file_size > 0) {
			nd_setlog_maxsize(m_config.i_cfg.log_file_size) ;
		}
		nd_setlog_func(nd_default_filelog) ;
	}

	if (m_config.i_cfg.callstack_file[0]){
		
#ifdef __ND_WIN__
		if (CALLSTACK_INIT(nd_filename(m_config.i_cfg.callstack_file)) == -1) {
#else 
		if (CALLSTACK_INIT(m_config.i_cfg.callstack_file) == -1) {
#endif
			nd_logfatal("create map file %s error!\n" AND m_config.i_cfg.callstack_file);
			return -1 ;
		}
	}
	nd_net_set_crypt((nd_netcrypt)nd_TEAencrypt, (nd_netcrypt)nd_TEAdecrypt, sizeof(tea_v)) ;

	OnCreate();
	m_bCreated = 1;
	return 0 ;

}

void NDInstanceBase::Destroy(int flag)
{
	if (m_bCreated==0){
		return;
	}
	m_bCreated = 0;
	NDAlarm::Destroy();
	
    nd_host_eixt() ;
    if(NDInstanceBase::Close(flag)==-1) {
        return  ;
    }
    DestructListener() ;

	OnDestroy();
}

int NDInstanceBase::Open(int session_size )
{
	ND_TRACE_FUNC() ;
	size_t size = session_size ;
	if (session_size == 0) {
		size = getSessionAllocSize();
	}
	if (pListen){
		Close(1) ;
	}
	//create listener
	pListen = ConstructListener() ;
	if(!pListen) {
		return -1 ;
	}

	if(-1==pListen->Create(m_config.l_cfg.listen_name,m_config.l_cfg.max_connect, size) ) {
		DestructListener() ;
		return -1 ;
	}
	OnListenerCreate() ;
	
	pListen->SetAccept(1) ;
	struct listen_config &pCfg = m_config.l_cfg;

	int ret = pListen->Open(pCfg.port,pCfg.thread_pool_num, pCfg.bind_ip, pCfg.is_ipv6) ;
    if (ret != 0) {
        return  -1 ;
    }

    //set listen config 
	if (m_config.l_cfg.connected_tmout){
		nd_listener_set_timeout(pListen->GetHandle(),m_config.l_cfg.connected_tmout) ;
	}
    
    if (m_config.l_cfg.empty_close_tmout){
        pListen->SetEmptyConnTimeout(m_config.l_cfg.empty_close_tmout) ;
    }
    
    if (m_config.l_cfg.closed_unknown){
        nd_net_set_unregmsg_handler(pListen->GetHandle(),1) ;
    }
    
    if (m_config.l_cfg.cloase_unauthorize){
        nd_net_set_unauthorize_handler(pListen->GetHandle(),1) ;
    }
    
	pListen->m_inst = this ;
    
    
	OnInitilize() ;

	pListen->SetAccept(0) ;

	nd_logmsg("(-: (-:  %s start SUCCESS current TIMEZONE = %d :-)  :-) \n PROGRAM RUNNING .....\n", nd_process_name(), nd_time_zone());
	return ret;
}

int NDInstanceBase::WaitServer()
{
	ND_TRACE_FUNC() ;	
	int ret = wait_services() ;
	int host_error = nd_host_get_error();
	if (host_error == NDERR_HOST_SHUTDOWN || host_error == NDERR_SUCCESS) {
		SetExitCode(0);
	}
	else{
		SetExitCode(1);
	}
    return  ret ;
}


int NDInstanceBase::Close(int force)
{
	ND_TRACE_FUNC() ;

	OnClose();
	if (pListen){
		pListen->Close(force) ;
		pListen->Destroy(force) ;
	}
	return 0;
}


NDListener *NDInstanceBase::ConstructListener() 
{ 
	return new NDListener();	
}

void NDInstanceBase::DestructListener() 
{ 
	if(pListen){delete pListen;	 pListen = 0 ;} 
}

bool NDInstanceBase::CheckReliable(NDSession *psession) 
{
	ndip_t peerip = psession->GetPeerip() ;
	return CheckReliableHost(peerip);
}

bool NDInstanceBase::CheckReliableConn(nd_handle hsession)
{
	ndip_t peerip = nd_sock_getpeerip(((nd_netui_handle)hsession)->fd);
	return CheckReliableHost(peerip);
}

bool NDInstanceBase::CheckReliableHost(ndip_t& peerip) 
{
	ND_TRACE_FUNC();
	for(int i=0; i<m_config.reliable_num; i++) {
		if (0 == nd_sock_cmp_ip(m_config.reliable_hosts[i], peerip, m_config.reliable_ipmask[i])){
#ifdef ND_DEBUG
			char reliable_buf[20],peer_buf[20]/*, mask_buf[20]*/;
			ND_INET_NTOA(m_config.reliable_hosts[i], reliable_buf);
			ND_INET_NTOA(peerip, peer_buf);
			//nd_inet_ntoa(m_config.reliable_ipmask[i], mask_buf);

			nd_logdebug("check ip success reliable=%s peer=%s mask=0x%x\n", reliable_buf, peer_buf, m_config.reliable_ipmask[i]);

#endif
			return true;
		}
	}
	return false ;
}

bool NDInstanceBase::CheckIsDeveVer() 
{
	return m_un_develop?false:true;
}

int NDInstanceBase::GetVersionID() 
{
	return __g_version_id ; 
}
const char *NDInstanceBase::GetVersionDesc() 
{
	return __g_version_desc ;
}

void NDInstanceBase::StartStaticsMem2()
{
#if defined(_MSC_VER)
	GetProcessMemoryInfo(GetCurrentProcess(), &m_Stat, sizeof(m_Stat));
#endif
}

void NDInstanceBase::EndStaticsMem2()
{
#if defined(_MSC_VER)
	_PROCESS_MEMORY_COUNTERS   Stat; 

	GetProcessMemoryInfo(GetCurrentProcess(), &Stat, sizeof(Stat));

	long long _Delta = (long long)Stat.WorkingSetSize - (long long)m_Stat.WorkingSetSize;

#endif
}

#if defined(ND_DEBUG)
#if defined(ND_USE_VLD)
void NDInstanceBase::StartStaticsMem() 
{
	VLDMarkAllLeaksAsReported();
}

void NDInstanceBase::EndStaticsMem() 
{
	VLDReportLeaks();
}
#elif defined(_MSC_VER) 
void NDInstanceBase::StartStaticsMem() 
{
	_CrtMemDumpAllObjectsSince( NULL );
	_CrtMemCheckpoint( &m_s1 );
}
int NDReportHook(int nRptType, char *szMsg,int  *retVal)
{
	const char *RptTypes[] = { "Warning", "Error", "Assert" };
	if ( ( nRptType > 0 ) || ( ndstrstr( szMsg, "HEAP CORRUPTION DETECTED" ) ) )
		nd_logmsg("%s: %s" AND RptTypes[nRptType] AND szMsg );

	retVal = 0;
	return 1;         // Allow the report to be made as usual
}
void NDInstanceBase::EndStaticsMem() 
{
	_CrtMemState  s2, s3;
	_CrtMemCheckpoint( &s2 );
	if ( _CrtMemDifference( &s3, &m_s1, &s2 ) ) {
		_CrtSetReportHook( NDReportHook ) ;
		_CrtMemDumpStatistics( &s3 );
		_CrtSetReportHook( NULL ) ;
	}
}
#else //unix-like
void NDInstanceBase::StartStaticsMem()
{
	if (!CheckIsDeveVer()) {
		return ;
	}
	nd_mm_statics_start() ;
}
void NDInstanceBase::EndStaticsMem()
{
	if (!CheckIsDeveVer()) {
		return ;
	}
	nd_mm_statics_end() ;
	
}
#endif 

#else  //debug
void NDInstanceBase::StartStaticsMem() 
{
	
	if (!CheckIsDeveVer()) {
		return ;
	}
	nd_mm_statics_start() ;
}
void NDInstanceBase::EndStaticsMem() 
{
	if (!CheckIsDeveVer()) {
		return ;
	}
	nd_mm_statics_end() ;

}
#endif 

//添加一个定时器到监听主线程
int NDInstanceBase::AddSysTimer(nd_timer_entry tmfunc, void *param, ndtime_t interval )
{
	nd_handle hl= GetDeftListener()->GetHandle() ;
	if (!hl){
		return -1 ;
	}
	return nd_listener_add_timer(hl,tmfunc,param,interval,ETT_LOOP ) ? 0 : -1;
}
void NDInstanceBase::DelSysTimer(int timerid)
{
	nd_handle hl= GetDeftListener()->GetHandle() ;
	if (!hl){
		return ;
	}
	nd_listener_del_timer(hl,timerid );
}


int NDInstanceBase::EnableAlarm(bool bEnable )
{
    if (bEnable) {
        if (m_alarm_id ==-1) {
            NDAlarm::Create() ;
            m_alarm_id = AddSysTimer(instance_tick_entry, this, 100) ;
        }
    }
    else {
        if (m_alarm_id != -1) {
            DelSysTimer(m_alarm_id) ;
            NDAlarm::Destroy() ;
            m_alarm_id = -1 ;
        }
    }
    return 0;
}

void NDInstanceBase::trytoDaemon() 
{
	if (!m_bDaemon) {
		return ;
	}
#ifdef ND_UNIX
	nd_init_daemon() ;
#endif 
}
//read config from file
int NDInstanceBase::ReadConfig(const char *configname) 
{
	int ret;
	ndxml *xmlroot ;
	ndxml_root xmlfile;

	ret = ndxml_load(config_file, &xmlfile) ;
	if(0!=ret) {
		nd_logfatal("load xml from file\n") ;
		return -1;
	}

	xmlroot = ndxml_getnode(&xmlfile,"root") ;
	if(!xmlroot) {		
		ndxml_destroy(&xmlfile);
		nd_logfatal("xml read error : root-node can not found \n") ;
		return -1;
	}

	if(-1==read_config(xmlroot, configname, &m_config) ) {
		ndxml_destroy(&xmlfile);
		nd_logfatal("xml config error %s node \n", configname) ;
		return -1;
	}

	ndxml_destroy(&xmlfile);
    
	return 0;

}

bool NDInstanceBase::createConnect(const char*name, NDConnector &connector)
{
	ND_TRACE_FUNC();
	NDListener *pl = GetDeftListener();
	nd_assert(pl);
	//ndthread_t thid = pl->GetListenThid();

	//create not connect 
	connect_config *connmgr_cfg = getConnectorInfo(name);
	if (!connmgr_cfg) {
		nd_logerror("can not get  %s connector config\n", name);
		return false;
	}
	if (-1 == connector.Create(connmgr_cfg->protocol_name)) {
		nd_logerror("create %s connector error\n", name);
		return false;
	}
	connector.setName(name);
	pl->Attach(connector);
	return true;
}

int NDInstanceBase::connectServer(const char *connectorName, NDConnector &connector, NDOStreamMsg *firstMsg)
{
	ND_TRACE_FUNC();

	if (-1 == _openConnector(connectorName, &connector)) {
		return -1;
	}
	pListen->Attach(connector);

	int isRegisterOk = 0;
	int size = sizeof(int);

	connector.Ioctl(NDIOCTL_SET_LEVEL, &isRegisterOk, &size);

	if (firstMsg) {
		if (connector.SendMsg(*firstMsg) == -1) {
			return -1;
		}
	}
	onConnecteServer(connector);
	return 0;
}


int NDInstanceBase::_openConnector(const char *name,NDConnector *inconnect)
{
	connect_config *pcof = getConnectorInfo(name) ;
	if (!pcof) {
		return -1;
	}	
	
	if(0 !=inconnect->Open(pcof->host, pcof->port, pcof->protocol_name) ) {
		return -1 ;
	}
	
	
	if (pcof->tmout> 0) {
		inconnect->SetConnectTimeOut(pcof->tmout) ;
	}
	
	inconnect->setName(name);
	return 0;
	
}

connect_config *NDInstanceBase::getConnectorInfo(const char *name)
{
	for (int i=0; i<ND_CONNECT_OTHER_HOSTR_NUM; ++i) {
		if (m_config.i_cfg.connectors[i].port == 0) {
			continue ;
		}
		if (0==ndstricmp(m_config.i_cfg.connectors[i].connector_name, name)) {
			return &m_config.i_cfg.connectors[i] ;
		}
	}
	return NULL ;
}

NDInstanceBase *nd_get_appinst()
{
	return g_base_inst ;
}
#if defined (_MSC_VER)

int set_mp() 
{
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);

	if(SystemInfo.dwNumberOfProcessors <= 1) 
		return 0;
	BOOL ret = SetProcessAffinityMask(GetCurrentProcess(), SystemInfo.dwActiveProcessorMask);
	return ret ? 0 : -1;
}

#include <dbghelp.h>
#pragma comment(lib, "Dbghelp.lib")

int nd_unhandler_except(struct _EXCEPTION_POINTERS *lpExceptionInfo)
{
	nd_msgbox("programe run error,memory dump!", "ERROR") ;
	LONG ret = EXCEPTION_EXECUTE_HANDLER;

	char szFileName[256];
	SYSTEMTIME st;
	::GetLocalTime(&st);
	ndsnprintf(szFileName,256, "%04d-%02d-%02d-%02d-%02d-%02d-%02d-%02d.dmp", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, rand()%100);

	HANDLE hFile = ::CreateFileA(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if (hFile != INVALID_HANDLE_VALUE){

		MINIDUMP_EXCEPTION_INFORMATION ExInfo;
		ExInfo.ThreadId = ::GetCurrentThreadId();
		ExInfo.ExceptionPointers = lpExceptionInfo;
		ExInfo.ClientPointers = false;
		nd_host_set_error(NDERR_HOST_CRASH);
		// write the dump
		BOOL bOK = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL );
		/*
		if (bOK){
			ndprintf("Create Dump File Success!\n");
		}
		else{
			ndprintf("MiniDumpWriteDump Failed: %d\n", GetLastError());
		}
		*/
		::CloseHandle(hFile);
	}
	else
	{
		//ndprintf("Create File %s Failed %d\n", szFileName, GetLastError());
	}
	return ret;
}

#include <iphlpapi.h>
#pragma comment(lib,"Iphlpapi.lib")
int ipaddr_mac_cmp(const char *input_addr) 
{
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;

	pAdapterInfo = (IP_ADAPTER_INFO *) malloc( sizeof(IP_ADAPTER_INFO) );
	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);

	dwRetVal = GetAdaptersInfo( pAdapterInfo, &ulOutBufLen) ;
	if ( dwRetVal != ERROR_SUCCESS) 	{
		free (pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *) malloc (ulOutBufLen);
	}
	if((dwRetVal=GetAdaptersInfo(pAdapterInfo ,&ulOutBufLen))==NO_ERROR){
		pAdapter= pAdapterInfo;
		while(pAdapter)	{
			char mac_buf[128] ;
			char *p = mac_buf ;
			for(UINT i=0;i<pAdapter->AddressLength;i++)	{
				p +=ndsnprintf(p,128 , "%02x-" , pAdapter->Address[i]) ;
			}
			--p ;
			*p = 0 ;
			if (0==ndstricmp((char*)mac_buf,(char*) input_addr)){
				return 0 ;
			}
			pAdapter=pAdapter->Next;
		}
	}
	else {
		nd_logerror("last error  :%s\n" AND nd_last_error()) ;
	}

	free (pAdapterInfo);
	return -1;
}

#elif defined (__ND_LINUX__)

int set_mp() 
{
	return 0 ;
}

#include <net/if.h>
#include <net/if_arp.h>

int ipaddr_mac_cmp(const char *input_addr)
{
	int fd, intrface, ret = -1;
	struct ifreq buf[32];
	struct arpreq arp;
	struct ifconf ifc;
	char ma_buf[128] ;

	if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) >= 0) {
		ifc.ifc_len = sizeof buf;
		ifc.ifc_buf = (caddr_t) buf;
		if (!ioctl (fd, SIOCGIFCONF, (char *) &ifc)) {
			intrface = ifc.ifc_len / sizeof (struct ifreq);
			while (intrface-- > 0){
				if (!(ioctl (fd, SIOCGIFHWADDR, (char *) &buf[intrface])))	{

					ndsnprintf(ma_buf, 128, "%02x-%02x-%02x-%02x-%02x-%02x",
						(unsigned char)buf[intrface].ifr_hwaddr.sa_data[0],
						(unsigned char)buf[intrface].ifr_hwaddr.sa_data[1],
						(unsigned char)buf[intrface].ifr_hwaddr.sa_data[2],
						(unsigned char)buf[intrface].ifr_hwaddr.sa_data[3],
						(unsigned char)buf[intrface].ifr_hwaddr.sa_data[4],
						(unsigned char)buf[intrface].ifr_hwaddr.sa_data[5]);
					if(0==ndstricmp(ma_buf,(char*)input_addr)) {
						ret = 0 ;
						break ;
					}
				}

			}
		} 
		else
			ret = -1;

	} 
	else
		ret = -1;

	close(fd);
	return ret;
}
#else 

int set_mp()
{
    return 0 ;
}

int ipaddr_mac_cmp(const char *input_addr)
{
    return 0;
}
#endif

int applib_exit_callback(int flag)
{
	if (g_base_inst){
		g_base_inst->Destroy(flag) ;
	}
	return 0;
}


int instance_tick_entry(void *param)
{
    ND_TRACE_FUNC() ;
    if(nd_host_check_exit())
        return -1;
    NDInstanceBase *pInst = (NDInstanceBase*) param ;
    pInst->Update() ;
    return 0;
}
