//
//  nd_iconv.c
//  gameHall
//
//  Created by duanxiuyun on 15-5-11.
//  Copyright (c) 2015 duanxiuyun. All rights reserved.
//

#include "nd_common/nd_iconv.h"

#ifndef WITHOUT_ICONV
// ------------------------------------------------------------------------------------------
#ifdef _MSC_VER


char * nd_gbk_to_utf8(const char *input_text, char *output_buf, int size_buf)
{
	int input_len = ndstrlen(input_text);
	int insize = ( input_len+1) * sizeof(wchar_t);
	int wlen, nLen;
	wchar_t *pWbuf = malloc(insize);
	if (!pWbuf) {
		return NULL;
	}
	
	wlen =MultiByteToWideChar(CP_ACP, 0, input_text,input_len , pWbuf, insize);
	pWbuf[wlen] = 0;
	
	nLen = WideCharToMultiByte(CP_UTF8 , 0, pWbuf, wlen, output_buf, size_buf, NULL, NULL);
	output_buf[nLen] = 0 ;
	
	free(pWbuf);
	
	return  output_buf;
}

char * nd_utf8_to_gbk(const char *input_text, char *output_buf, int size_buf)
{
	int input_len = ndstrlen(input_text);
	int insize = ( input_len+1) * sizeof(wchar_t);

	int wlen, nLen;
	wchar_t *pWbuf = malloc(insize);
	if (!pWbuf) {
		return NULL;
	}
	
	wlen = MultiByteToWideChar(CP_UTF8, 0, input_text, input_len, pWbuf, insize);
	pWbuf[wlen] = 0;
	
	nLen = WideCharToMultiByte(CP_ACP, 0, pWbuf, wlen, output_buf, size_buf, NULL, NULL);
	output_buf[nLen] = 0 ;
	
	free(pWbuf);
	
	return  output_buf;
	
}

#else
#include <iconv.h>
static int code_convert(const char *from_charset,const char *to_charset, char *inbuf,int inlen,char *outbuf,int outlen)
{
	iconv_t cd;
	//int rc;
	char **pin = &inbuf;
	char **pout = &outbuf;

	size_t in_buf=inlen;
	size_t out_buf=outlen;

	cd = iconv_open(to_charset,from_charset);
	if (cd==0)  {
		nd_logerror("iconv_open %s\n", nd_last_error() ) ;
		return -1;
	}
	memset(outbuf,0,outlen);
	if (iconv(cd,pin,&in_buf,pout,&out_buf)==-1) {
		nd_logerror("iconv %s\n", nd_last_error() ) ;
		return -1;
	}
	iconv_close(cd);
	return 0;
}

char * nd_gbk_to_utf8(const char *input_text, char *output_buf, int size_buf)
{
	int inlen =(int) ndstrlen(input_text) ;
	int ret = code_convert("GBK","UTF-8",(char*)input_text,inlen,output_buf,size_buf);
	if (0==ret) {
		return output_buf ;
	}
	return NULL ;
}

char * nd_utf8_to_gbk(const char *input_text, char *output_buf, int size_buf)
{
	int inlen = (int) ndstrlen(input_text) ;
	int ret = code_convert("UTF-8","GBK",(char*)input_text,inlen,output_buf,size_buf);
	if (0==ret) {
		return output_buf ;
	}
	return NULL ;
}


#endif


char * nd_utf8_to_ndcode(const char *input_text, char *outbuf, int size)
{
#if (ND_ENCODE_TYPE==E_SRC_CODE_GBK)
	return nd_utf8_to_gbk(input_text, outbuf, size) ;
#else 
	return ndstrncpy(outbuf,input_text,size) ;
#endif

}
char * nd_ndcode_to_utf8(const char *input_text, char *outbuf, int size)
{
#if (ND_ENCODE_TYPE==E_SRC_CODE_GBK)
	return nd_gbk_to_utf8(input_text, outbuf, size);
#else 
	return ndstrncpy(outbuf, input_text, size);
#endif


}

#endif
//-----------------end WITHOUT_ICONV-------------

nd_code_convert_func nd_get_code_convert(int fromType, int toType)
{
	nd_code_convert_func func = NULL;
	//CONVERT CODE 
	if (toType == E_SRC_CODE_UTF_8 && fromType != E_SRC_CODE_UTF_8)	{
		func = nd_gbk_to_utf8;
	}
	else if (toType != E_SRC_CODE_UTF_8 && fromType == E_SRC_CODE_UTF_8)	{
		func = nd_utf8_to_gbk;
	}
	return func;
}
int check_is_space_line(char *bufLine)
{
	while (*bufLine){
		if (*bufLine > 0x20)	{
			return 0;
		}
		++bufLine;
	}
	return 1;
}
int nd_code_convert_file(const char *file, int fromType, int toType)
{
	size_t size;
	char *pconvertbuf;
	char *p;
	FILE *pf;
	char buf[4096];
	nd_code_convert_func func = nd_get_code_convert(fromType, toType);
	if (!func){
		return 0;
	}
	size = nd_get_file_size(file);
	if (0==size){
		return -1;
	}
	size *= 2;
	pconvertbuf = malloc(size * 2);
	if (!pconvertbuf)	{
		return -1;
	}

	pf = fopen(file, "r");
	if (!pf){
		free(pconvertbuf);
		return -1;
	}

	p = pconvertbuf;
	while (fgets(buf,sizeof(buf),pf)!=NULL) {
		
		int len = 0;
		char outbuf[4096];

		if (func(buf, outbuf, sizeof(outbuf))) {
			len = ndsnprintf(p, size, "%s", outbuf);
		}
		else {
			len = ndsnprintf(p, size, "%s", buf);
		}
		p += len;
		size -= len;
		if (*(p-1) !='\n' )	{
			*p++ = '\n';
			--size;
		}
	}
	fclose(pf);

	pf = fopen(file, "w");
	if (!pf){
		free(pconvertbuf);
		return -1;
	}
	size = p - pconvertbuf;
	fwrite(pconvertbuf, size, 1, pf);
	fclose(pf);
	free(pconvertbuf);
	return 0;
	
}
