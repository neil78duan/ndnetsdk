/* file ndchar.h
 * define ND engine character-set operate function
 * ver 1.0
 * neil duan
 * 2007-9-28 
 * all right reserved 2007 by neil
 */

/*
 * ndchar ������TCHAR
 * ��Ϊ�Ҳ���ֱ�ʵ��ansi��unicode�����ư汾,ֻ�����ڱ���ʱ�������ѡ��!
 */
#ifndef _NDCHAR_H_
#define _NDCHAR_H_

#include <wchar.h>

#define ND_ESC		0x1b
#if !defined(ND_UNIX) 
#if _MSC_VER < 1300 // 1200 == VC++ 6.0

#define snprintf 		_snprintf
#define snwprintf 		_snwprintf
#define vsnwprintf		_vsnwprintf
#define vsnprintf		_vsnprintf

#else 
#endif

#endif 

#ifdef ND_UNICODE

#define ndchar_t wchar_t
#define _NDT(x)		L ## x
#define ndisalnum     iswalnum		//�����ַ��Ƿ�Ϊ���ֻ���ĸ 
#define ndisalpha     iswalpha 		// �����ַ��Ƿ�����ĸ 
#define ndiscntrl     iswcntrl 		//�����ַ��Ƿ��ǿ��Ʒ� 
#define ndisdigit     iswdigit 		//�����ַ��Ƿ�Ϊ���� 
#define ndisgraph     iswgraph 		//�����ַ��Ƿ��ǿɼ��ַ� 
#define ndislower     iswlower 		//�����ַ��Ƿ���Сд�ַ� 
#define ndisprint     iswprint 		//�����ַ��Ƿ��ǿɴ�ӡ�ַ� 
#define ndispunct     iswpunct 		//�����ַ��Ƿ��Ǳ����� 
#define ndisspace     iswspace 		//�����ַ��Ƿ��ǿհ׷��� 
#define ndisupper     iswupper 		//�����ַ��Ƿ��Ǵ�д�ַ� 
#define ndisxdigit    iswxdigit		//�����ַ��Ƿ���ʮ�����Ƶ����� 

#define ndtolower     towlower 		//���ַ�ת��ΪСд 
#define ndtoupper     towupper 		//���ַ�ת��Ϊ��д 
#define ndcscoll      wcscoll 		//�Ƚ��ַ��� 

/*
��ӡ��ɨ���ַ����� 
���ַ��������� 
*/
#define ndfprintf		fwprintf     //ʹ��vararg�����ĸ�ʽ����� 
#define ndprintf		wprintf      //ʹ��vararg�����ĸ�ʽ���������׼��� 
#define ndsprintf		swprintf     //����vararg�������ʽ�����ַ��� 
#define ndvfprintf		vfwprintf    //ʹ��stdarg�������ʽ��������ļ� 
#define ndvsprintf		vsnwprintf    //��ʽ��stdarg������д���ַ��� 

#define ndsnprintf 		snwprintf

#define ndstrtod 		wcstod    //�ѿ��ַ��ĳ�ʼ����ת��Ϊ˫���ȸ����� 
#define ndstrtol		wcstol     //�ѿ��ַ��ĳ�ʼ����ת��Ϊ������ 
#define ndstrtoul		wcstoul    //���ַ��ĳ�ʼ����ת��Ϊ�޷��ų����� 

/*
�ַ��������� 
���ַ�����        ��ͨC�������� 
*/
#define ndwitch		wcwidth			//���Ե����ַ�ռ�õ��ڴ���
#define ndstrcat	wcscat			//strcat��һ���ַ����ӵ���һ���ַ�����β�� 
#define ndstrncat	wcsncat			//strncat
#define ndstrchr	wcschr			//strchr�������ַ����ĵ�һ��λ�� 
#define ndstrrchr	wcsrchr			// strrchr����     ��β����ʼ�������ַ������ֵĵ�һ��λ�� 
#define ndstrpbrk	wcspbrk         //strpbrk����     ��һ�ַ��ַ����в�����һ�ַ������κ�һ���ַ���һ�γ��ֵ�λ�� 
#define ndstrstr	wcsstr			//    strstr����     ��һ�ַ����в�����һ�ַ�����һ�γ��ֵ�λ�� 
#define ndstrcspn	wcscspn			//����         strcspn����     ���ز������ڶ����ַ����ĵĳ�ʼ��Ŀ 
#define ndstrspn	wcsspn			//����         strspn����     ���ذ����ڶ����ַ����ĳ�ʼ��Ŀ 
#define ndstrcpy	wcscpy			//����         strcpy����     �����ַ��� 
#define ndstrncpy	wcsncpy			//����         strncpy����     ������wcscpy������ ͬʱָ����������Ŀ 
#define ndstrcmp	wcscmp			//����         strcmp����     �Ƚ��������ַ��� 
#define ndstrncmp	wcsncmp			//����         strncmp����     ������wcscmp������ ��Ҫָ���Ƚ��ַ��ַ�������Ŀ 
#define ndstrlen	wcslen			//����         strlen����     ��ÿ��ַ�������Ŀ 
#define ndstrtok	wcstok			//����         strtok����     ���ݱ�ʾ���ѿ��ַ����ֽ��һϵ���ַ��� 

