/* file : nd_trace.h
 * define function  trace some source used info
 *
 * version 1.0
 * neil duan
 * all right reserved 2007 by neil!
 * 2007-10-5 
 */

#ifndef _ND_TRACE_H_
#define _ND_TRACE_H_

#include <stdio.h>
#include <stdlib.h>

#include "nd_common/nd_comcfg.h"


 //��¼��Դ��ʹ�����
 //�ڳ����˳��Ժ����ȷ����Щ��Դû���ͷ�
 // 
 // nd_source_log ��¼��Դsource ������operate������� ����ʹ�����msg
 // int nd_sourcelog(void *source, char *operate, char *msg) ;
 // �ͷ���Դsource
 // it nd_source_release(void *source) ;
 // �����˳�,dump ��δ�ͷŵ���Դ,����Ҫ�ֶ��ͷ�
 // void nd_sourcelog_dump() ; 

#if defined(ND_FILE_TRACE) && defined(ND_SOURCE_TRACE)
#undef  fopen
#undef  fclose
#define fopen(filename, mod) nd_fopen_dbg(filename, mod,__FILE__,__LINE__)
#define fclose(fp)			nd_fclose_dbg(fp)
#endif


#endif
