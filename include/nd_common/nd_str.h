/* file : nd_str.h
 * defien string function of nd common 
 * 2007-3-15 17:29
 * last mod 2008-8-24 
 * v1.0
 * neil 
 */
 
#ifndef _ND_STR_H_
#define _ND_STR_H_

//#include "nd_common/nd_common.h"
#include "nd_common/nd_os.h"

#define _DOT  '.'
#define _MINUS '-' 
#define _ND_SPACE			0x20
#define _ND_TAB				0x09
#define _ND_QUOT			0x22		// ansi code of "
#define _ND_SINGLE_QUOT		0x27		// ansi code of '
#define IS_NUMERALS(a)		((a) >= '0' && (a) <='9')

#define IS_PRINTABLE(a)		((a) >= 0x20 && (a) < 0x7f)

#define IS_BIG_LATIN(a)		( (a) >= 'A' && (a)<='Z')
#define IS_LITTLE_LATIN(a)	( (a) >= 'a' && (a)<='z')
#define BIG_2_LITTLE(a)		(a)+0x20 
#define LITTLE_2_BIG(a)		(a)-0x20 


ND_COMMON_API int ndstr_read_utf8char(char **src, char** dest) ;

/* skip the first char can not printable (include space tab) */
ND_COMMON_API const char *ndstr_first_valid(const char *src) ;

ND_COMMON_API int ndstr_is_numerals(const char *src);

ND_COMMON_API int ndstr_is_naturalnumber(const char *src);

ND_COMMON_API const char *ndstr_read_numerals(const char *src, char *desc, int *isok) ;

//parse the string to world , _ , number,and latin letter is valid
ND_COMMON_API const char *ndstr_parse_word(const char *src, char *outstr);
ND_COMMON_API const char *ndstr_parse_word_n(const char *src, char *outstr, int n);


ND_COMMON_API int ndstr_parse_variant_n(const char *src, char *outstr, int n); //return 0 nothing, -1 error ,else data size

//Parse string to command line , return number of commands-lines
ND_COMMON_API int ndstr_parse_command(const char *input_text, char *argv[], int bufize, int number);

ND_COMMON_API int ndstr_get_ip(const char *src, NDUINT32 *ip);
ND_COMMON_API int ndstr_get_ip6(const char *src, ndip_t *ip);

//parse string to substring ,untill space, tab ..
ND_COMMON_API const  char *ndstr_parse_string(const char *src, char *outstr);

/*read string untill the 'end' */
ND_COMMON_API const char *ndstr_str_end(const char *src, char *outstr, const char end);
ND_COMMON_API const char *ndstr_nstr_end(const char *src, char *outstr, const char end, int n);

/* read text from src to outstr, until to end, only input ansi text*/
ND_COMMON_API const char *ndstr_str_ansi(const char *src, char *outstr, const char end);
ND_COMMON_API const char *ndstr_nstr_ansi(const char *src, char *outstr, const char end, int n);

/*text comp ignore caps*/
ND_COMMON_API int ndstricmp(const char *src, const char *desc);
ND_COMMON_API int ndstricmp_n(const char *src, const char *desc, int n);

ND_COMMON_API const char *ndstristr(const char *src, const char *desc);

// serach char from tail
ND_COMMON_API const char *ndstr_reverse_chr(const char *src, char ch, const char *end);


ND_COMMON_API unsigned long ndstr_atoi_hex(const char *src);
ND_COMMON_API NDUINT64 ndstr_atoll_hex(const char *src);

ND_COMMON_API char *ndstr_to_little(char *src);
#ifdef _MSC_VER
#define nd_atoi64 _atoi64
#else 
#define nd_atoi64 atoll
#endif

ND_COMMON_API int ndstr_set_code(int type);
ND_COMMON_API int nd_get_encode_val(const char *encodeText);
ND_COMMON_API const char* nd_get_encode_name(int encode_type);
#endif 
