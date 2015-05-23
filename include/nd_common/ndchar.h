/* file ndchar.h
 * define ND engine character-set operate function
 * ver 1.0
 * neil duan
 * 2007-9-28 
 * all right reserved 2007 by neil
 */

/*
 * ndchar 类似于TCHAR
 * 因为我不想分别实现ansi和unicode二进制版本,只是想在编译时方便进行选择!
 */
#ifndef _NDCHAR_H_
#define _NDCHAR_H_

#include <wchar.h>

#define ND_ESC		0x1b
#if !defined(ND_UNIX) 
#if _MSC_VER < 1300 // 1200 == VC++ 6.0

#define snprintf 		_snprintf
#define snwprintf 		_snwprintf
#define vsnwprintf		_vsnwprintf
#define vsnprintf		_vsnprintf

#else 
#endif

#endif 

#ifdef ND_UNICODE

#define ndchar_t wchar_t
#define _NDT(x)		L ## x
#define ndisalnum     iswalnum		//测试字符是否为数字或字母 
#define ndisalpha     iswalpha 		// 测试字符是否是字母 
#define ndiscntrl     iswcntrl 		//测试字符是否是控制符 
#define ndisdigit     iswdigit 		//测试字符是否为数字 
#define ndisgraph     iswgraph 		//测试字符是否是可见字符 
#define ndislower     iswlower 		//测试字符是否是小写字符 
#define ndisprint     iswprint 		//测试字符是否是可打印字符 
#define ndispunct     iswpunct 		//测试字符是否是标点符号 
#define ndisspace     iswspace 		//测试字符是否是空白符号 
#define ndisupper     iswupper 		//测试字符是否是大写字符 
#define ndisxdigit    iswxdigit		//测试字符是否是十六进制的数字 

#define ndtolower     towlower 		//把字符转换为小写 
#define ndtoupper     towupper 		//把字符转换为大写 
#define ndcscoll      wcscoll 		//比较字符串 

/*
打印和扫描字符串： 
宽字符函数描述 
*/
#define ndfprintf		fwprintf     //使用vararg参量的格式化输出 
#define ndprintf		wprintf      //使用vararg参量的格式化输出到标准输出 
#define ndsprintf		swprintf     //根据vararg参量表格式化成字符串 
#define ndvfprintf		vfwprintf    //使用stdarg参量表格式化输出到文件 
#define ndvsprintf		vsnwprintf    //格式化stdarg参量表并写到字符串 

#define ndsnprintf 		snwprintf

#define ndstrtod 		wcstod    //把宽字符的初始部分转换为双精度浮点数 
#define ndstrtol		wcstol     //把宽字符的初始部分转换为长整数 
#define ndstrtoul		wcstoul    //宽字符的初始部分转换为无符号长整数 

/*
字符串操作： 
宽字符函数        普通C函数描述 
*/
#define ndwitch		wcwidth			//测试单个字符占用的内存宽度
#define ndstrcat	wcscat			//strcat把一个字符串接到另一个字符串的尾部 
#define ndstrncat	wcsncat			//strncat
#define ndstrchr	wcschr			//strchr查找子字符串的第一个位置 
#define ndstrrchr	wcsrchr			// strrchr（）     从尾部开始查找子字符串出现的第一个位置 
#define ndstrpbrk	wcspbrk         //strpbrk（）     从一字符字符串中查找另一字符串中任何一个字符第一次出现的位置 
#define ndstrstr	wcsstr			//    strstr（）     在一字符串中查找另一字符串第一次出现的位置 
#define ndstrcspn	wcscspn			//（）         strcspn（）     返回不包含第二个字符串的的初始数目 
#define ndstrspn	wcsspn			//（）         strspn（）     返回包含第二个字符串的初始数目 
#define ndstrcpy	wcscpy			//（）         strcpy（）     拷贝字符串 
#define ndstrncpy	wcsncpy			//（）         strncpy（）     类似于wcscpy（）， 同时指定拷贝的数目 
#define ndstrcmp	wcscmp			//（）         strcmp（）     比较两个宽字符串 
#define ndstrncmp	wcsncmp			//（）         strncmp（）     类似于wcscmp（）， 还要指定比较字符字符串的数目 
#define ndstrlen	wcslen			//（）         strlen（）     获得宽字符串的数目 
#define ndstrtok	wcstok			//（）         strtok（）     根据标示符把宽字符串分解成一系列字符串 

#else		//ansi
#define _NDT(x) x
#define ndchar_t char

#define ndisalnum     isalnum		//测试字符是否为数字或字母 
#define ndisalpha     isalpha 		// 测试字符是否是字母 
#define ndiscntrl     iscntrl 		//测试字符是否是控制符 
#define ndisdigit     isdigit 		//测试字符是否为数字 
#define ndisgraph     isgraph 		//测试字符是否是可见字符 
#define ndislower     islower 		//测试字符是否是小写字符 
#define ndisprint     isprint 		//测试字符是否是可打印字符 
#define ndispunct     ispunct 		//测试字符是否是标点符号 
#define ndisspace     isspace 		//测试字符是否是空白符号 
#define ndisupper     isupper 		//测试字符是否是大写字符 
#define ndisxdigit    isxdigit		//测试字符是否是十六进制的数字 

#define ndtolower     tolower 		//把字符转换为小写 
#define ndtoupper     toupper 		//把字符转换为大写 
#define ndcscoll     strcoll 		//比较字符串 

/*
打印和扫描字符串： 
宽字符函数描述 
*/
#define ndwitch(ch)		1			//测试单个字符占用的内存宽度
#define ndfprintf		fprintf     //使用vararg参量的格式化输出 
#define ndprintf		printf      //使用vararg参量的格式化输出到标准输出 
#define ndsprintf		sprintf     //根据vararg参量表格式化成字符串 
#define ndvfprintf		vfprintf    //使用stdarg参量表格式化输出到文件 
#define ndvsprintf		vsnprintf    //格式化stdarg参量表并写到字符串 
#define ndsnprintf 		snprintf

#define ndstrtod 		strtod		//把宽字符的初始部分转换为双精度浮点数 
#define ndstrtol		strtol		//把宽字符的初始部分转换为长整数 
#define ndstrtoul		strtoul		//把宽字符的初始部分转换为无符号长整数 

/*
字符串操作： 
宽字符函数        普通C函数描述 
*/
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

#endif //ND_UNICODE

#endif
