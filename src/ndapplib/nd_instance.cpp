/* file nd_instance.cpp
 * define class ndCInstance
 * 
 * all right reserved by neil duan
 *
 * 2009-4-24 22:41
 */

#if !defined(ND_UNIX) 

#if defined(ND_DEBUG)
#pragma comment(lib,"srv_libs_dbg.lib")
#else
#pragma comment(lib,"srv_libs.lib")
#endif

#pragma comment(lib,"Psapi.lib")

#pragma comment(lib,"Ws2_32.lib")
#include <conio.h>
int nd_unhandler_except(struct _EXCEPTION_POINTERS *lpExceptionInfo) ;
#endif
#include "nd_common/nd_common.h"
#include "nd_net/nd_netlib.h"
#include "nd_srvcore/nd_srvlib.h"

//#include "ndapplib/applib.h"

#include "ndapplib/nd_instance.h"

#include "ndapplib/nd_cmmgr.h"
#include "ndapplib/nd_listener.h"
#include "ndapplib/nd_session.h"
//#include "ndapplib/nd_allocator.h"
//#include "pg_config.h"

extern int create_stl_allocator() ;
extern  void destroy_stl_allocator() ;
static int applib_exit_callback(int flag) ;

int instance_tick_entry(void *param) ;
static NDInstanceBase *g_base_inst = NULL ;

//srv_config NDInstanceBase::srvcfg ={0};

//#define HANDLE_UNIX_SIGNAL 1        //handle unix signal

class NDStaticInitHelper{
public:
	NDStaticInitHelper() 
	{
// #if defined( ND_USE_GPERF)
// 		__tcmalloc() ;
// #endif 
		nd_common_init() ;
		nd_net_init() ;
		nd_srvcore_init() ;

		_init_pool_for_new() ;
		create_stl_allocator() ;
		nd_log_screen("init common lib end\n") ;
	}

	~NDStaticInitHelper()
	{
		nd_srvcore_destroy() ;
		nd_net_destroy() ;

		destroy_stl_allocator() ;
		_destroy_pool_for_new() ;
		nd_common_release() ;
		nd_log_screen("RELEASE common lib end\n") ;
	}
};
#ifdef _MSC_VER
#pragma init_seg(".CRT$XCB")
#define __initdata__ 
#elif defined(__LINUX__)
#define __initdata__ __attribute__ (( section(".init.data")))
#else
#define __initdata__
#endif
__initdata__ NDStaticInitHelper _g_static_init_helper ;

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
	//char *configname ;
}

NDInstanceBase::~NDInstanceBase() 
{
	g_base_inst = NULL ;
}

int NDInstanceBase::Create(int argc,const char *argv[])
{
	int i ;
	const char *logfileName = NULL ;

    system_signals_init() ;
    
	nd_arg(argc, argv);	
	
	//get config file 	
	for (i=1; i<argc; i++){
		if(0 == strcmp(argv[i],"-f" ) && i< argc-1) {
			NDInstanceBase::config_file = argv[++i] ;
		}
		else if(0== strcmp(argv[i], "-c") && i< argc-1) {
			m_config_name = argv[++i] ;
		}
		else if(0==strcmp(argv[i],"-nodev")) {
			m_un_develop = 1;
		}
		
		else if(0==strcmp(argv[i],"-log") && i<argc -1) {
			logfileName = argv[++i];
		}

        else if(0==strcmp(argv[i],"-pid") && i<argc -1){
            FILE *pf = fopen(argv[i+1], "w") ;
            if (pf) {
                fprintf(pf, "%d", nd_processid()) ;
                fclose(pf) ;
            }
            else {
                fprintf(stderr,"write pid error %s file not exist\n", argv[i+1]) ;
                exit(1) ;
            }
        }
	}

	if(!config_file|| !m_config_name) {
		printf("usage: -f config-file -c config-name\n press ANY key to continue\n") ;
		//getch() ;
		exit(1) ;
		//return -1 ;
	}

	if(-1==ReadConfig(m_config_name) ) {
		printf("Read config %s file error \n press ANY key to continue\n", m_config_name) ;
		//getch() ;
		exit(1) ;
	}
	
	nd_set_exit_callback(applib_exit_callback) ;

#ifndef ND_UNIX
	if (m_config.i_cfg.open_dump){
		::SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)nd_unhandler_except);
	}
