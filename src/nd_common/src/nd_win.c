/* file nd_win.c
 * define function in windows platform
 * version  1.0 
 *
 * author : neil duan 
 * all right reserved  2007-9-27 
 */
 
#include "nd_common/nd_common.h"
#if !defined(ND_UNIX) 
#include <tchar.h>
#include <crtdbg.h>

#ifdef _DEBUG
#define _CRTTRAC(msg) do { if ((1 == _CrtDbgReport(_CRT_WARN, NULL, 0, NULL, "%s", msg))) _CrtDbgBreak(); } while (0)
#else 
#define _CRTTRAC(msg)
#endif


ndpid_t nd_createprocess(const char *working_path, const char *path,...)
{
	// get args
	BOOL ret;
	const char *p;
	ndpid_t pid = 0;
	va_list va;

	char cmdbuf[1024];
	char tmp_path[ND_FILE_PATH_SIZE];

	char *pstr = cmdbuf;

	int len = snprintf(cmdbuf, sizeof(cmdbuf), "%s", path);
	pstr += len;

	va_start(va, path);
	while ((p = va_arg(va, const char*))) {
		len = snprintf(pstr, sizeof(cmdbuf) - (pstr - cmdbuf), " %s", p);
		pstr += len;
	}
	va_end(va);

	if (path || cmdbuf[0]) {
		STARTUPINFO si = { 0 };
		PROCESS_INFORMATION pi = { 0 };
		si.cb = sizeof(si);

		//si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = TRUE;

		if (nd_path_is_relative(path)) {
			path = nd_absolute_filename(path, tmp_path, sizeof(tmp_path));
		}

		ret = CreateProcessA(path, cmdbuf, NULL, NULL, FALSE,  CREATE_NEW_CONSOLE, NULL, working_path, &si, &pi);
		if (FALSE == ret) {
			nd_logerror("create process error %s\n", nd_last_error());
			return 0;
		}
		nd_logdebug("create process %s success \n", path);
		return pi.dwProcessId;
	}
	return 0;
}


void nd_terminate_process(ndpid_t pid)
{
	HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
	if (hProcess) {
		TerminateProcess(hProcess, 0);
		CloseHandle(hProcess);
	}
}

const char *nd_get_sys_username()
{
	DWORD length ;
	static char s_username[256] = {0};
	if (s_username[0]) {
		return s_username ;
	}
	length =(DWORD)sizeof(s_username);
	GetUserNameA(s_username, &length);
	if(length==0) {
		nd_logerror("get system user name error :%s\n", nd_last_error()) ;
		return NULL ;
	}
	return s_username ;
}

//message box for error 
DWORD _ErrBox(char *file, int line)
{
	const char *perrdesc=nd_last_error() ;
	char buf[4096]={0} ;
	ndsnprintf(buf,4096,"Runtime ERROR in:%s %d line\n %s ",
		file,line,perrdesc);
	MessageBoxA(GetActiveWindow(), buf, "error", MB_OK);
	DebugBreak();
	return 0;
}
const char *nd_str_error(int errcode) 
{
	HLOCAL hlocal = NULL;   // Buffer that gets the error message string
	DWORD dwError = errcode;
	char *pstrNoErr ="Error number not found." ;
	static char buf_desc[1024] ;
	BOOL fOk ;

	if(0==dwError) {
		ndstrncpy(buf_desc,pstrNoErr,sizeof(buf_desc));
		return buf_desc ;
	}
	// Get the error code's textual description
	fOk = FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, 
		NULL, dwError, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), 
		(PTSTR) &hlocal, 0, NULL);

	if (!fOk) {
		// Is it a network-related error?
		HMODULE hDll = LoadLibraryEx(TEXT("netmsg.dll"), NULL, 
			DONT_RESOLVE_DLL_REFERENCES);

		if (hDll != NULL) {
			FormatMessage(
				FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_FROM_SYSTEM,
				hDll, dwError, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
				(PTSTR) &hlocal, 0, NULL);
			FreeLibrary(hDll);
		}
	}

	if (hlocal != NULL) {
		ndstrncpy(buf_desc,(char *) LocalLock(hlocal),sizeof(buf_desc));
		LocalFree(hlocal);
	} else {
		ndstrncpy(buf_desc,pstrNoErr,sizeof(buf_desc));
	}
	return buf_desc ;
}
const char *nd_last_error()
{
	return nd_str_error((int)GetLastError() );
}
char* get_rlimit_info(char *buf, int buf_size) 
{
	ndstrncpy(buf, "unknow", buf_size) ;
	return buf;
}

HINSTANCE nd_dll_load(const char *dllpath)
{
	return LoadLibrary(dllpath);
}
void nd_dll_unload(HINSTANCE hdll)
{
	FreeLibrary(hdll);
}
void* nd_dll_entry(HINSTANCE hdll, const char *name)
{
	return (void*)GetProcAddress(hdll, name);
}


#ifdef ND_OPEN_TRACE
int MyDbgReport(const char *file, int line, const char *stm, ...)
{
	char buf[1024], *p = buf ;
	va_list arg;
	int done;

	if(file) {
		ndsprintf(p, "%s ", file) ;
		p += ndstrlen(p) ;
	}
	if(line) {
		ndsprintf(p, "%d ", line) ;
		p += ndstrlen(p) ;
	}

	va_start (arg, stm);
	done = vsprintf (p, stm, arg);
	va_end (arg);

	_CRTTRAC(buf) ;
	return 0;
}
#endif

int nd_getcpu_num() 
{
	SYSTEM_INFO sinf;
	GetSystemInfo(&sinf);
	return sinf.dwNumberOfProcessors ;
}

