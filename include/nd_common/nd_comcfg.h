/* file nd_comcfg.h
 *
 * config nd common
 *
 * 2009-12-6 22:15
 */

#ifndef _ND_COMCFG_H_
#define _ND_COMCFG_H_

#define ND_PLATFORM "x86"
#define ND_VERSION "3.0"

#define E_SRC_CODE_ANSI  	0
#define E_SRC_CODE_GBK		1
#define E_SRC_CODE_UTF_8	2

//source code compiled encode

#ifdef _MSC_VER
#define ND_ENCODE_TYPE E_SRC_CODE_GBK
#else
#define ND_ENCODE_TYPE E_SRC_CODE_UTF_8
#endif

#if defined(_GUI_TOOL_)

//#define ND_OPEN_TRACE		1
#define ND_OPEN_LOG_COMMON 	1
//#define ND_OPEN_LOG_DEBUG	1
#define ND_OPEN_LOG_WARN	1
#define ND_OPEN_LOG_ERROR	1
#define ND_OPEN_LOG_FATAL	1
//#define ND_SOURCE_TRACE	1
//#define ND_FILE_TRACE		1
//#define ND_OUT_LOG_2CTRL	1
//#define ND_OUT_LOG_2FILE	1
//#define ND_MEM_CHECK		1
#define ND_USE_MSGBOX		1
//#define ND_LOG_WITH_SOURCE	1
#define ND_LOG_WITH_TIME	1
//#define ND_MEM_STATICS	1
//#define ND_UNUSE_STDC_ALLOC 1
#else 


//#define ND_USE_GPERF		1		//ʹ��GOOGLE���ڴ��Ż���

#ifdef ND_DEBUG

#ifdef _MSC_VER
//#define ND_USE_VLD		1		//ʹ��VDL�����ڴ�й¶
#endif

#define ND_OVER_RIDE_NEW	1

#if defined(ND_USE_GPERF )||defined(ND_USE_VLD)
#else
//#define ND_UNUSE_STDC_ALLOC 1		//��ʹ�ñ�׼C��alloc
//#define ND_MEM_CHECK		1		//�ڴ�����Խ������
//#define ND_MEM_STATICS		1		//�ڲ�����ͳ��
//#define ND_SOURCE_TRACE		1		//��¼ÿ����������Դ(�򿪵��ļ����������ڴ�,����ϵͳ��Դ)
//#define ND_FILE_TRACE		1		//��¼ÿ���򿪵��ļ�
#endif

//#define ND_OPEN_TRACE 		1		//����VC���Դ�����������
#define ND_OPEN_LOG_COMMON 	1		//��������log����
#define ND_OPEN_LOG_DEBUG	1		//����debug log����
#define ND_OPEN_LOG_WARN	1		//�򿪾��� log
#define ND_OPEN_LOG_ERROR	1		//�򿪴���log
#define ND_OPEN_LOG_FATAL	1		//�������ش�������
#define ND_OUT_LOG_2CTRL	1		//����־����������̨
#define ND_OUT_LOG_2FILE	1		//����־�������ļ�
#define ND_USE_MSGBOX		1		//ʹ��messagebox
#define ND_LOG_WITH_SOURCE	1		//��־�������ļ���
#define ND_LOG_WITH_TIME	1		//��־������ʱ��
#else

//#define ND_USE_GPERF		1		//ʹ��GOOGLE���ڴ��Ż���
#if defined(ND_USE_GPERF )||defined(ND_USE_VLD)
#else
//#define ND_UNUSE_STDC_ALLOC 1		//��ʹ�ñ�׼C��alloc
//#define ND_MEM_CHECK		1		//�ڴ�����Խ������
//#define ND_MEM_STATICS		1		//�ڲ�����ͳ��
#endif
//#define ND_OPEN_TRACE		1		//����VC���Դ�����������
#define ND_OPEN_LOG_COMMON 	1		//��������log����
//#define ND_OPEN_LOG_DEBUG	1		//����debug log����
//#define ND_OPEN_LOG_WARN	1		//�򿪾��� log
#define ND_OPEN_LOG_ERROR	1		//�򿪴���log
#define ND_OPEN_LOG_FATAL	1		//�������ش�������
//#define ND_SOURCE_TRACE	1		//��¼ÿ����������Դ(�򿪵��ļ����������ڴ�,����ϵͳ��Դ)
//#define ND_FILE_TRACE		1		//��¼ÿ���򿪵��ļ�
#define ND_OUT_LOG_2CTRL	1		//����־����������̨
#define ND_OUT_LOG_2FILE	1		//����־�������ļ�
//#define ND_MEM_CHECK		1		//�ڴ�����Խ������
#define ND_USE_MSGBOX		1		//ʹ��messagebox
#define ND_LOG_WITH_SOURCE	1		//��־�������ļ���
#define ND_LOG_WITH_TIME	1		//��־������ʱ��
//#define ND_MEM_STATICS		1		//�ڲ�����ͳ��

#endif			//end debug
#endif 			//ANDROID

#define ND_BUFSIZE 4096						//Ĭ�ϻ�����С(��ʱ����,��Ҫ�޸�)

#define ND_FILE_PATH_SIZE	256

#define NOT_SUPPORT_THIS_FUNCTION 0			//��assert����ʾ��֧�ֵĹ���

#define ND_MULTI_THREADED 1				//ʹ�ö��߳�(����)

#define ND_MAX_THREAD_NUM 16

#define ND_CALLSTACK_TRACE	1			//���ٺ������ö�ջ

#ifndef BUILD_AS_STATIC_LIB
	#define BUILD_AS_STATIC_LIB
#endif
// 
// #ifdef BUILD_AS_STATIC_LIB
// #else
// #define ND_COMPILE_AS_DLL	1			//�����ɶ�̬��
// #endif

//��֧��unicode
//#define ND_UNICODE			1

//without iconv
//#define WITHOUT_ICONV 1

#endif
