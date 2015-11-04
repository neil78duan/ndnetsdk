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
ND_COMMON_API char * nd_gbk_to_utf8(const char *input_text, char *output_buf, int size_buf);
ND_COMMON_API char * nd_utf8_to_gbk(const char *input_text, char *output_buf, int size_buf);

ND_COMMON_API char * nd_utf8_to_ndcode(const char *input_text, char *outbuf, int size);
ND_COMMON_API char * nd_ndcode_to_utf8(const char *input_text, char *outbuf, int size);

#endif /* defined(__gameHall__nd_iconv__) */
