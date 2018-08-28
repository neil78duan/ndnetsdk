/* file  nddir.h
 *
 * define direct 
 * 
 * create by duan 
 *
 * 2012/9/10 11:00:03
 *
 */

#include "nd_common/nddir.h"
#include "nd_common/nd_os.h"
#include "nd_common/nd_str.h"


static char _current_dir[ND_FILE_PATH_SIZE] = {0} ; //current dir 
static char _original_dir[ND_FILE_PATH_SIZE] = {0} ; // program start dir / original dir 

size_t nd_get_file_size(const char *file)
{
	size_t size =0 ;
	FILE *fp;

	fp = fopen(file, "rb") ;
	if(!fp) {
		return 0;
	}
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	fclose(fp) ;

	return size ;
}

//merge path and file to full-path
char* nd_full_path(const char*in_path, const char *in_file, char *outbuf, size_t size)
{
	size_t len = 0;
	if (!in_file) {
		return NULL;
	}

	if(in_path &&*in_path) {
		len =strlen(in_path);
	}

	if (len == 0)	{
		snprintf(outbuf, size,"%s/%s", nd_getcwd(), in_file);
	}
	else {
		if (in_path[len - 1] == '/' || in_path[len - 1] == '\\') {
			snprintf(outbuf, size, "%s%s", in_path, in_file);
		}
		else {
#if  defined(ND_UNIX) 
			snprintf(outbuf, size, "%s/%s", in_path, in_file);
#else 
			snprintf(outbuf, size, "%s\\%s", in_path, in_file);
#endif
		}
	}
	return outbuf;
}

void nd_unload_file(void *file_data)
{
	free(file_data) ;
}

void* nd_load_file(const char *file, size_t *size)
{
	size_t data_len,buf_size ;
	FILE *fp;
	char *buf= NULL ;

	fp = fopen(file, "rb") ;
	if(!fp) {
		//nd_logerror("open file %s : %s\n", file, nd_last_error());
		return 0;
	}
	fseek(fp, 0, SEEK_END);
	buf_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if(buf_size==0) {
		fclose(fp) ;
		//nd_logmsg("load file %s is empty\n", file);
		return NULL ;
	}
	buf_size += 1 ;
	buf =(char*) malloc(buf_size) ;

	if(!buf){
		fclose(fp) ;
		return 0 ;
	}
	data_len = fread(buf,1, buf_size, fp) ;
	if(data_len==0 || data_len>= buf_size) {
		fclose(fp) ;
		free(buf) ;
		return 0 ;

	}
	buf[data_len] = 0 ;
	fclose(fp) ;
	if (size) {
		*size = data_len ;
	}
	return (void*) buf ;
}

#if defined(ND_UNIX) 

const char * nd_getcwd()
{
	if (_current_dir[0]) {
		return _current_dir ;
	}
	return getcwd(_current_dir, ND_FILE_PATH_SIZE);
}

int nd_chdir(const char *dir)
{
	if (0==chdir(dir)) {
		getcwd(_current_dir, ND_FILE_PATH_SIZE);
		return 0 ;
	}
	return -1;
}
const char * nd_get_init_dir()
{
	if (_original_dir[0]==0)	{
		return getcwd(_original_dir, ND_FILE_PATH_SIZE);
	}
	return _original_dir ;
}

int nd_cpfile(const char *oldfile,const  char *newfile)
{
	int ret = 0 ;
	int readsize;
	FILE *fp1, *fp2 ;
	char buf[1024] ;
	fp1 = fopen(oldfile,"rb");
	if(fp1==NULL){
		return -1;
	}

	fp2 = fopen(newfile,"wb");
	if(fp2==NULL){
		fclose(fp1);
		return -1;
	}

	while((readsize = fread(buf,1,sizeof(buf),fp1))>0){
		if (readsize != sizeof(buf)) {
			int err = ferror(fp1) ;
			if (err) {
				ret = -1 ;
				break ;
			}
		}
		fwrite(buf,1,readsize,fp2);
		if (feof(fp1)) {
			break ;
		}
	}

	fclose(fp1);
	fclose(fp2);
	if (ret !=0) {
		nd_rmfile(newfile) ;
	}
	return ret;
}

