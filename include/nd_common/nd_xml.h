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

typedef void (*xml_errlog) (const char *errdesc) ;		//错误描述函数
//typedef struct tagxml ndxml ;
//xml 节点,可以单独操作
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

//xml的根节点,保存多个ndxml,
//主要是在文件加载和保存时使用
typedef struct tagxml ndxml_root;
// typedef struct tagxmlroot
// {
// 	int num ;
// 	struct list_head lst_xml ;	
// }ndxml_root;

//xml属性节点
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

//设置xml解析出错时的log函数,返回默认函数
ND_COMMON_API xml_errlog nd_setxml_log(xml_errlog logfunc) ;
<<<<<<< HEAD
//���ļ��м���һ��xml�б�
ND_COMMON_API int ndxml_load(const char *file, ndxml_root *xmlroot);
ND_COMMON_API int ndxml_load_ex(const char *file, ndxml_root *xmlroot,const char*encodeType);
=======
//从文件中加载一个xml列表
ND_COMMON_API int ndxml_load(const char *file,ndxml_root *xmlroot) ;
>>>>>>> 1c126e2cec5cc3d03c46d021c2f595fb454a905c

//查找到一个xml接点
ND_COMMON_API ndxml *ndxml_getnode(ndxml_root *xmlroot, const char *name) ;
ND_COMMON_API ndxml *ndxml_getnodei(ndxml_root *xmlroot, int index) ;

ND_COMMON_API int ndxml_merge(ndxml_root *host, ndxml_root *merged) ;
//添加一个xml节点
ND_COMMON_API ndxml *ndxml_addnode(ndxml_root *xmlroot, const char *name,const char *value) ;

//删除一个XML节点
ND_COMMON_API int ndxml_delnode(ndxml_root *xmlroot, const char *name) ;
ND_COMMON_API int ndxml_delnodei(ndxml_root *xmlroot, int index);
ND_COMMON_API int ndxml_delxml(ndxml *delnode, ndxml *xmlParent);
ND_COMMON_API int ndxml_remove(ndxml *node, ndxml *xmlParent); //remove not dealloc-memory


//销毁这个xml集合
ND_COMMON_API void ndxml_destroy(ndxml_root *xmlroot) ;

//把xml保存到文件中
ND_COMMON_API int ndxml_save(ndxml_root *xmlroot, const char *file) ;

ND_COMMON_API int ndxml_save_ex(ndxml_root *xmlroot, const char *file,const char*header) ;

ND_COMMON_API ndxml *ndxml_copy(ndxml *node);
ND_COMMON_API int ndxml_insert(ndxml *parent, ndxml*child);
ND_COMMON_API int ndxml_insert_ex(ndxml *parent, ndxml*child, int index);
ND_COMMON_API int ndxml_get_index(ndxml *parent, ndxml*child);

//引用一个子节点
ND_COMMON_API ndxml *ndxml_refsub(ndxml *root, const char *name) ;

//引用一个子节点
ND_COMMON_API ndxml *ndxml_refsub(ndxml *root, const char *name) ;

//通过下标引用一个子节点
ND_COMMON_API ndxml *ndxml_refsubi(ndxml *root, int index) ;

//得到xml的值
ND_COMMON_API char *ndxml_getval(ndxml *node);
ND_COMMON_API char *ndxml_getval_buf(ndxml *node, char *buf, size_t size);
ND_COMMON_API int ndxml_getval_int(ndxml *node);
ND_COMMON_API float ndxml_getval_float(ndxml *node);
//得到属性节点
ND_COMMON_API struct ndxml_attr *ndxml_getattrib(ndxml *node , const char *name);
ND_COMMON_API struct ndxml_attr  *ndxml_getattribi(ndxml *node, int index);

//////////////////////////////////////////////////////////////////////////
//给xml增加一个属性,
ND_COMMON_API struct ndxml_attr  *ndxml_addattrib(ndxml *parent, const char *name, const char *value) ;

//设置xml属性值
ND_COMMON_API int ndxml_setattrval(ndxml *parent, const char *name,const char *value) ;
ND_COMMON_API int ndxml_setattrvali(ndxml *parent, int index, const char *value) ;

//给xml增加一个子节点需要输入新节点的名字和值,返回新节点地址
ND_COMMON_API ndxml *ndxml_addsubnode(ndxml *parent, const char *name, const char *value) ;
//设置XML的值
ND_COMMON_API int ndxml_setval(ndxml *node , const char *val) ;
//删除一个属性节点
ND_COMMON_API int ndxml_delattrib(ndxml *parent, const char *name) ;
ND_COMMON_API int ndxml_delattribi(ndxml *parent, int index) ;
//删除一个子节点
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
