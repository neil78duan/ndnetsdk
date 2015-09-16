//
//  nd_iconv.c
//  gameHall
//
//  Created by duanxiuyun on 15-5-11.
//  Copyright (c) 2015 duanxiuyun. All rights reserved.
//

#include "nd_common/nd_iconv.h"

// ------------------------------------------------------------------------------------------
#ifdef _MSC_VER


char * nd_gbk_to_utf8(const char *input_text, char *output_buf, int size_buf)
{
	int input_len = strlen(input_text);
	int insize = ( input_len+1) * sizeof(wchar_t);
	
	wchar_t *pWbuf = malloc(insize);
	if (!pWbuf) {
		return NULL;
	}
	
	int wlen =MultiByteToWideChar(CP_ACP, 0, input_text,input_len , pWbuf, insize);
	pWbuf[wlen] = 0;
	
	int nLen = WideCharToMultiByte(CP_UTF8 , 0, pWbuf, wlen, output_buf, size_buf, NULL, NULL);
	output_buf[nLen] = 0 ;
	
	free(pWbuf);
	
	return  output_buf;
}

char * nd_utf8_to_gbk(const char *input_text, char *output_buf, int size_buf)
{
	int input_len = strlen(input_text);
	int insize = ( input_len+1) * sizeof(wchar_t);
	
	wchar_t *pWbuf = malloc(insize);
	if (!pWbuf) {
		return NULL;
	}
	
	int wlen = MultiByteToWideChar(CP_UTF8, 0, input_text, input_len, pWbuf, insize);
	pWbuf[wlen] = 0;
	
	int nLen = WideCharToMultiByte(CP_ACP, 0, pWbuf, wlen, output_buf, size_buf, NULL, NULL);
	output_buf[nLen] = 0 ;
	
	free(pWbuf);
	
	return  output_buf;
	
}
#elif defined(ND_ANDROID)

char * nd_gbk_to_utf8(const char *input_text, char *output_buf, int size_buf)
{
	return (char*)input_text;
}

char * nd_utf8_to_gbk(const char *input_text, char *output_buf, int size_buf)
{
	return (char*)input_text;
}

#else
#include <iconv.h>
static int code_convert(const char *from_charset,const char *to_charset, char *inbuf,int inlen,char *outbuf,int outlen)
{
	iconv_t cd;
	//int rc;
	char **pin = &inbuf;
	char **pout = &outbuf;
	
	cd = iconv_open(to_charset,from_charset);
	if (cd==0)  {
		nd_logerror("iconv_open %s\n", nd_last_error() ) ;
		return -1;
	}
	memset(outbuf,0,outlen);
	size_t in_buf=inlen;
	size_t out_buf=outlen;
	if (iconv(cd,pin,&in_buf,pout,&out_buf)==-1) {
		nd_logerror("iconv %s\n", nd_last_error() ) ;
		return -1;
	}
	iconv_close(cd);
	return 0;
}

char * nd_gbk_to_utf8(const char *input_text, char *output_buf, int size_buf)
{
	int inlen = strlen(input_text) ;
	int ret = code_convert("GBK","UTF-8",(char*)input_text,inlen,output_buf,size_buf);
	if (0==ret) {
		return output_buf ;
	}
	return NULL ;
}

char * nd_utf8_to_gbk(const char *input_text, char *output_buf, int size_buf)
{
	int inlen = strlen(input_text) ;
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
	return strncpy(outbuf,input_text,size) ;
#endif

}
char * nd_ndcode_to_utf8(const char *input_text, char *outbuf, int size)
{
#if (ND_ENCODE_TYPE==E_SRC_CODE_GBK)
	return nd_gbk_to_utf8(input_text, outbuf, size);
#else 
	return strncpy(outbuf, input_text, size);
#endif


}