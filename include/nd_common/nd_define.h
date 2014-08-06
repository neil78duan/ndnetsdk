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

typedef unsigned int	ndtime_t ;	//时间 1/1000 s
typedef long long		ndbigtime_t;	//超级时间，time_t*1000+tick

typedef unsigned int ndip_t ;

typedef unsigned short ndport_t ;

#if defined(ND_UNICODE)
typedef unsigned short  NDBYTE;
#else 
typedef unsigned char	NDBYTE;
#endif

#define NDTRUE			1 
#define NDFALSE			0

#if !defined(ND_UNIX) 
typedef signed __int64		NDINT64  ;
typedef unsigned __int64	NDUINT64 ; 
//typedef signed long			ssize_t;

#else
typedef signed long long	INT64  ;
typedef signed long long	NDINT64  ;
typedef unsigned long long  NDUINT64 ;
typedef unsigned int		DWORD ;
typedef unsigned char 		BYTE ;
typedef unsigned short		WORD ;
#endif

#endif
