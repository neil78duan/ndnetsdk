/* file : nd_str.h
 * defien string function of nd common 
 * 2007-3-15 17:29
 * last mod 2008-8-24 
 * v1.0
 * neil 
 */
 
#ifndef _ND_STR_H_
#define _ND_STR_H_

#include "nd_common/nd_common.h"
 
#define _DOT  '.'
#define _MINUS '-' 
#define _ND_SPACE			0x20
#define _ND_TAB				0x09
#define _ND_QUOT			0x22		//双引号
#define _ND_SINGLE_QUOT		0x27		//单引号
#define IS_NUMERALS(a)		((a) >= '0' && (a) <='9')

#define IS_PRINTABLE(a)		((a) >= 0x20 && (a) < 0x7f)

#define IS_BIG_LATIN(a)		( (a) >= 'A' && (a)<='Z')
#define IS_LITTLE_LATIN(a)	( (a) >= 'a' && (a)<='z')
#define BIG_2_LITTLE(a)		(a)+0x20 
#define LITTLE_2_BIG(a)		(a)-0x20 

/* 去掉字符串开头部分无用的字符（不可打印的字符）*/
ND_COMMON_API char *ndstr_first_valid(const char *src) ;

/* 检测字符是否是有效的数字*/
ND_COMMON_API int ndstr_is_numerals(const char *src);

//检测字符串是否自然数
ND_COMMON_API int ndstr_is_naturalnumber(const char *src);

ND_COMMON_API char *ndstr_read_numerals(const char *src, char *desc, int *isok) ;

//分解一个单词,单词只能是数字,字母和下划线
ND_COMMON_API char *ndstr_parse_word(const char *src, char *outstr);
ND_COMMON_API char *ndstr_parse_word_n(const char *src, char *outstr, int n);

//Parse string to command line , return number of commands-lines
ND_COMMON_API int ndstr_parse_command(const char *input_text, char *argv[], int bufize, int number);

ND_COMMON_API int ndstr_get_ip(const char *src, ndip_t *ip);

//分解可显示的字符床
ND_COMMON_API char *ndstr_parse_string(const char *src, char *outstr);

/*读取一个字符串，知道遇到一个制定的结束字符为止*/
ND_COMMON_API char *ndstr_str_end(const char *src, char *outstr, const char end);
ND_COMMON_API char *ndstr_nstr_end(const char *src, char *outstr, const char end, int n);

/* read text from src to outstr, until to end, only input ansi text*/
ND_COMMON_API char *ndstr_str_ansi(const char *src, char *outstr, const char end);
ND_COMMON_API char *ndstr_nstr_ansi(const char *src, char *outstr, const char end, int n);

/*不区分大小写,比较字符串*/
ND_COMMON_API int ndstricmp(const char *src, const char *desc);

//在src中查找desc 不区分大小写
ND_COMMON_API char *ndstristr(const char *src, const char *desc);

//从src所指的方向向前查找字符ch,
//如果找到end位置还没有找到则返回null
ND_COMMON_API char *ndstr_reverse_chr(const char *src, char ch, const char *end);

#ifdef _MSC_VER
#define nd_atoi64 _atoi64
#else 
#define nd_atoi64 atoll
#endif

enum source_code_type
{
	E_SRC_CODE_ANSI = 0 ,
	E_SRC_CODE_GBK,
	E_SRC_CODE_UTF_8
};

ND_COMMON_API int ndstr_set_code(int type);
#endif 