#else		//ansi
#define _NDT(x) x
#define ndchar_t char

#define ndisalnum     isalnum		//�����ַ��Ƿ�Ϊ���ֻ���ĸ 
#define ndisalpha     isalpha 		// �����ַ��Ƿ�����ĸ 
#define ndiscntrl     iscntrl 		//�����ַ��Ƿ��ǿ��Ʒ� 
#define ndisdigit     isdigit 		//�����ַ��Ƿ�Ϊ���� 
#define ndisgraph     isgraph 		//�����ַ��Ƿ��ǿɼ��ַ� 
#define ndislower     islower 		//�����ַ��Ƿ���Сд�ַ� 
#define ndisprint     isprint 		//�����ַ��Ƿ��ǿɴ�ӡ�ַ� 
#define ndispunct     ispunct 		//�����ַ��Ƿ��Ǳ����� 
#define ndisspace     isspace 		//�����ַ��Ƿ��ǿհ׷��� 
#define ndisupper     isupper 		//�����ַ��Ƿ��Ǵ�д�ַ� 
#define ndisxdigit    isxdigit		//�����ַ��Ƿ���ʮ�����Ƶ����� 

#define ndtolower     tolower 		//���ַ�ת��ΪСд 
#define ndtoupper     toupper 		//���ַ�ת��Ϊ��д 
#define ndcscoll     strcoll 		//�Ƚ��ַ��� 

/*
��ӡ��ɨ���ַ����� 
���ַ��������� 
*/
#define ndwitch(ch)		1			//���Ե����ַ�ռ�õ��ڴ���
#define ndfprintf		fprintf     //ʹ��vararg�����ĸ�ʽ����� 
#define ndprintf		printf      //ʹ��vararg�����ĸ�ʽ���������׼��� 
#define ndsprintf		sprintf     //����vararg�������ʽ�����ַ��� 
#define ndvfprintf		vfprintf    //ʹ��stdarg�������ʽ��������ļ� 
#define ndvsprintf		vsnprintf    //��ʽ��stdarg������д���ַ��� 
#define ndsnprintf 		snprintf

#define ndstrtod 		strtod		//�ѿ��ַ��ĳ�ʼ����ת��Ϊ˫���ȸ����� 
#define ndstrtol		strtol		//�ѿ��ַ��ĳ�ʼ����ת��Ϊ������ 
#define ndstrtoul		strtoul		//�ѿ��ַ��ĳ�ʼ����ת��Ϊ�޷��ų����� 

/*
�ַ��������� 
���ַ�����        ��ͨC�������� 
*/
#define ndstrcat	strcat			//��һ���ַ����ӵ���һ���ַ�����β�� 
#define ndstrncat	strncat			//����ָ��ճ���ַ�����ճ�ӳ���. 
#define ndstrchr	strchr			//�������ַ����ĵ�һ��λ�� 
#define ndstrrchr	strrchr			//��β����ʼ�������ַ������ֵĵ�һ��λ�� 
#define ndstrpbrk	strpbrk			//��һ�ַ��ַ����в�����һ�ַ������κ�һ���ַ���һ�γ��ֵ�λ�� 
#define ndstrstr	strstr			//��һ�ַ����в�����һ�ַ�����һ�γ��ֵ�λ�� 
#define ndstrcspn	strcspn			//���ز������ڶ����ַ����ĵĳ�ʼ��Ŀ 
#define ndstrspn	strspn			//���ذ����ڶ����ַ����ĳ�ʼ��Ŀ 
#define ndstrcpy	strcpy			//�����ַ��� 
#define ndstrncpy	strncpy			//ͬʱָ����������Ŀ 
#define ndstrcmp	strcmp			//�Ƚ��������ַ��� 
#define ndstrncmp	strncmp			//ָ���Ƚ��ַ��ַ�������Ŀ 
#define ndstrlen	strlen			// ��ÿ��ַ�������Ŀ 
#define ndstrtok	strtok

#endif //ND_UNICODE

#endif
