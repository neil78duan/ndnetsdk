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


static char _current_dir[ND_FILE_PATH_SIZE] = {0} ;
static char _working_dir[ND_FILE_PATH_SIZE] = {0} ;


#if defined(ND_UNIX) 

const char * nd_getcwd()
{
	long size = getcwd(_current_dir, ND_FILE_PATH_SIZE);
	if (size > 0){
		return _current_dir ;
	}
	return 0;
}

const char * nd_getworkingdir()
{
	if (_working_dir[0]==0)	{
		long size = getcwd(_working_dir, ND_FILE_PATH_SIZE);
		if (-1==size)	{
			return NULL ;
		}
	}

	return _working_dir ;
}
//���õ�ǰ�Ĺ���Ŀ¼
char * nd_setworkingdir(const char *d)
{
	if (chdir(d)==0){
		strncpy(_working_dir, d, sizeof(_working_dir)) ;
	}
	return _working_dir;
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
	GetCurrentDirectory( sizeof(_current_dir),(LPTSTR)_current_dir);
	return _current_dir ;
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
	return SetCurrentDirectory((LPCTSTR)dir);
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
	GetSystemDirectory((LPTSTR)_current_dir,sizeof(_current_dir));
	return _current_dir ;
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

//�õ�����Ŀ¼
const char * nd_getworkingdir()
{
	if(_working_dir[0]==0) {
		GetCurrentDirectory( sizeof(_working_dir),(LPTSTR)_working_dir);
	}
	return _working_dir ;
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

//���õ�ǰ�Ĺ���Ŀ¼
char * nd_setworkingdir(const char *d)
{
	if (SetCurrentDirectory((LPCTSTR)d)){
		strncpy(_working_dir, d, sizeof(_working_dir)) ;
	}
	return _working_dir;
}
#endif