#endif 
	if(logfileName && logfileName[0]) {
		strncpy(m_config.i_cfg.log_file, logfileName, sizeof(m_config.i_cfg.log_file)) ;
	}
	if (m_config.i_cfg.log_file[0]){
		set_log_file(m_config.i_cfg.log_file) ;
		if (m_config.i_cfg.log_file_size > 0) {
			nd_setlog_maxsize(m_config.i_cfg.log_file_size) ;
		}
		nd_setlog_func(nd_default_filelog) ;
	}

	if (m_config.i_cfg.callstack_file[0]){
		if(CALLSTACK_INIT(m_config.i_cfg.callstack_file)==-1) {
			nd_logfatal("create map file %s error!\n" AND m_config.i_cfg.callstack_file) ;
			return -1 ;
		}
	}

	nd_net_set_crypt((nd_netcrypt)nd_TEAencrypt, (nd_netcrypt)nd_TEAdecrypt, sizeof(tea_v)) ;
	
// 	if (g_base_inst==NULL){
// 		g_base_inst = this ;
// 	}

	OnCreate();

	return 0 ;

}

void NDInstanceBase::Destroy(int flag)
{
	OnDestroy();
    
    nd_host_eixt() ;
    if(NDInstanceBase::Close(flag)==-1) {
        return  ;
    }
    DestructListener() ;

}

