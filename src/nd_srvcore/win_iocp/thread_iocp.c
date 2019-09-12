/* file : thread_iocp.c
 * implement windows iocp model using  CreateIoCompletionPort
 *
 * user create thread watching I/O 
  *
 * version 
 * all right reserved by neil duan 
 * 2007-11
 */


#include "nd_srvcore/nd_srvlib.h"
#include "nd_iocp.h"
//#include "nd_common/nd_alloc.h"

#ifdef USER_THREAD_POLL

HANDLE g_hIOCompletionPort ;
HANDLE *g_hWorkerThreads;
int g_max_worker_threads ;
//WSAEVENT g_hAcceptEvent;		//Network Event for Accept

static  volatile int __n_exit = 0;

static int initialize_iocp();
static DWORD WINAPI WorkerThread(LPVOID lpParam);
static void DeInitialize();
WSAOVERLAPPED g_overlapped ;
//把节点acceptEx好之后放到完成端口句柄中
static struct nd_session_iocp * _pre_accept(struct listen_contex *listen_info)
{
	struct nd_session_iocp *iocp_map;
	struct nd_session_tcp *client = (struct nd_session_tcp *)
		listen_info->tcp.conn_manager.alloc(listen_info->tcp.cm_alloctor) ;
	if(!client)
		return NULL ;
	iocp_map = list_entry(client, struct nd_session_iocp,__client_map) ;

	if(-1==nd_init_iocp_client_map(iocp_map, get_listen_fd(listen_info)) )
		return NULL ;
	return iocp_map;
}

int thread_iocp_entry(struct listen_contex *listen_info)
{
	ndsocket_t listen_fd;
	int ret = -1;
	nd_handle context ;
//	struct nd_session_iocp *acc_client;

	nd_assert(listen_info) ;
	
	listen_fd = get_listen_fd(listen_info) ;
	
	context = nd_thsrv_gethandle(listen_info->listen_id) ;

	//Create I/O completion port
	g_hIOCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0 );
	if(!g_hIOCompletionPort) {
		nd_showerror() ;
		return -1;
	}
	
	//acc_client = _pre_accept(listen_info) ;
	//if(0==acc_client)
	//	goto QUIT;

	if(!CreateIoCompletionPort((HANDLE)listen_fd, g_hIOCompletionPort, (DWORD)&g_overlapped, 0 )) {
		nd_showerror() ;
		goto QUIT;
	}

	if(-1==initialize_iocp() ){
		goto QUIT;
	}

	pre_accept(&listen_info->tcp) ;
	
	//nd_atomic_set(&_listened_num,num) ;

	while (!nd_thsrv_isexit(context)){

		if(-1==nd_thsrv_msghandler(context) ){		//处理线程消息
			break ;
		}
		
		//handle all connect
		if(0==update_iocp_cm(listen_info) )
			nd_sleep(1000);
	}
	PostQueuedCompletionStatus(g_hIOCompletionPort, 0, (DWORD) NULL, NULL);
	__n_exit = 1 ;
	nd_sleep(1000);
	ret = 0;
QUIT:
	CloseHandle(g_hIOCompletionPort);
	DeInitialize();
	
	return 0;
}

//Function to Initialize IOCP
int initialize_iocp()
{
	int i;
	DWORD dwThreadID;
	SYSTEM_INFO sysInfo;

	ZeroMemory(&sysInfo,sizeof(SYSTEM_INFO));

	GetSystemInfo(&sysInfo);

	g_max_worker_threads = sysInfo.dwNumberOfProcessors * 2;
	if(g_max_worker_threads<1)
		g_max_worker_threads = 2 ;
	
	g_hWorkerThreads = (HANDLE*)malloc (sizeof(HANDLE)*g_max_worker_threads) ;

	if ( NULL == g_hIOCompletionPort)  {
		nd_showerror() ; 
		return -1;
	}

	//Create worker threads
	for (i = 0; i < g_max_worker_threads; i++){
		g_hWorkerThreads[i] = CreateThread(0, 0, WorkerThread, NULL, 0, &dwThreadID);
		if(!g_hWorkerThreads[i]) {
			return -1 ;
		}
	}

	return 0;
}


void DeInitialize()
{
	int i;
	//Cleanup IOCP.
	if(g_hIOCompletionPort){
		CloseHandle(g_hIOCompletionPort);
		g_hIOCompletionPort = NULL;
	}

	//Create worker threads
	for (i = 0; i < g_max_worker_threads; i++){
		if(g_hWorkerThreads[i]) {
			nd_waitthread(g_hWorkerThreads[i]) ;
			nd_close_handle(g_hWorkerThreads[i]);
			g_hWorkerThreads[i] = 0;
		}
	}
}

//Worker thread will service IOCP requests
DWORD WINAPI WorkerThread(LPVOID lpParam)
{
	void *lpContext = NULL;
	OVERLAPPED       *pOverlapped = NULL;
	DWORD            dwBytesTransfered = 0;
	DWORD			 dwErrCode;
	//Worker thread will be around to process requests, until a Shutdown event is not Signaled.
	while (0==__n_exit)	{
		BOOL bReturn = GetQueuedCompletionStatus(
			g_hIOCompletionPort,
			&dwBytesTransfered,
			(LPDWORD)&lpContext,
			&pOverlapped,
			INFINITE);

		if (NULL == lpContext)  {
			//We are shutting down
			break;
		}
		if(FALSE == bReturn) {
			dwErrCode = GetLastError() ;
		}
		else {
			dwErrCode = 0 ;
		}
		nd_assert(pOverlapped!= &g_overlapped) ;
		iocp_callback(dwErrCode, dwBytesTransfered, pOverlapped) ;
	}	//end while

	return 0;
}
#endif 
