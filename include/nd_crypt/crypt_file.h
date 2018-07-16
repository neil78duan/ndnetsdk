//
//  crypt_file.h
//  gameHall
//
//  Created by duanxiuyun on 14-11-19.
//  Copyright (c) 2014 duanxiuyun. All rights reserved.
//

// note please include this only you need this functions. It's a better way include it after other header-file.

//#ifndef gameHall_crypt_file_h
//#define gameHall_crypt_file_h


//#endif



#include "nd_crypt/nd_cryptfile.h"
#undef FILE
#define FILE NDC_FILE

#undef fread
#define fread ndc_fread

#undef fwrite
#define fwrite ndc_fwrite

#undef fclose
#define fclose ndc_fclose

#undef fseek
#define fseek ndc_fseek

#undef ftell
#define ftell ndc_ftell

#undef fflush
#define fflush ndc_fflush

//crypt read write file
static inline const char* getFileKey(char *buf, int inlen)
{
	MD5Crypt32((void*)"apoloKey", 9, buf);
	buf[32] = 0 ;
	return buf ;
}

#define MYOPEN_FILE_WITH_KEY(_filename, _mode, _keyBuf)	\
	if (0==strcmp(_mode, "r") ||0==strcmp(_mode, "rb") || 0==strcmp(_mode, "r+b")) {	\
		return ndc_fopen_r(_filename, getFileKey(_keyBuf, sizeof(_keyBuf)));			\
	}											\
	else {										\
		return ndc_fopen_w(_filename, getFileKey(_keyBuf, sizeof(_keyBuf)));			\
	}

static NDC_FILE *fopen_ex(const char * filename, const char *mode)
{
	char keybuf[33] ;
	MYOPEN_FILE_WITH_KEY(filename, mode, keybuf);
	return NULL;
}

#undef fopen
#define fopen fopen_ex

#undef nd_load_file
#define nd_load_file(file, size) ndc_load_file_ex(file, size,"apoloKey")