int nd_mkfile(const char *file)
{
	FILE *fp1 = fopen(file,"wb");
	if(!fp1)
		return -1;

	fclose(fp1);
	return 0 ;
}


int nd_existfile(const char *pachfilename)
{
	if (access(pachfilename, R_OK) < 0)	{
		return 0 ;
	}
	return 1;
}

const  char* nd_getsysdir()
{
	return "/" ;
}

#else 
//得到工作目录
const char * nd_getcwd()
{
	if (_current_dir[0]) {
		return _current_dir ;
	}
	GetCurrentDirectory( sizeof(_current_dir),(LPTSTR)_current_dir);
	return _current_dir ;
}

const char * nd_get_init_dir()
{
	if (_original_dir[0]==0)	{
		
		GetCurrentDirectory( sizeof(_original_dir),(LPTSTR)_original_dir);
	}
	return _original_dir ;
}

//删除文件
int nd_rmfile(const char *file)
{
	return DeleteFile((LPCTSTR)file) ? 0 :-1 ;
}

//重命名文件
int nd_renfile(const char *oldfile,const  char *newfile)
{
	return MoveFile((LPTSTR)oldfile, (LPTSTR)newfile) ? 0 : -1;
}

//改变工作目录
int nd_chdir(const char *dir)
{
	if(SetCurrentDirectory((LPCTSTR)dir) ) {			
		GetCurrentDirectory( sizeof(_current_dir),(LPTSTR)_current_dir);
		return 0;
	}
	return -1;
}

//copy 文件
int nd_cpfile(const char *oldfile, const char *newfile)
{
	return CopyFile( (LPCTSTR) oldfile, (LPCTSTR) newfile,TRUE) ? 0 : -1;

}

//创建一个目录
int nd_mkdir(const char *dir)
{
	return CreateDirectory((LPCTSTR)dir,NULL) ? 0 : -1;
}

//删除一个目录
int nd_rmdir(const char *dir)
{
	return  RemoveDirectory((LPCTSTR)dir) ? 0 : -1;
}

//得到系统目录
const char* nd_getsysdir()
{
	static char _system_dir[ND_FILE_PATH_SIZE] ;
	GetSystemDirectory((LPTSTR)_system_dir,sizeof(_system_dir));
	return _system_dir ;
}

//查找指定的文件是否存在
int nd_existfile(const char *pachfilename)
{
	WIN32_FIND_DATAA FindFileData;
	
	HANDLE hFind;
	hFind = FindFirstFileA((LPCSTR) pachfilename, &FindFileData);

	if (hFind == INVALID_HANDLE_VALUE) {
		return 0 ;
	} else {
		FindClose(hFind);
		return 1 ;
	}
}


//创建一个新文件
//可以读写,独占,如果存在则失败
int nd_mkfile(const char *file)
{
	HANDLE hfile= CreateFile(
		(LPCTSTR)file,                         // file name
		GENERIC_WRITE|GENERIC_READ,                      // access mode
		FILE_SHARE_READ,                          // share mode
		NULL, // SD
		CREATE_NEW,                // how to create
		FILE_ATTRIBUTE_NORMAL,                 // file attributes
		NULL                       // handle to template file
		);
	if(INVALID_HANDLE_VALUE==hfile) {
		return 0 ;
	}
	else {
		CloseHandle(hfile) ;
		return 1 ;
	}
}

#endif

const char * nd_filename(const char *filenamePath)
{
	const char * ret = filenamePath;
	const char *p = filenamePath;
	while (*p) {
		if (*p == '/' || *p == '\\') {
			ret = p + 1;
		}
		++p;
	}

	return ret;
}

const char * nd_getpath(const char *filenamePath, char *pathbuf, size_t size)
{
	const char *end = nd_filename(filenamePath);
	if (end != filenamePath){
		int len = 0;
		--end;
		if (*end == '/' || *end == '\\')	{
			len = end - filenamePath;
			len = NDMIN(len, size);
			memcpy(pathbuf, filenamePath, len);
			pathbuf[len] = 0;
			return pathbuf;
		}
		
	}
	return NULL;
}

