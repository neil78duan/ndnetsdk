/* file : nd_str.h
 * defien string function of nd common 
 * 2007-3-15 17:29
 * last mod 2008-8-24 
 * v1.0
 * neil 
 */
 
#include "nd_common/nd_str.h"
#include "nd_common/nd_comcfg.h"

static __INLINE__ int _read_word(unsigned  char** dest, unsigned char **src)
{
#if defined(ND_GB2312)
	if (**src > (unsigned  char)0x80){
		**dest = **src ; ++(*src) ; ++(*dest) ;
		**dest = **src ; ++(*src) ; ++(*dest) ;
		return 2;
	}
#elif defined(ND_UTF_8)
	if (**src > (unsigned  char)0x80){
		int ret =1 ;
		if (**src >=(unsigned  char)224)	{
			**dest = **src ; ++(*src) ; ++(*dest) ;
			**dest = **src ; ++(*src) ; ++(*dest) ;
			ret += 2 ;
		}
		else if(**src >=(unsigned  char)192)	{
			**dest = **src ; ++(*src) ; ++(*dest) ;
			++ret ;
		} 
		**dest = **src ; ++(*src) ; ++(*dest) ;
		return ret;
	}
#endif
	**dest = **src ; 
	++(*src) ; 
	++(*dest) ;
	return 1 ;
 
}

/* 去掉字符串开头部分无用的字符（不可打印的字符）*/
char *ndstr_first_valid(const char *src) 
{
	unsigned char *tmp = (unsigned char *)src ;
	while(*tmp <=(unsigned char) _ND_SPACE) {
		if(*tmp==0) {
			return NULL;
		}
		tmp++ ;
	}
	return (char*)tmp ;
		
}

/* 检测字符是否是有效的数字*/
int ndstr_is_numerals(const char *src)
{
	int dot = 0 ;
	int ret = 0 ;
	src = ndstr_first_valid(src) ;
	
	if(*src==_MINUS)
		++src ;
	else if(IS_NUMERALS(*src)){
		++src ; ret = 1 ;
	}
	else 
		return 0 ;
	while(*src) {
		if(IS_NUMERALS(*src) ){
			ret = 1 ;
		}
		else if(_DOT==*src) {
			dot++ ;
			if(dot > 1)
				return 0 ;
		}
		else
			return 0 ;
		++src ;
	}
	return ret ;	
}

//检测字符串是否自然数
int ndstr_is_naturalnumber(const char *src)
{	
	int ret = 1 ;
	src = ndstr_first_valid(src) ;
	
	if(*src=='+')
		++src ;
	else if(IS_NUMERALS(*src)){
		++src ; ret = 1 ;
	}
	else 
		return 0 ;
	while(*src) {
		if(!IS_NUMERALS(*src) ){
			ret = 0 ;
			break ;
		}
		++src ;
	}
	return ret ;	
}


/* 读取有效数字,*isok == 0出错*/
char *ndstr_read_numerals(const char *src, char *desc, int *isok) 
{
	int dot = 0 ;
	*isok = 0 ;
	src = ndstr_first_valid(src) ;
	
	if(*src==_MINUS||*src=='+')
		*desc++ =*src++ ;
	else if(*src==_DOT){
		*desc++ = '0' ;
		*desc++ =*src++ ;
		dot++ ;
	}
	else if(IS_NUMERALS(*src)){
		*desc++ =*src++ ;
		*isok = 1 ;
	}
	else {
		*isok = 0 ;
		return (char*)src ;
	}
	while(*src) {
		if(IS_NUMERALS(*src) ){
			*isok = 1 ;
		}
		else if(_DOT==*src) {
			dot++ ;
			if(dot > 1) {
				*isok = 0 ;
				break ;
			}
		}
		else{
			break ;
		}
		*desc++ =*src++ ;
	}
	*desc = 0;
	return (char*)src ;	
}


