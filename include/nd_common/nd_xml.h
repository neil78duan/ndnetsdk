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

typedef int (*xml_errlog) (const char *errdesc) ;		//outpur error in parser xml text
//typedef struct tagxml ndxml ;
typedef struct tagxml ndxml_root;
//xml node
typedef struct tagxml 
{
	struct list_head  lst_self ;
	char name[MAX_XMLNAME_SIZE];
	char *value ;
	ndxml_root *parent;
	size_t val_size ;

	int attr_num ;
	struct list_head lst_attr; 

	int sub_num ;
	struct list_head lst_sub ;
	
} ndxml;


struct ndxml_attr {
	unsigned short name_size ;
	unsigned short value_size ;
	struct list_head lst; 
};

static __INLINE__ void init_xml_attr(struct ndxml_attr *attr) 
{
	attr->name_size = 0 ;
	attr->value_size = 0 ;
	INIT_LIST_HEAD(&attr->lst);
}

static __INLINE__ void init_xml_node(ndxml *xmlnode)
{
	xmlnode->name[0] = 0 ;
	xmlnode->value = 0 ;
	xmlnode->val_size = 0 ;

	xmlnode->attr_num = 0 ;
	xmlnode->sub_num = 0;
	xmlnode->parent = 0;
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

//set error callback 
ND_COMMON_API xml_errlog nd_setxml_log(xml_errlog logfunc) ;

ND_COMMON_API int ndxml_is_empty(ndxml *node);
//load 
ND_COMMON_API int ndxml_load(const char *file,ndxml_root *xmlroot) ;
ND_COMMON_API int ndxml_load_ex(const char *file, ndxml_root *xmlroot,const char*encodeType);
ND_COMMON_API int ndxml_load_from_buf(const char *fileName, const char *buf, size_t size, ndxml_root *xmlroot, const char *toEncodeType);
//get node 
ND_COMMON_API ndxml *ndxml_getnode(ndxml_root *xmlroot, const char *name) ;
ND_COMMON_API ndxml *ndxml_getnodei(ndxml_root *xmlroot, int index) ;

ND_COMMON_API ndxml* ndxml_big_brother(ndxml *node);
ND_COMMON_API ndxml* ndxml_little_brother(ndxml *node);


ND_COMMON_API int ndxml_merge(ndxml_root *host, ndxml_root *merged) ;
//add node 
ND_COMMON_API ndxml *ndxml_addnode(ndxml_root *xmlroot, const char *name,const char *value) ;

//create a node from text
ND_COMMON_API ndxml *ndxml_from_text(const char *text);
ND_COMMON_API size_t ndxml_to_buf(ndxml *node, char *buf, size_t size);

//delete node 
ND_COMMON_API int ndxml_delnode(ndxml_root *xmlroot, const char *name) ; //delete and free node 
ND_COMMON_API int ndxml_delnodei(ndxml_root *xmlroot, int index);
ND_COMMON_API int ndxml_delxml(ndxml *delnode, ndxml *xmlParent);
ND_COMMON_API int ndxml_unlink(ndxml *node, ndxml *xmlParent);		//unlink from parent , and not free memory
//ND_COMMON_API int ndxml_remove(ndxml *node, ndxml *xmlParent); //unlink node without dealloc-memory
ND_COMMON_API void ndxml_delall_children(ndxml* xml);
ND_COMMON_API void ndxml_delall_attrib(ndxml* xml);

#define ndxml_remove ndxml_unlink

static __INLINE__ int ndxml_disconnect(ndxml *xmlParent, ndxml *node)
{
	return ndxml_remove(node, xmlParent);
}

static __INLINE__ ndxml *ndxml_get_parent(ndxml *xmlnode)
{
	return xmlnode->parent;
}

ND_COMMON_API void ndxml_free(ndxml *xmlnode);
//destroy 
ND_COMMON_API void ndxml_destroy(ndxml_root *xmlroot) ; //destroy xmlroot's children and attributes but not free xmlroot self


ND_COMMON_API int ndxml_save(ndxml_root *xmlroot, const char *file) ;

ND_COMMON_API int ndxml_save_ex(ndxml_root *xmlroot, const char *file, const char*header);
ND_COMMON_API int ndxml_save_encode(ndxml_root *xmlroot, const char *file, int inputCode, int outputCode);


ND_COMMON_API ndxml *ndxml_copy(ndxml *node);
ND_COMMON_API int ndxml_insert(ndxml *parent, ndxml*child);
ND_COMMON_API int ndxml_insert_ex(ndxml *parent, ndxml*child, int index);

ND_COMMON_API int ndxml_insert_after(ndxml *parent, ndxml*insertNode, ndxml* brother); //insertNode after brother
ND_COMMON_API int ndxml_insert_before(ndxml *parent, ndxml*insertNode, ndxml*brother);//insertNode before brother

ND_COMMON_API int ndxml_get_index(ndxml *parent, ndxml*child);
ND_COMMON_API int ndxml_get_myindex(ndxml*xml);

//get node 
ND_COMMON_API ndxml *ndxml_refsub(ndxml *root, const char *name) ;
ND_COMMON_API ndxml *ndxml_refsubi(ndxml *root, int index) ;

//get value
ND_COMMON_API const char *ndxml_getval(ndxml *node);
ND_COMMON_API char *ndxml_getval_buf(ndxml *node, char *buf, size_t size);
ND_COMMON_API int ndxml_getval_int(ndxml *node);
ND_COMMON_API float ndxml_getval_float(ndxml *node);
//get attribute value
ND_COMMON_API struct ndxml_attr *ndxml_getattrib(ndxml *node , const char *name);
ND_COMMON_API struct ndxml_attr  *ndxml_getattribi(ndxml *node, int index);

//////////////////////////////////////////////////////////////////////////

ND_COMMON_API struct ndxml_attr  *ndxml_addattrib(ndxml *parent, const char *name, const char *value) ;

//set attribute value 
ND_COMMON_API int ndxml_setattrval(ndxml *parent, const char *name,const char *value) ;
ND_COMMON_API int ndxml_setattrvali(ndxml *parent, int index, const char *value) ;

//create a sub-node
ND_COMMON_API ndxml *ndxml_addsubnode(ndxml *parent, const char *name, const char *value) ;
//set value
ND_COMMON_API int ndxml_setval(ndxml *node, const char *val);
ND_COMMON_API int ndxml_setval_int(ndxml *node, int val);
ND_COMMON_API int ndxml_setval_float(ndxml *node, float val);
//del attribute node 
ND_COMMON_API int ndxml_delattrib(ndxml *parent, const char *name) ;
ND_COMMON_API int ndxml_delattribi(ndxml *parent, int index) ;
//delete sub node 
ND_COMMON_API int ndxml_delsubnode(ndxml *parent, const char *name);
ND_COMMON_API int ndxml_delsubnodei(ndxml *parent, int index) ;
ND_COMMON_API int ndxml_del_all_children(ndxml *parent);

ND_COMMON_API int ndxml_output(ndxml *node, FILE *pf);

// get xml node with recursive : ndxml_recursive_ref(xml, "../../node1/subnode")
ND_COMMON_API ndxml* ndxml_recursive_ref(ndxml *node, const char *xmlNodePath);
ND_COMMON_API const char* ndxml_recursive_getval(ndxml *node, const char *xmlNodePath);
ND_COMMON_API int ndxml_recursive_setval(ndxml *node, const char *xmlNodePath, const char *val);

// get xml node attribute value with recursive : ndxml_recursive_ref(xml, "../../node1/subnode.name")
ND_COMMON_API const char* ndxml_recursive_getattr(ndxml *node, const char *xmlAttrPathName);
ND_COMMON_API int ndxml_recursive_setattr(ndxml *node, const char *xmlAttrPathName,const char *attrVal);

//////////////////////////////////////////////////////////////////////////

static __INLINE__ const char *ndxml_getname(ndxml *node)
{
	if (!node) {
		return 0;
	}
	return node->name ;
}
static __INLINE__ int ndxml_getattr_num(ndxml *node)
{
	if (!node) {
		return 0;
	}
	return node->attr_num ;
}
static __INLINE__ int ndxml_num(ndxml_root *root)
{
	if (!root) {
		return 0;
	}
	return root->sub_num;
}
static __INLINE__ int ndxml_getsub_num(ndxml *node)
{
	if (!node) {
		return 0;
	}
	return node->sub_num ;
}
static __INLINE__ const char *ndxml_getattr_name(ndxml *node, int index )
{
	struct ndxml_attr  * attr =ndxml_getattribi(node,  index);
	if(attr) {
		const char *p = (const char*) (attr+1) ;
		return *p ? p : NULL;
	}
	return NULL ;
}

static __INLINE__ const char *ndxml_getattr_val(ndxml *node, const char *name )
{
	struct ndxml_attr  * attr =ndxml_getattrib(node,  name);
	if(attr) {
		const char *p = ((const char*)(attr + 1)) + attr->name_size;
		return *p ? p : NULL;
	}
	return NULL;
}

static __INLINE__ const char *ndxml_getattr_vali(ndxml *node, int index )
{
	struct ndxml_attr  * attr =ndxml_getattribi(node,  index);
	if(attr) {
		const char *p = ((const char*)(attr + 1)) + attr->name_size;
		return *p ? p : NULL;
	}
	return NULL;
}
#endif //_ND_XML_H_