#if  defined(ND_UNIX) 
static __INLINE__ int __is_equal_char(char a, char b)
{
	return (a == b);
}

#else
static int __is_equal_char(char a, char b)
{
	if (IS_BIG_LATIN(a)) {
		a = BIG_2_LITTLE(a);
	}
	if (IS_BIG_LATIN(b)) {
		b = BIG_2_LITTLE(b);
	}

	if (a == b) {
		return 1;
	}
	else if (a == '/' && b == '\\') {
		return 1;
	}
	else if (b == '/' && a == '\\') {
		return 1;
	}
	return 0;
}

#endif

int __path_depth(const char *mypath)
{
	int ret = 1;
	while (*mypath)	{
		if (*mypath == '/' || *mypath == '\\') {
			if (*(mypath + 1)){
				++ret;
			}
		}
		++mypath;
	}
	return ret;
}

const char *nd_relative_path(const char *fullPath, const char *workPath, char *buf, size_t bufsize)
{
	const char *srcpath = fullPath;
	const char *pos = fullPath;
	char *p = buf;
	int depth = 0;
	int i;

	*p = 0;
	if (!workPath)	{
		strncpy(buf, fullPath, bufsize);
		goto _EXIT;
	}

	//find same path index 
	while (*fullPath){

		if (!__is_equal_char(*fullPath, *workPath))	{
			break; 
		}

		if (*fullPath == '/' || *fullPath == '\\') {
			++depth;
			pos = fullPath + 1;
		}
		++fullPath;
		++workPath;
	}

	if (pos == srcpath){
		strncpy(buf, fullPath, bufsize);
		goto _EXIT;
	}

	depth = __path_depth(workPath);

	for (i = 0; i < depth; i++){
		strncat(p, "../", bufsize);
		p += 3;
		bufsize -= 3;
	}
	if (*pos){
		strncat(p, pos, bufsize);
	}

_EXIT:
	p += strlen(p) -1;

	if (*p == '/' || *p == '\\') {
		*p= 0;
	}
	return buf;
}

const char * nd_absolute_path(const char *relative_path, char *outbuf, size_t bufsize)
{
	char tmp_buf[ND_FILE_PATH_SIZE];
	const char *wdir = nd_getcwd();
	
	strncpy(tmp_buf, wdir, sizeof(tmp_buf));
	if (-1 == nd_chdir(relative_path)) {
		return NULL;
	}

	wdir = nd_getcwd();
	strncpy(outbuf, wdir, bufsize) ;
	nd_chdir(tmp_buf);
	return outbuf;
}

const char * nd_absolute_filename(const char *relative_file, char *outbuf, size_t bufsize)
{
	char tmp_buf[ND_FILE_PATH_SIZE];
	if (!nd_getpath(relative_file, tmp_buf,sizeof(tmp_buf)) ) {
		return NULL;
	}
	if (!nd_absolute_path(tmp_buf, tmp_buf, sizeof(tmp_buf))) {
		return NULL;
	}
	return nd_full_path(tmp_buf, nd_filename(relative_file), outbuf, bufsize);
}

//get extend name 
const char * nd_file_ext_name(const char *fullPath)
{
	const char *start = fullPath;
	size_t size = strlen(start);
	const char *p = start + size;

	int ret = 0;

	while (p-- > start){
		if (*p == '.'){
			ret = 1;
			break;
		}
		else if (*p == '/' || *p=='\\')	{
			ret = 0;
			break;
		}
	}

	if (ret){
		return p + 1;
	}
	return NULL;
}


//test input_path is in parent_path
int  nd_is_subpath(const char *parent_path, const char *input_path)
{
	char tmp_path[ND_FILE_PATH_SIZE];
	char *p = nd_absolute_path(input_path, tmp_path, sizeof(tmp_path));
	const char *r = parent_path;

	while (*r) {
		if (*p == 0) {
			return 0;
		}
		if (*p==0 || !__is_equal_char(*p++, *r++)) {
			return 0;
		}
	}
	return 1;
}