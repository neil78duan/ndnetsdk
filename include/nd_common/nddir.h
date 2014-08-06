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

//得到工作目录
ND_COMMON_API const char * nd_getcwd();

//删除文件
ND_COMMON_API int nd_rmfile(const char *file);

//重命名文件
ND_COMMON_API int nd_renfile(const char *oldfile,const  char *newfile);

//改变工作目录
ND_COMMON_API int nd_chdir(const char *dir);

//copy 文件
ND_COMMON_API int nd_cpfile(const char *oldfile,const  char *newfile);

//创建一个目录
ND_COMMON_API int nd_mkdir(const char *dir);
//删除一个目录
ND_COMMON_API int nd_rmdir(const char *dir);

//得到系统目录
ND_COMMON_API const  char* nd_getsysdir();

//查找指定的文件是否存在
ND_COMMON_API int nd_existfile(const char *pachfilename);

//得到工作目录
ND_COMMON_API const char * nd_getworkingdir();

//创建一个新文件
ND_COMMON_API int nd_mkfile(const char *file);

//设置当前的工作目录
ND_COMMON_API char * nd_setworkingdir(const char *d);
#else 

#include<dirent.h>

//得到工作目录
ND_COMMON_API const char * nd_getcwd();
//得到工作目录
ND_COMMON_API const char * nd_getworkingdir();
//设置当前的工作目录
ND_COMMON_API char * nd_setworkingdir(const char *d);
//copy 文件
ND_COMMON_API int nd_cpfile(const char *oldfile,const  char *newfile);

//得到系统目录
ND_COMMON_API const  char* nd_getsysdir();

//查找指定的文件是否存在
ND_COMMON_API int nd_existfile(const char *pachfilename);
//创建一个新文件
ND_COMMON_API int nd_mkfile(const char *file);

#define nd_rmfile remove
#define nd_renfile rename
#define nd_chdir chdir 
#define nd_mkdir(d) mkdir(d, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#define nd_rmdir rmdir

#endif


#define nd_access access

#endif