//分解一个单词,单词只能是数字,字母和下划线
char *ndstr_parse_word(char *src, char *outstr)
{
	register unsigned char a ;
	while(*src) {
		a = (unsigned char)*src ;
		if(IS_NUMERALS(a) || IS_BIG_LATIN(a) || IS_LITTLE_LATIN(a) || a=='_' ){			
			*outstr++ = *src++ ;
		}
#ifndef ND_ANSI
		else if(a>(unsigned char)0x80){		//chinese		
			_read_word((unsigned char**)&outstr, (unsigned  char**)&src) ;
			//*outstr++ = *src++ ;
			//*outstr++ = *src++ ;
		}
#endif 
		else{
			break ;
		}
	}
	*outstr = 0 ;
	return *src?src:NULL ;
}

//分解可显示的字符床
char *ndstr_parse_string(char *src, char *outstr)
{
	register unsigned char a ;
	while(*src) {
		a = (unsigned char)*src ;
		if (a<=0x20)	{
			break ;
		}
		_read_word((unsigned char**)&outstr, (unsigned  char**)&src) ;
		/*
		if(a > 0x20){			
			*outstr++ = *src++ ;
		}

		else if(a>(unsigned char)0x80){		//chinese		
			*outstr++ = *src++ ;
			*outstr++ = *src++ ;
		}
		else{
			break ;
		}*/

	}
	*outstr = 0 ;
	return *src?src:NULL ;
}


char *ndstr_parse_word_n(char *src, char *outstr, int n)
{
	register unsigned char a ;
	while(*src && n-- > 0) {
		a = (unsigned char)*src ;
		if(IS_NUMERALS(a) || IS_BIG_LATIN(a) || IS_LITTLE_LATIN(a) || a=='_' || a=='.'){
			*outstr++ = *src++ ;
		}
		
#ifndef ND_ANSI
		else if(a>(unsigned char)0x80){		//chinese		
			int ret  = _read_word((unsigned char**)&outstr, (unsigned  char**)&src) ;
			n -= ret -1 ;
			//*outstr++ = *src++ ;
			//*outstr++ = *src++ ;
		}
#endif 
		else{
			break ;
		}
	}
	*outstr = 0 ;
	return *src?src:NULL ;
}

char *_ndstr_read_cmd(char *src, char *outstr, int n, char endmark) 
{
	register unsigned char a ;
	while(*src && n-- > 0) {		
		a = (unsigned char)*src ;
		if (a=='\\') {
			src++ ;
			a = (unsigned char)*src ;
		}
		else if(endmark==a) {
			++src ; //skip endmark ,such as " hello world "
			break ;
		}
		*outstr++ = *src++ ;
		
	}
	*outstr = 0 ;
	return *src?src:NULL ;
	
}

//Parse string to command line , return number of commands-lines
int ndstr_parse_command(char *input_text, char *argv[], int bufize, int number) 
{
	int ret = 0 ;
	char *next_text = ndstr_first_valid((const char*)input_text) ;
	while (next_text && *next_text ) {
#ifdef ND_UNIX
		if (*next_text==0x24) { // $
			char envValName[1024] ;
			envValName[0] = 0 ;
			next_text = ndstr_parse_word(++next_text, envValName) ;
			if (envValName[0]) {
				char *envVal = getenv(envValName) ;
				if (envVal && envVal[0]) {
					strncpy(argv[ret], envVal, bufize) ;					
				}
				else {
					break ;
				}
			}
			else {
				break ;
			}
			
		}
#else 
		if (*next_text==0x25) { // %
			char envValName[1024] ;
			envValName[0] = 0 ;
			
			next_text = ndstr_str_end(++next_text, envValName, 0x25) ;
			if (envValName[0]) {
				++next_text ;
				
				char *envVal = getenv(envValName) ;
				if (envVal && envVal[0]) {
					strncpy(argv[ret], envVal, bufize) ;					
				}
				else {
					break ;
				}
			}
			else {
				break ;
			}
		}
#endif
		else if (*next_text=='\"') {
			++next_text ;
			next_text = _ndstr_read_cmd(next_text, argv[ret], bufize, '\"' ) ;
		}
		else {
			next_text = ndstr_parse_string(next_text, argv[ret]) ;
		}
		++ret ;
		if (ret>=number) {
			break ;
		}
		next_text = ndstr_first_valid((const char*)next_text) ;
	}
	return ret ;
	
}