int NDInstanceBase::Open(int session_size )
{
	ND_TRACE_FUNC() ;
	size_t size = session_size ;
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
	pListen->SetAccept(1) ;
	int ret = pListen->Open(m_config.l_cfg.port,m_config.l_cfg.thread_pool_num) ;
    if (ret != 0) {
        return  -1 ;
    }

    //set listen config 
	if (m_config.l_cfg.connected_tmout){
		nd_set_connection_timeout(pListen->GetHandle(),m_config.l_cfg.connected_tmout) ;
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
	return ret;
}

int NDInstanceBase::WaitServer()
{
	ND_TRACE_FUNC() ;	
	int ret = wait_services() ;
    SetExitCode(0) ;
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
	ndip_t peerip = nd_net_peer_getip(hsession) ;
	return CheckReliableHost(peerip);
}

bool NDInstanceBase::CheckReliableHost(ndip_t peerip) 
{
	ND_TRACE_FUNC();
	for(int i=0; i<MAX_RELIABLE_HOST; i++) {
		if(0==nd_sock_cmp_ip(m_config.reliable_hosts[i], peerip, m_config.reliable_ipmask[i]) )
			return true ;

	}
	return false ;
}

bool NDInstanceBase::CheckIsDeveVer() 
{
	return m_un_develop?false:true;
}

void NDInstanceBase::StartStaticsMem2()
{
#if defined(_WIN32)
	GetProcessMemoryInfo(GetCurrentProcess(), &m_Stat, sizeof(m_Stat));
#endif
}

void NDInstanceBase::EndStaticsMem2()
{
#if defined(_WIN32)
	_PROCESS_MEMORY_COUNTERS   Stat; 

	GetProcessMemoryInfo(GetCurrentProcess(), &Stat, sizeof(Stat));

	long long _Delta = (long long)Stat.WorkingSetSize - (long long)m_Stat.WorkingSetSize;

#endif
}

#if !defined(ND_UNIX) && defined(ND_DEBUG)
void NDInstanceBase::StartStaticsMem() 
{
	_CrtMemDumpAllObjectsSince( NULL );
	_CrtMemCheckpoint( &m_s1 );
}
int NDReportHook(int nRptType, char *szMsg,int  *retVal)
{
	char *RptTypes[] = { "Warning", "Error", "Assert" };
	if ( ( nRptType > 0 ) || ( strstr( szMsg, "HEAP CORRUPTION DETECTED" ) ) )
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
#else 
void NDInstanceBase::StartStaticsMem() 
{

}
void NDInstanceBase::EndStaticsMem() 
{

}
#endif 

//添加一个定时器到监听主线程
int NDInstanceBase::AddSysTimer(nd_timer_entry tmfunc, void *param, ndtime_t interval )
{
	nd_handle hl= GetDeftListener()->GetHandle() ;
	if (!hl){
		return -1 ;
	}
	return nd_listensrv_timer(hl,tmfunc,param,interval,ETT_LOOP ) ? 0 : -1;
}
void NDInstanceBase::DelSysTimer(int timerid)
{
	nd_handle hl= GetDeftListener()->GetHandle() ;
	if (!hl){
		return ;
	}
	nd_listensrv_del_timer(hl,timerid );
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

NDInstanceBase *getbase_inst() 
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
	snprintf(szFileName,256, "%04d-%02d-%02d-%02d-%02d-%02d-%02d-%02d.dmp", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, rand()%100);

	HANDLE hFile = ::CreateFileA(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if (hFile != INVALID_HANDLE_VALUE){

		MINIDUMP_EXCEPTION_INFORMATION ExInfo;
		ExInfo.ThreadId = ::GetCurrentThreadId();
		ExInfo.ExceptionPointers = lpExceptionInfo;
		ExInfo.ClientPointers = false;

		// write the dump
		BOOL bOK = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL );
		/*
		if (bOK){
			printf("Create Dump File Success!\n");
		}
		else{
			printf("MiniDumpWriteDump Failed: %d\n", GetLastError());
		}
		*/
		::CloseHandle(hFile);
	}
	else
	{
		//printf("Create File %s Failed %d\n", szFileName, GetLastError());
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
				p +=snprintf(p,128 , "%02x-" , pAdapter->Address[i]) ;
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

#elif defined (__LINUX__)

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

					snprintf(ma_buf, 128, "%02x-%02x-%02x-%02x-%02x-%02x",
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


MSG_ENTRY_INSTANCE(nd_transfer_to_session)
{
	ND_TRACE_FUNC() ;
	NDUINT16 sid ;
	NDIStreamMsg inmsg(msg) ;
	
	//nd_logdebug("receive message need transfer\n");
	if(-1==inmsg.Read(sid) || sid ==0) {
		
		nd_logmsg("transfer-message error sessionid ==0\n") ;
		return 0 ;
	}
	if(!h_listen){
		h_listen = getbase_inst()->GetDeftListener()->GetHandle() ;
		nd_assert(h_listen) ;
	}

	if(-1==nd_netmsg_handle(sid,(nd_usermsghdr_t*)msg, h_listen) ){
		nd_logmsg("nd_netmsg_handle() to %d error\n", sid) ;		
	}
	return 0 ;
}

MSG_ENTRY_INSTANCE(nd_get_message_name_handler)
{
	ND_TRACE_FUNC() ;
	NDUINT16 sid ;
	NDIStreamMsg inmsg(msg) ;
	NDOStreamMsg omsg(inmsg.MsgMaxid(),inmsg.MsgMinid()) ;
	NDUINT16 maxID, minID ;
	
	if (-1==inmsg.Read(maxID)) {
		return 0 ;
	}
	if (-1==inmsg.Read(minID)) {
		return 0 ;
	}
	const char *p = nd_msgentry_get_name(h_listen, maxID, minID) ;
	
	omsg.Write(maxID) ;
	
	omsg.Write(minID) ;
	
	omsg.Write((NDUINT8*)p) ;
	
	ND_MSG_SEND(nethandle, omsg.GetMsgAddr(),  h_listen) ;
	return 0;
}

