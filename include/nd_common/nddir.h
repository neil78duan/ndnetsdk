/* file  nddir.h
 *
 * define direct 
 * 
 * create by duan 
 *
 * 2012/9/10 11:00:03
 *
 */

#ifndef _ND_IDR_H_
#define _ND_IDR_H_

#include "nd_os.h"
#ifdef _MSC_VER
#include <io.h>

//�õ�����Ŀ¼
ND_COMMON_API const char * nd_getcwd();

//ɾ���ļ�
ND_COMMON_API int nd_rmfile(const char *file);

//�������ļ�
ND_COMMON_API int nd_renfile(const char *oldfile,const  char *newfile);

//�ı乤��Ŀ¼
ND_COMMON_API int nd_chdir(const char *dir);

//copy �ļ�
ND_COMMON_API int nd_cpfile(const char *oldfile,const  char *newfile);

//����һ��Ŀ¼
ND_COMMON_API int nd_mkdir(const char *dir);
//ɾ��һ��Ŀ¼
ND_COMMON_API int nd_rmdir(const char *dir);

//�õ�ϵͳĿ¼
ND_COMMON_API const  char* nd_getsysdir();

//����ָ�����ļ��Ƿ����
ND_COMMON_API int nd_existfile(const char *pachfilename);

//�õ�����Ŀ¼
ND_COMMON_API const char * nd_getworkingdir();

//����һ�����ļ�
ND_COMMON_API int nd_mkfile(const char *file);

//���õ�ǰ�Ĺ���Ŀ¼
ND_COMMON_API char * nd_setworkingdir(const char *d);
#else 

#include<dirent.h>

//�õ�����Ŀ¼
ND_COMMON_API const char * nd_getcwd();
//�õ�����Ŀ¼
ND_COMMON_API const char * nd_getworkingdir();
//���õ�ǰ�Ĺ���Ŀ¼
ND_COMMON_API char * nd_setworkingdir(const char *d);
//copy �ļ�
ND_COMMON_API int nd_cpfile(const char *oldfile,const  char *newfile);

//�õ�ϵͳĿ¼
ND_COMMON_API const  char* nd_getsysdir();

//����ָ�����ļ��Ƿ����
ND_COMMON_API int nd_existfile(const char *pachfilename);
//����һ�����ļ�
ND_COMMON_API int nd_mkfile(const char *file);

#define nd_rmfile remove
#define nd_renfile rename
#define nd_chdir chdir 
#define nd_mkdir(d) mkdir(d, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#define nd_rmdir rmdir

#endif


#define nd_access access

#endif
