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

ND_COMMON_API size_t nd_get_file_size(const char *file);
ND_COMMON_API void nd_unload_file(void *file_data);
ND_COMMON_API void* nd_load_file(const char *file, size_t *size);

//merge path and file to full-path
ND_COMMON_API char* nd_full_path(const char*in_path, const char *in_file, char *outbuf, size_t size);

#ifdef _MSC_VER
#include <io.h>

//删除文件
ND_COMMON_API int nd_rmfile(const char *file);
//重命名文件
ND_COMMON_API int nd_renfile(const char *oldfile,const  char *newfile);

//创建一个目录
ND_COMMON_API int nd_mkdir(const char *dir);
//删除一个目录
ND_COMMON_API int nd_rmdir(const char *dir);

#else 

#include<dirent.h>
#include <sys/stat.h>

#define nd_rmfile remove
#define nd_renfile rename
#define nd_mkdir(d) mkdir(d, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#define nd_rmdir rmdir

#endif

//得到工作目录
ND_COMMON_API const char * nd_getcwd();
ND_COMMON_API int nd_chdir(const char *dir);
//得到工作目录
ND_COMMON_API const char * nd_get_init_dir();
//copy 文件
ND_COMMON_API int nd_cpfile(const char *oldfile,const  char *newfile);
//得到系统目录
ND_COMMON_API const  char* nd_getsysdir();
//查找指定的文件是否存在
ND_COMMON_API int nd_existfile(const char *pachfilename);
//创建一个新文件
ND_COMMON_API int nd_mkfile(const char *file);


ND_COMMON_API  const char * nd_filename(const char *filenamePath);

ND_COMMON_API  const char * nd_getpath(const char *filenamePath, char *pathbuf, size_t size);

#define nd_access access

//ND_COMMON_API  const char * nd_relative_path(const char *fullPath);
ND_COMMON_API  const char * nd_relative_path(const char *fullPath,const char *workPath, char *outbuf, size_t bufsize);

// get absolute path
ND_COMMON_API  const char * nd_absolute_path(const char *relative_path, char *outbuf, size_t bufsize);
ND_COMMON_API  const char * nd_absolute_filename(const char *relative_file, char *outbuf, size_t bufsize);

//get extend name 
ND_COMMON_API  const char * nd_file_ext_name(const char *fullPath);


#endif