int ndstr_get_ip(char *src, ndip_t *ip) 
{
	int i= 0;
	union {
		ndip_t ip ;
		NDUINT8 buf[4] ;
	}readip;
	char *p = src ;
	if(!p)
		return -1;
	while( *p) {
		if(IS_NUMERALS(*p) || *p=='.' || *p==0x25) {
			++p ;
		}
		else {
			return -1;
		}
	}
	//1
	p = src ;
	for(i=0; i<3; i++) {
		if(*p==0)
			return -1;
		if(*p=='%') {
			readip.buf[i] = 0xff;
			++p;
		}
		else {
			readip.buf[i] = (NDUINT8)strtol(p, &p, 0);
		}
		if(*p==0)
			return -1;
		++p;//skip '.'
	}

	//4
	if(*p==0)
		return -1;
	if(*p=='%') {
		readip.buf[i++] = 0xff;
	}
	else {
		readip.buf[i++] = (NDUINT8)strtol(p, &p, 0);
	}
	*ip = readip.ip ;
	return 0;
}

/*读取一个字符串，知道遇到一个制定的结束字符为止*/
char *ndstr_str_end(char *src, char *outstr, const char end)
{
	while(*src) {
		if(end== *src ){
			break ;
		}
		_read_word((unsigned char**)&outstr, (unsigned  char**)&src) ;
// 		if((unsigned char)*src>(unsigned char)0x80)		//chinese		
// 			*outstr++ = *src++ ;
// 		*outstr++ = *src++ ;
	}
	*outstr = 0 ;
	return src ;
}
char *ndstr_nstr_end(char *src, char *outstr, const char end, int n)
{
	while(*src && n>0) {
		if(end== *src ){
			break ;
		}
		else {
			int ret  = _read_word((unsigned char**)&outstr, (unsigned  char**)&src) ;
			n -= ret ;
		}
// 		if((unsigned char)*src>(unsigned char)0x80){	//chinese		
// 			*outstr++ = *src++ ;
// 			--n ;
// 		}
// 		*outstr++ = *src++ ;
// 		--n ;
	}
	*outstr = 0 ;
	return src ;
}

/*不区分大小写,比较字符串*/
int ndstricmp(char *src, char *desc) 
{
	int ret ;
	do {
		ret = *src - *desc ;
		if(ret){
			char a ;
			if(IS_BIG_LATIN(*src)) {
				a= BIG_2_LITTLE(*src) ;
			}
			else if(IS_LITTLE_LATIN(*src)) {
				a=LITTLE_2_BIG(*src) ;
			}
			else {
				return ret ;
			}
			if(a!=*desc)
				return ret ;
		}
		desc++ ;
	}while (*src++) ;
	return ret ;
}

//在src中查找desc 不区分大小写
char *ndstristr(char *src, char *desc)
{
	int ret=0 ;
	while(*src) {
		char *tmp = src;
		char *aid = desc;
		
		while(*aid) {
			ret = *tmp - *aid ;
			if(ret){
				char a ;
				if(IS_BIG_LATIN(*tmp)) {
					a= BIG_2_LITTLE(*tmp) ;
				}
				else if(IS_LITTLE_LATIN(*tmp)) {
					a=LITTLE_2_BIG(*tmp) ;
				}
				else {
					break ;
				}
				
				if(a!=*aid)
					break ;
				else 
					ret = 0 ;
			}
			aid++ ;
			tmp++ ;
		}

		if(0==ret)
			return src ;
		++src;
	}
	return NULL ;
}

//从src所指的方向向前查找字符ch,
//如果找到end位置还没有找到则返回null
char *ndstr_reverse_chr(char *src, char ch, char *end)
{
	while(end <= src) {
		if(ch== *src ){
			return src ;
		}
		--src;
	}
	
	return NULL;
}
