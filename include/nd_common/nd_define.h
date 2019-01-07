/* file nd_define.h
 *
 * define data type
 *
 * create by duan 
 *
 * 2011/4/2 11:10:35
 */
 
#ifndef _ND_DEFINE_H_
#define _ND_DEFINE_H_


typedef signed char 	NDINT8  ;
typedef unsigned char 	NDUINT8 ;

typedef signed short 	NDINT16  ;
typedef unsigned short 	NDUINT16 ;

typedef signed int		NDINT32  ;
typedef unsigned int	NDUINT32 ;

typedef unsigned int	NDBOOL ;

typedef unsigned int	ndtime_t ;		// time type  1/1000 s
typedef long long		ndbigtime_t;	//big time type£¬time_t*1000+tick

#if defined(ND_UNICODE)
typedef unsigned short  NDBYTE;
#else 
typedef unsigned char	NDBYTE;
#endif

#define NDTRUE			1 
#define NDFALSE			0

#define ND_ESC			0x1b
#define _NDT(x) x
typedef  char ndchar_t;

#if defined(_MSC_VER) 
typedef signed __int64		NDINT64  ;
typedef unsigned __int64	NDUINT64 ; 
//typedef signed long			ssize_t;
#else 
typedef signed long long	NDINT64  ;
typedef unsigned long long  NDUINT64;
#endif

#if !defined(ND_UNDEF_WIN32_TYPE) && !defined(_MSC_VER)
#ifndef INT64
typedef signed long long	INT64  ;
#endif 

#ifndef DWORD
typedef unsigned int		DWORD ;
#endif

#ifndef BYTE
typedef unsigned char 		BYTE; 
#endif

#ifndef WORD
typedef unsigned short		WORD ;
#endif

#endif


//typedef NDUINT32 ndip_t ;
//typedef NDUINT64 ndip_v6_t ;
typedef struct _ipdata {
	int sin_family;
	union {
		NDUINT32 ip;
		char ip6[16];
		NDUINT64 netIp[2];
	};
}ndip_t;

#define ND_IP_INIT {AF_INET}

typedef unsigned short ndport_t ;


#define ND_ELEMENTS_NUM(a) (sizeof(a)/sizeof(a[0]))

#define ND_MAKE_WORD(hiByte, loByte) ( ((loByte) & 0xff) | (((hiByte) & 0xff)<< 8) )
#define ND_MAKE_DWORD(hiWord, loWord) ( ((loWord) & 0xffff) | (((hiWord) & 0xffff)<< 16) )
#define ND_MAKE_QWORD(hiDword, loDword) ( ((NDUINT64)(loDword) & 0xffffffff) | (((NDUINT64)(hiDword) & 0xffffffff)<< 32) )

#define ND_LOBYTE(wordval) ((wordval)  & 0xff)
#define ND_HIBYTE(wordval) (((wordval)>>8)  & 0xff)

#define ND_LOWORD(dwordval) ((dwordval)  & 0xffff)
#define ND_HIWORD(dwordval) (((dwordval)>>16)  & 0xffff)

#define ND_LODWORD(qwordval) ((qwordval)  & 0xffffffff)
#define ND_HIDWORD(qwordval) (((qwordval)>>32)  & 0xffffffff)

//nd_error define 
enum END_ERROR_TYPE
{
#undef ErrorElement 
#define ErrorElement(_errId, _err_description) ND##_errId 
	NDERR_SUCCESS = 0,	//success
#include "_nderr.h"
	NDERR_SYS_MAX_NUMBER
#undef ErrorElement 
};

#define NDERR_USERDEFINE 1024 

#endif
