/* file nd_xml.h
 * define xml parser of nd engine 
 * version 1.0 
 * 2008-8-24 
 * all right reserved by duan xiuyun 
 */

#ifndef _ND_XML_H_
#define _ND_XML_H_
#include "nd_common/list.h"
#define MAX_XMLNAME_SIZE 64

typedef void (*xml_errlog) (const char *errdesc) ;		//������������
//typedef struct tagxml ndxml ;
//xml �ڵ�,���Ե�������
typedef struct tagxml 
{
	struct list_head  lst_self ;
	char name[MAX_XMLNAME_SIZE];
	char *value ;
	size_t val_size ;

	int attr_num ;
	struct list_head lst_attr; 

	int sub_num ;
	struct list_head lst_sub ;
	
} ndxml;

//xml�ĸ��ڵ�,������ndxml,
//��Ҫ�����ļ����غͱ���ʱʹ��
typedef struct tagxml ndxml_root;
// typedef struct tagxmlroot
// {
// 	int num ;
// 	struct list_head lst_xml ;	
// }ndxml_root;

//xml���Խڵ�
/*struct ndxml_attr {
	char *name ;
	char *value ;
	struct list_head lst; 
};
*/
struct ndxml_attr {
	unsigned short name_size ;
	unsigned short value_size ;
	struct list_head lst; 
};

static __INLINE__ void init_xml_attr(struct ndxml_attr *attr) 
{
	attr->name_size = 0 ;
	attr->value_size = 0 ;
	//attr->name = 0 ;
	//attr->value = 0 ;
	INIT_LIST_HEAD(&attr->lst);
}

static __INLINE__ void init_xml_node(ndxml *xmlnode)
{
	xmlnode->name[0] = 0 ;
	xmlnode->value = 0 ;
	xmlnode->val_size = 0 ;

	xmlnode->attr_num = 0 ;
	xmlnode->sub_num = 0;
	INIT_LIST_HEAD(&xmlnode->lst_self);
	INIT_LIST_HEAD(&xmlnode->lst_attr);
	INIT_LIST_HEAD(&xmlnode->lst_sub);
}

static __INLINE__ void ndxml_initroot(ndxml_root *root)
{
	init_xml_node((ndxml *)root);
// 	root->num = 0 ;
// 	INIT_LIST_HEAD(&root->lst_xml) ;
}

//����xml��������ʱ��log����,����Ĭ�Ϻ���
ND_COMMON_API xml_errlog nd_setxml_log(xml_errlog logfunc) ;
//���ļ��м���һ��xml�б�
ND_COMMON_API int ndxml_load(const char *file,ndxml_root *xmlroot) ;

//���ҵ�һ��xml�ӵ�
ND_COMMON_API ndxml *ndxml_getnode(ndxml_root *xmlroot, const char *name) ;
ND_COMMON_API ndxml *ndxml_getnodei(ndxml_root *xmlroot, int index) ;

ND_COMMON_API int ndxml_merge(ndxml_root *host, ndxml_root *merged) ;
//���һ��xml�ڵ�
ND_COMMON_API ndxml *ndxml_addnode(ndxml_root *xmlroot, const char *name,const char *value) ;

//ɾ��һ��XML�ڵ�
ND_COMMON_API int ndxml_delnode(ndxml_root *xmlroot, const char *name) ;
ND_COMMON_API int ndxml_delnodei(ndxml_root *xmlroot, int index);
ND_COMMON_API int ndxml_delxml(ndxml *delnode, ndxml *xmlParent);

//�������xml����
ND_COMMON_API void ndxml_destroy(ndxml_root *xmlroot) ;

//��xml���浽�ļ���
ND_COMMON_API int ndxml_save(ndxml_root *xmlroot, const char *file) ;

ND_COMMON_API int ndxml_save_ex(ndxml_root *xmlroot, const char *file,const char*header) ;

ND_COMMON_API ndxml *ndxml_copy(ndxml *node);
ND_COMMON_API int ndxml_insert(ndxml *parent, ndxml*child);

//����һ���ӽڵ�
ND_COMMON_API ndxml *ndxml_refsub(ndxml *root, const char *name) ;

//����һ���ӽڵ�
ND_COMMON_API ndxml *ndxml_refsub(ndxml *root, const char *name) ;

//ͨ���±�����һ���ӽڵ�
ND_COMMON_API ndxml *ndxml_refsubi(ndxml *root, int index) ;

//�õ�xml��ֵ
ND_COMMON_API char *ndxml_getval(ndxml *node);
ND_COMMON_API char *ndxml_getval_buf(ndxml *node, char *buf, size_t size);
ND_COMMON_API int ndxml_getval_int(ndxml *node);
ND_COMMON_API float ndxml_getval_float(ndxml *node);
//�õ����Խڵ�
ND_COMMON_API struct ndxml_attr *ndxml_getattrib(ndxml *node , const char *name);
ND_COMMON_API struct ndxml_attr  *ndxml_getattribi(ndxml *node, int index);

//////////////////////////////////////////////////////////////////////////
//��xml����һ������,
ND_COMMON_API struct ndxml_attr  *ndxml_addattrib(ndxml *parent, const char *name, const char *value) ;

//����xml����ֵ
ND_COMMON_API int ndxml_setattrval(ndxml *parent, const char *name,const char *value) ;
ND_COMMON_API int ndxml_setattrvali(ndxml *parent, int index, const char *value) ;

//��xml����һ���ӽڵ���Ҫ�����½ڵ�����ֺ�ֵ,�����½ڵ��ַ
ND_COMMON_API ndxml *ndxml_addsubnode(ndxml *parent, const char *name, const char *value) ;
//����XML��ֵ
ND_COMMON_API int ndxml_setval(ndxml *node , const char *val) ;
//ɾ��һ�����Խڵ�
ND_COMMON_API int ndxml_delattrib(ndxml *parent, const char *name) ;
ND_COMMON_API int ndxml_delattribi(ndxml *parent, int index) ;
//ɾ��һ���ӽڵ�
ND_COMMON_API int ndxml_delsubnode(ndxml *parent, const char *name);
ND_COMMON_API int ndxml_delsubnodei(ndxml *parent, int index) ;

ND_COMMON_API int ndxml_output(ndxml *node, FILE *pf);

//////////////////////////////////////////////////////////////////////////

static __INLINE__ const char *ndxml_getname(ndxml *node)
{
	return node->name ;
}
static __INLINE__ int ndxml_getattr_num(ndxml *node)
{
	return node->attr_num ;
}
static __INLINE__ int ndxml_num(ndxml_root *root)
{
	return root->sub_num;
}
static __INLINE__ int ndxml_getsub_num(ndxml *node)
{
	return node->sub_num ;
}
static __INLINE__ const char *ndxml_getattr_name(ndxml *node, int index )
{
	struct ndxml_attr  * attr =ndxml_getattribi(node,  index);
	if(attr) {
		return (const char*) (attr+1) ;
	}
	return NULL ;
}

static __INLINE__ const char *ndxml_getattr_val(ndxml *node, const char *name )
{
	struct ndxml_attr  * attr =ndxml_getattrib(node,  name);
	if(attr) {
		return  ((const char*) (attr+1) ) + attr->name_size ;
	}
	return NULL;
}

static __INLINE__ const char *ndxml_getattr_vali(ndxml *node, int index )
{
	struct ndxml_attr  * attr =ndxml_getattribi(node,  index);
	if(attr) {
		return  ((const char*) (attr+1) ) + attr->name_size ;
	}
	return NULL;
}
#endif //_ND_XML_H_
