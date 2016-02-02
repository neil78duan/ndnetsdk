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
		return 0;
	}
	fseek(fp, 0, SEEK_END);
	buf_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if(buf_size==0) {
		fclose(fp) ;
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
		fwrite(buf,sizeof(buf),1,fp2);
	}

	fclose(fp1);
	fclose(fp2);
	return 0;
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
//�õ�����Ŀ¼
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

//ɾ���ļ�
int nd_rmfile(const char *file)
{
	return DeleteFile((LPCTSTR)file);
}

//�������ļ�
int nd_renfile(const char *oldfile,const  char *newfile)
{
	return MoveFile((LPTSTR)oldfile, (LPTSTR)newfile) ;
}

//�ı乤��Ŀ¼
int nd_chdir(const char *dir)
{
	if(SetCurrentDirectory((LPCTSTR)dir) ) {			
		GetCurrentDirectory( sizeof(_current_dir),(LPTSTR)_current_dir);
		return 0;
	}
	return -1;
}

//copy �ļ�
int nd_cpfile(const char *oldfile, const char *newfile)
{
	return CopyFile( (LPCTSTR) oldfile, (LPCTSTR) newfile,TRUE) ;

}

//����һ��Ŀ¼
int nd_mkdir(const char *dir)
{
	return CreateDirectory((LPCTSTR)dir,NULL);
}

//ɾ��һ��Ŀ¼
int nd_rmdir(const char *dir)
{
	return  RemoveDirectory((LPCTSTR)dir);
}

//�õ�ϵͳĿ¼
const char* nd_getsysdir()
{
	static char _system_dir[ND_FILE_PATH_SIZE] ;
	GetSystemDirectory((LPTSTR)_system_dir,sizeof(_system_dir));
	return _system_dir ;
}

//����ָ�����ļ��Ƿ����
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


//����һ�����ļ�
//���Զ�д,��ռ,���������ʧ��
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