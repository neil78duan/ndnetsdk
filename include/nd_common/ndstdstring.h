/* file ndstdstring.h
 *
 * define std string functions
 *
 * create by duan 
 *
 * 2018.12.6
 */
 
#ifndef _ND_STD_STRING_H_
#define _ND_STD_STRING_H_


#define ndfprintf		fprintf     //使用vararg参量的格式化输出 
#define ndprintf		printf      //使用vararg参量的格式化输出到标准输出 
#define ndsprintf		sprintf     //根据vararg参量表格式化成字符串 
#define ndvfprintf		vfprintf    //使用stdarg参量表格式化输出到文件 
#define ndvsnprintf		vsnprintf    //格式化stdarg参量表并写到字符串 
#define ndsnprintf 		snprintf

#define ndstrcat	strcat			//把一个字符串接到另一个字符串的尾部 
#define ndstrncat	strncat			//而且指定粘接字符串的粘接长度. 
#define ndstrchr	strchr			//查找子字符串的第一个位置 
#define ndstrrchr	strrchr			//从尾部开始查找子字符串出现的第一个位置 
#define ndstrpbrk	strpbrk			//从一字符字符串中查找另一字符串中任何一个字符第一次出现的位置 
#define ndstrstr	strstr			//在一字符串中查找另一字符串第一次出现的位置 
#define ndstrcspn	strcspn			//返回不包含第二个字符串的的初始数目 
#define ndstrspn	strspn			//返回包含第二个字符串的初始数目 
#define ndstrcpy	strcpy			//拷贝字符串 
#define ndstrncpy	strncpy			//同时指定拷贝的数目 
#define ndstrcmp	strcmp			//比较两个宽字符串 
#define ndstrncmp	strncmp			//指定比较字符字符串的数目 
#define ndstrlen	strlen			// 获得宽字符串的数目 
#define ndstrtok	strtok


#endif
