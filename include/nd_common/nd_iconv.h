//
//  nd_iconv.h
//  gameHall
//
//  Created by duanxiuyun on 15-5-11.
//  Copyright (c) 2015duanxiuyun. All rights reserved.
//

#ifndef __gameHall__nd_iconv__
#define __gameHall__nd_iconv__

#include <stdio.h>

#include "nd_common/nd_common.h"
#ifdef WITHOUT_ICONV
static __INLINE__ char * nd_gbk_to_utf8(const char *input_text, char *output_buf, int size_buf)
{
	return ndstrncpy(output_buf, input_text,(size_t) size_buf) ;
}
static __INLINE__ char * nd_utf8_to_gbk(const char *input_text, char *output_buf, int size_buf)
{
	return ndstrncpy(output_buf, input_text,(size_t) size_buf) ;
}

static __INLINE__ char * nd_utf8_to_ndcode(const char *input_text, char *outbuf, int size)
{
	return ndstrncpy(outbuf, input_text,(size_t) size) ;
}
static __INLINE__ char * nd_ndcode_to_utf8(const char *input_text, char *outbuf, int size)
{
	return ndstrncpy(outbuf, input_text,(size_t) size) ;
}


#else
ND_COMMON_API char * nd_gbk_to_utf8(const char *input_text, char *output_buf, int size_buf);
ND_COMMON_API char * nd_utf8_to_gbk(const char *input_text, char *output_buf, int size_buf);

ND_COMMON_API char * nd_utf8_to_ndcode(const char *input_text, char *outbuf, int size);
ND_COMMON_API char * nd_ndcode_to_utf8(const char *input_text, char *outbuf, int size);
#endif


typedef char*(*nd_code_convert_func)(const char *, char *, int);
ND_COMMON_API nd_code_convert_func nd_get_code_convert(int fromType, int toType);
ND_COMMON_API int nd_code_convert_file(const char *file, int fromType, int toType);
#endif /* defined(__gameHall__nd_iconv__) */