#pragma comment (lib, "Winmm.lib") 
ndtime_t nd_time(void) 
{
	return timeGetTime() ;
}
//create thread 

WINBASEAPI BOOL WINAPI SwitchToThread(VOID );
/*create thread function
 * return thread handle 
 * if return 0 or NULL failed .
 * but in linux success return 1 else return 0

 *@func : thread function 
 *@param: thread function parameter
 *@thid : output thread id
 */

ndth_handle nd_createthread(NDTH_FUNC func, void* param,ndthread_t *thid,int priority)
{
	DWORD dwID = 0 ;
	HANDLE h = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, param, 0 ,&dwID) ;
	if(h && thid) {
		*thid = dwID ;				
		if(priority==NDT_PRIORITY_HIGHT) {
			SetThreadPriority(h,THREAD_PRIORITY_ABOVE_NORMAL) ;
		}
		else if(NDT_PRIORITY_LOW) {
			SetThreadPriority(h,THREAD_PRIORITY_BELOW_NORMAL) ;
		}
	}
	return h ;
}
int nd_threadsched(void) 
{
	BOOL ret = SwitchToThread() ;
	return ret ? 0:-1 ;
}

void nd_threadexit(int exitcode)
{
	ExitThread(exitcode) ;
}
//wait a thread exit
int nd_waitthread(ndth_handle handle) 
{
	DWORD dwRet = WaitForSingleObject(handle,INFINITE) ;
	if(dwRet==WAIT_OBJECT_0) {
		return 0 ;
	}
	else {
		nd_logerror("wait handle error: %s", nd_last_error());
		return -1 ;
	}
}

int nd_terminal_thread(ndth_handle handle,int exit_code)
{
	BOOL ret =TerminateThread(handle,(DWORD)exit_code) ;
	return ret?0:-1;
}


int create_filemap(const char *filename,const char *map_name, size_t size,nd_filemap_t *out_handle) 
{
	PVOID pvFile ;
	HANDLE hFile =INVALID_HANDLE_VALUE,hFileMap ;

	// Open the file for reading and writing.

	if (filename){
		hFile = CreateFileA(filename, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, 
			NULL, CREATE_ALWAYS , FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			return -1 ;
		}
	}
	
	hFileMap = CreateFileMappingA(hFile, NULL, PAGE_READWRITE, 0, (DWORD)size, map_name);

	if (hFileMap == NULL) {
		CloseHandle(hFile);
		return -1;
	}

	// Get the address where the first byte of the file is mapped into memory.
	pvFile = MapViewOfFile(hFileMap, FILE_MAP_WRITE, 0, 0, 0);

	if (pvFile == NULL) {
		CloseHandle(hFileMap);
		CloseHandle(hFile);
		return -1;
	}

	out_handle->hFile = hFile ;
	out_handle->hFileMap = hFileMap ;
	out_handle->paddr = pvFile ;
	out_handle->size = size ;

	return 0;
}

int close_filemap(nd_filemap_t *mapinfo)
{
	nd_assert(mapinfo) ;
	nd_assert(mapinfo->hFile && mapinfo->hFileMap && mapinfo->paddr ) ;
	UnmapViewOfFile(mapinfo->paddr);
	mapinfo->paddr = 0 ;

	CloseHandle(mapinfo->hFileMap);
	mapinfo->hFileMap = 0;

	if (mapinfo->hFile !=INVALID_HANDLE_VALUE){

		SetFilePointer(mapinfo->hFile, (long)mapinfo->size, NULL, FILE_BEGIN);
		SetEndOfFile(mapinfo->hFile);
		CloseHandle(mapinfo->hFile);
		mapinfo->hFile = 0;
	}
	return 0;
}

//
int open_filemap(char *map_name, nd_filemap_t *out_handle) 
{
	out_handle->hFile = INVALID_HANDLE_VALUE ;

	out_handle->hFileMap = OpenFileMappingA(FILE_MAP_READ | FILE_MAP_WRITE,FALSE, map_name);

	if (out_handle->hFileMap == NULL) {
		return -1;
	}

	out_handle->paddr = MapViewOfFile(out_handle->hFileMap,FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	if(!out_handle->paddr) {
		return -1 ;
	}

	out_handle->size = GetFileSize(out_handle->hFileMap, NULL);

	return 0;
}


//
int open_filemap_r(char *map_name, nd_filemap_t *out_handle)
{
	out_handle->hFile = INVALID_HANDLE_VALUE;

	out_handle->hFileMap = OpenFileMappingA(FILE_MAP_READ , FALSE, map_name);

	if (out_handle->hFileMap == NULL) {
		return -1;
	}

	out_handle->paddr = MapViewOfFile(out_handle->hFileMap, FILE_MAP_READ , 0, 0, 0);
	if (!out_handle->paddr) {
		return -1;
	}

	out_handle->size = GetFileSize(out_handle->hFileMap, NULL);

	return 0;
}


int nd_mem_share_create(const char *name, size_t size, nd_filemap_t *map_handle)
{
	if (0 == open_filemap(name, map_handle)){
		return 0;
	}
	if (0 == size) {
		return -1;
	}
	if (-1 == create_filemap(NULL, name, size, map_handle)) {
		return -1;
	}
	memset(map_handle->paddr, 0, map_handle->size);
	return 0;
}

int nd_mem_share_close(nd_filemap_t *map_handle)
{
	return close_filemap(map_handle);
}


int nd_get_sys_callstack(char *buf, size_t size)
{
	return ndsnprintf(buf, size, "call-stack-unknow\n") ;
}

int nd_sys_callstack_dump(nd_out_func func,FILE *outfile)
{
	return func(outfile,"%s\n", "call-stack-unknow\n") ;
	
}
#endif
	
