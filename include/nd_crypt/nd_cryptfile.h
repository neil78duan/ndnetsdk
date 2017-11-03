/* file nd_cryptfile.h
 *
 * define file encrypt / decrypt
 *
 * create by duan
 *
 * 2012/7/20 10:25:01
 */

#ifndef _ND_CRYPTFILE_H_
#define _ND_CRYPTFILE_H_

#include <stdlib.h>
#include <stdio.h>
#include "nd_crypt/nd_crypt.h"

#define NDC_FILE_BUF 1024 
typedef struct ndcrypt_file_io
{
	FILE *pf ;	
	size_t page_pos ;
	size_t cur_file_pos;
	size_t file_end;
	size_t size_inbuf ;
	
	tea_k passwd;	
	unsigned int isload : 1;
	unsigned int iswrite : 1;
	unsigned int is_flush : 1 ;
	char page_buf[NDC_FILE_BUF] ;
}NDC_FILE;

ND_CRYPT_API NDC_FILE *ndc_fopen_r(const char *filename, const char *passwd) ;
ND_CRYPT_API NDC_FILE *ndc_fopen_w(const char *filename, const char *passwd) ;
ND_CRYPT_API size_t ndc_fread(void *buf, size_t elementsize, size_t count,NDC_FILE *pf ) ;
ND_CRYPT_API size_t ndc_fwrite(const void *buf, size_t elementsize, size_t count,NDC_FILE *pf ) ;
ND_CRYPT_API int ndc_fclose(NDC_FILE *pf) ;
ND_CRYPT_API int ndc_fseek(NDC_FILE*pf, long offset, int flag) ;
ND_CRYPT_API long ndc_ftell(NDC_FILE *pf) ;
ND_CRYPT_API int ndc_fflush(NDC_FILE *pf) ;
ND_CRYPT_API void* ndc_load_file_ex(const char *file, size_t *size, const char *fileKey);

#endif
