/* file nd_xml.c
 * implement xml parser of nd engine 
 * version 1.0 
 * 2008-8-24 
 * all right reserved by duan xiuyun 
 */

#include "nd_common/nd_common.h"
#include "nd_common/nd_iconv.h"

#define MAX_FILE_SIZE 4*4096
#define XML_H_END   0x3e2f			// /> xml node end mark
#define XML_T_END   0x2f3c			// </ xml end node mark,���ֽ��ڸߵ�ַ /��ַ���� >
static int  _is_mark_end(const char *p)
{
	if (*p == 0x2f && *(p+1)=='>') {
		return 1;
	}
	return 0;
}

static int  _is_mark_start(const char *p)
{
	if (*p == '<' && *(p+1)==0x2f) {
		return 1;
	}
	return 0;
}

ndxml *parse_xmlbuf(const char *xmlbuf, int size, const char **parse_end, const char **error_addr) ;
ndxml *alloc_xml();
void _release_xml(ndxml *node);
void  dealloc_xml(ndxml *node );
struct ndxml_attr *alloc_attrib_node(const char *name, const char *value);
void dealloc_attrib_node(struct ndxml_attr *pnode);
int xml_write(ndxml *xmlnode, FILE *fp , int deep) ;
ndxml *_create_xmlnode(const char *name, const char *value)  ;

static void _errlog (const char *errdesc) ;
static void show_xmlerror(const char *file, const char *error_addr, const char *xmlbuf, size_t size) ;

static int xml_parse_fileinfo(ndxml_root *xmlroot, char *start, char **endaddr, char **erraddr);

static int xml_set_code_type(ndxml_root *xmlroot);
static xml_errlog __xml_logfunc = _errlog ;


int xml_load_from_buf(const char *buf, size_t size, ndxml_root *xmlroot,const char *filename)
{
	int data_len = (int)size;
	const char *parse_addr =  buf;
	const char *error_addr = 0;
	do {
		parse_addr = ndstr_first_valid(parse_addr) ;
		if (parse_addr) {
			ndxml *xmlnode = parse_xmlbuf(parse_addr, data_len, &parse_addr, &error_addr);
			if (xmlnode) {
				list_add_tail(&xmlnode->lst_self, &xmlroot->lst_sub);
				xmlnode->parent = xmlroot;
				xmlroot->sub_num++;
			}
			else if (error_addr) {
				ndxml_destroy(xmlroot);
				show_xmlerror(filename, error_addr, buf, (size_t)data_len);
				return -1;
			}
		}
		
	} while (parse_addr && *parse_addr);

	return 0;
}

int ndxml_load(const char *file,ndxml_root *xmlroot)
{
	return ndxml_load_ex(file, xmlroot, NULL);	
}

int ndxml_load_ex(const char *file, ndxml_root *xmlroot, const char*encodeType)
{
	int codeType = -1;
	int ret = 0;
	size_t size = 0;
	char *pAddr, *pErrorAddr=0;
	const char *pTextEncode = 0;
	char*pBuf = nd_load_file(file, &size);
	if (!pBuf){
		return -1;
	}
	//ndxml_root *xmlroot;
	ndxml_initroot(xmlroot);
	if (-1 == xml_parse_fileinfo(xmlroot, pBuf, &pAddr, &pErrorAddr)) {
		ret = -1;
		ndxml_destroy(xmlroot);
		show_xmlerror(file, pErrorAddr, pBuf, (size_t)size);
		goto EXIT_ERROR ;
	}
	
	pTextEncode = ndxml_getattr_val(xmlroot,"encoding");
	if (pTextEncode && *pTextEncode){
		if (encodeType && encodeType[0]) {
			int nTextType = nd_get_encode_val(pTextEncode);
			int nNeedType = nd_get_encode_val(encodeType);
			//char *pconvertbuf;
			//const char *encodeTextType = 0;
			nd_code_convert_func func = nd_get_code_convert(nTextType, nNeedType);
// 			typedef char*(*__convert_function)(const char *, char *, int);
// 			__convert_function func = NULL;
// 			//CONVERT CODE 
// 			if (nNeedType == E_SRC_CODE_UTF_8 && nTextType != E_SRC_CODE_UTF_8)	{
// 				func = nd_gbk_to_utf8;
// 			}
// 			else if (nNeedType != E_SRC_CODE_UTF_8 && nTextType == E_SRC_CODE_UTF_8)	{
// 				func = nd_utf8_to_gbk;	
// 			}

			if (func){
				char *pconvertbuf = malloc(size * 2);
				if (!pconvertbuf)	{
					ret = -1;
					goto EXIT_ERROR;
				}
				if (func(pBuf, pconvertbuf, size * 2)) {
					nd_unload_file(pBuf);
					pBuf = pconvertbuf;
					size = strlen(pconvertbuf);
				}
				else {
					free(pconvertbuf);
					ret = -1;
					goto EXIT_ERROR;
				}
				ndxml_setattrval(xmlroot, "encoding", encodeType);
			}
			codeType = ndstr_set_code(nNeedType);
		}
		else {
			codeType = xml_set_code_type(xmlroot);
		}
	}
		

	if (-1 == xml_load_from_buf(pBuf, size, xmlroot, file)) {
		ret = -1;
	}

	if (codeType != -1) {
		ndstr_set_code(codeType);
	}

EXIT_ERROR :
	nd_unload_file(pBuf);
	return ret;

}

void ndxml_destroy(ndxml_root *xmlroot)
{
// 	ndxml *sub_xml; 
// 	struct list_head *pos = xmlroot->lst_sub.next;
// 	while (pos != &xmlroot->lst_sub) {
// 		sub_xml = list_entry(pos,struct tagxml, lst_self) ;
// 		pos = pos->next ;
// 		dealloc_xml(sub_xml) ;
// 		
// 	}
// 
// 
// 	//dealloc attribute
// 	pos = xmlroot->lst_attr.next;
// 	while (pos != &xmlroot->lst_attr) {
// 		struct ndxml_attr *attrnode;
// 		attrnode = list_entry(pos, struct ndxml_attr, lst);
// 		pos = pos->next;
// 		dealloc_attrib_node(attrnode);
// 	}


	_release_xml(xmlroot);
	ndxml_initroot(xmlroot) ;
}

int xml_set_code_type(ndxml_root *xmlroot)
{
	int oldType = -1;
	const char *p = ndxml_getattr_val(xmlroot, "encoding");
	if (p){
		oldType = nd_get_encode_val(p);

		oldType = ndstr_set_code(oldType);
	}
	return oldType;
}
int xml_parse_fileinfo(ndxml_root *xmlroot, char *start, char **endaddr, char **erraddr)
{
	const char *p = start;
	const char *pEnd;
	
	*endaddr = start;

	p = ndstr_first_valid(p);
	if (!p){
		*erraddr = start;
		return -1;
	}
	
	p = strstr(p, "<?");
	if (!p){
		return 0;
	}

	p = strstr(p, "xml");
	if (!p){
		return 0;
	}
	p += 3;

	pEnd = strstr(p, "?>");
	if (!pEnd){
		*endaddr =(char*) p;
		*erraddr = (char*)p;
		return -1;
	}

	//parse attribute 
	while (p < pEnd) {
		struct ndxml_attr *attrib_node;
		char attr_name[MAX_XMLNAME_SIZE];
		char valbuf[1024];

		p = ndstr_first_valid(p);
		*endaddr = (char*)p;
		*erraddr = start;
		
		attr_name[0] = 0;
		p = ndstr_parse_word(p, attr_name);
		if (attr_name[0] == 0)	{
			break;
		}
		p = strchr(p, _ND_QUOT);
		if (!p) {
			return -1;
		}
		++p;
		//read attrib VALUE
		valbuf[0] = 0;
		p = ndstr_str_end(p, valbuf, _ND_QUOT);
		++p;

		attrib_node = alloc_attrib_node(attr_name, valbuf);
		if (attrib_node) {
			list_add_tail(&attrib_node->lst, &xmlroot->lst_attr);
			(xmlroot->attr_num)++;
		}
	}


	*endaddr = (char*)pEnd + 2;
	*erraddr = 0;
	return 0;

}
//��ʾxml����
void show_xmlerror(const char *file, const char *error_addr, const char *xmlbuf, size_t size)
{
	int line = 0 ;
	const char *pline, *pnext ;
	char errbuf[1024] ;

	errbuf[0] = 0 ;
	pline = xmlbuf ;

	//pnext = pline ;

	while (pline<xmlbuf+size) {
		//pline = pnext ;
		pnext = strchr(pline, '\n') ;
		if(!pnext) {
			snprintf(errbuf, 1024, "know error in file %s  line %d", file, line) ;
			break ;				
		}
		pnext  ;
		++line ;
		if(error_addr>=pline && error_addr< pnext) {
			pline = ndstr_first_valid(pline) ;
			snprintf(errbuf, 1024, "parse error in file %s  line %d \n", file, line) ;
			break ;				
		}
		pline = ++pnext ;
	}
	
	if(__xml_logfunc && errbuf[0]) {
		__xml_logfunc(errbuf) ;
	}
	return ;
}

 xml_errlog nd_setxml_log(xml_errlog logfunc) 
 {
	 xml_errlog ret =  __xml_logfunc ;
	 __xml_logfunc = logfunc ;
	 return ret ;
 }

int ndxml_save_ex(ndxml_root *xmlroot, const char *file,const char*header)
{
    FILE *fp;
    ndxml *sub_xml;
	struct list_head *pos = xmlroot->lst_sub.next;
    
    fp = fopen(file, "w") ;
    if(!fp) {
        return -1;
    }
    if (header && header[0]) {
        fprintf(fp, "<?xml %s ?>\n", header) ;
    }
    
	while (pos != &xmlroot->lst_sub) {
        sub_xml = list_entry(pos,struct tagxml, lst_self) ;
        pos = pos->next ;
        xml_write(sub_xml,fp, 0) ;
        
    }
    
    fclose(fp) ;
    
    return 0;
    
}

int ndxml_save(ndxml_root *xmlroot, const char *file)
{
	char buf[4096];
	struct list_head *pos;
	int size = 0;
	char *p = buf;
	int bufsize = sizeof(buf);

	buf[0] = 0;
	//save attribute to file 
	pos = xmlroot->lst_attr.next;
	while (pos != &xmlroot->lst_attr){
		struct ndxml_attr *xml_attr = list_entry(pos, struct ndxml_attr, lst);
		char *attr_val1 = (char*)(xml_attr + 1) + xml_attr->name_size;
		pos = pos->next;
		if (attr_val1[0]) {
			size = snprintf(p, bufsize, " %s=\"%s\"", (char*)(xml_attr + 1), attr_val1);
			bufsize -= size;
			p += size;
		}
		else {
			size =  snprintf(p, bufsize, " %s=\"\"", (char*)(xml_attr + 1));
			bufsize -= size;
			p += size;
		}
	}
	if (buf[0] == 0){
#if defined(ND_GB2312)  || defined(ND_GBK)
		snprintf(p, bufsize, "version=\"1.0\" encoding=\"GBK\"");
#elif defined(ND_UTF_8)
		snprintf(p, bufsize, "version=\"1.0\" encoding=\"utf-8\"");
#else 
		snprintf(p, bufsize, "version=\"1.0\" encoding=\"ANSI\"");
#endif
	}
    return ndxml_save_ex(xmlroot, file, buf) ;
}


int ndxml_save_encode(ndxml_root *xmlroot, const char *file, int inputCode, int outputCode)
{
	char buf[128];
	buf[0] = 0;
	
	if (outputCode == E_SRC_CODE_GBK){
		snprintf(buf, sizeof(buf), "version=\"1.0\" encoding=\"GBK\"");
	}
	else if (outputCode == E_SRC_CODE_UTF_8){
		snprintf(buf, sizeof(buf), "version=\"1.0\" encoding=\"utf-8\"");
	}
	else {
		snprintf(buf, sizeof(buf), "version=\"1.0\" encoding=\"ANSI\"");
	}

		
	int ret = ndxml_save_ex(xmlroot, file, buf); 
	nd_code_convert_func func = nd_get_code_convert(inputCode, outputCode);

	if (func){
		return nd_code_convert_file(file, inputCode, outputCode);
	}
	return ret;
}

ndxml *ndxml_copy(ndxml *node)
{
	int i;
	ndxml *newnode = NULL ;
	
	if (!node) {
		return NULL;
	}
	
	newnode = _create_xmlnode(node->name, node->value) ;
	
	for (i=0; i<ndxml_getattr_num(node); ++i) {
		ndxml_addattrib(newnode, ndxml_getattr_name(node, i), ndxml_getattr_vali(node, i)) ;
	}
	for (i=0; i<ndxml_getsub_num(node); ++i) {
		ndxml *sub1 = ndxml_refsubi(node, i);
		ndxml *sub_new = ndxml_copy(sub1);
		if (sub_new) {
			list_add_tail(&sub_new->lst_self, &newnode->lst_sub) ;
			sub_new->parent = newnode;
			newnode->sub_num++;
		}
	}
	return newnode ;
}

int ndxml_insert(ndxml *parent, ndxml*child)
{
	list_add_tail(&child->lst_self, &parent->lst_sub);
	child->parent = parent;
	parent->sub_num++;
	return 0;
}

int ndxml_get_index(ndxml *parent, ndxml*child)
{
	int i = 0;
	struct list_head *pos;
	list_for_each(pos, &parent->lst_sub) {
		if (pos == &(child->lst_self)) {
			return i;
		}
		++i;
	}
	return -1;
}
int ndxml_insert_ex(ndxml *parent, ndxml*child, int index)
{
	int i = 0;
	if (index != -1)	{
		struct list_head *pos;
		list_for_each(pos, &parent->lst_sub) {
			if (index <= i)	{
				list_add(&child->lst_self, pos);
				parent->sub_num++;
				return 0;
			}
			++i;
		}
	}

	list_add_tail(&child->lst_self, &parent->lst_sub);
	child->parent = parent;
	parent->sub_num++;
	return 0;
	
}
int ndxml_merge(ndxml_root *host, ndxml_root *merged) 
{
	if(merged->sub_num > 0) {
		list_join(&merged->lst_sub, &host->lst_sub);
		host->sub_num += merged->sub_num ;

		ndxml_initroot(merged);
	}
	return 0 ;
}

ndxml *ndxml_getnode(ndxml_root *xmlroot,const  char *name)
{
	ndxml *sub_xml; 
	struct list_head *pos = xmlroot->lst_sub.next;
	while (pos != &xmlroot->lst_sub) {
		sub_xml = list_entry(pos,struct tagxml, lst_self) ;
		pos = pos->next ;
		if (0==ndstricmp((char*)name,sub_xml->name)){
			return sub_xml ;
		}
	}
	return NULL ;
}
ndxml *ndxml_getnodei(ndxml_root *xmlroot, int index) 
{
	int i = 0 ;
	ndxml *sub_xml; 
	struct list_head *pos = xmlroot->lst_sub.next;

	while (pos != &xmlroot->lst_sub) {
		sub_xml = list_entry(pos,struct tagxml, lst_self) ;
		pos = pos->next ;
		if(i==index){
			return sub_xml ;
		}
		++i ;
	}
	return NULL ;
}

ndxml *ndxml_addnode(ndxml_root *xmlroot, const char *name,const char *value)
{
	ndxml *xmlnode = _create_xmlnode(name, value) ;
	if(xmlnode){
		list_add_tail(&xmlnode->lst_self, &xmlroot->lst_sub);
		xmlnode->parent = xmlroot;

		xmlroot->sub_num++ ;
	}
	return xmlnode ;
}


ndxml* ndxml_big_brother(ndxml *node)
{
	struct list_head *prev = node->lst_self.prev;
	ndxml *parent = node->parent;
	if (!parent){
		return NULL;
	}
	if (prev != &parent->lst_sub) {
		return list_entry(prev, struct tagxml, lst_self);
	}
	return NULL;
}
ndxml* ndxml_little_brother(ndxml *node)
{
	struct list_head *next = node->lst_self.next;
	ndxml *parent = node->parent;
	if (!parent){
		return NULL;
	}
	if (next != &parent->lst_sub) {
		return list_entry(next, struct tagxml, lst_self);
	}
	return NULL;
}


ndxml *ndxml_from_text(const char *text)
{
	char *pend, *perraddr;
	return parse_xmlbuf(text, strlen(text), &pend, &perraddr);
}

int ndxml_delnode(ndxml_root *xmlroot,const  char *name)
{
	ndxml *node = ndxml_getnode(xmlroot,name) ;
	if(!node)
		return -1 ;
	list_del(&node->lst_self) ;
	xmlroot->sub_num-- ;
	dealloc_xml(node);
	return 0 ;
}
int ndxml_delnodei(ndxml_root *xmlroot, int index) 
{
	ndxml *node = ndxml_getnodei(xmlroot,index) ;
	if(!node)
		return -1 ;
	list_del(&node->lst_self) ;
	xmlroot->sub_num-- ;
	dealloc_xml(node);
	return 0 ;
}


int ndxml_remove(ndxml *node, ndxml *xmlParent)
{
	int ret = -1;
	struct list_head *pos;

	if (xmlParent == NULL)	{
		xmlParent = node->parent;
		if (!xmlParent)	{
			list_del_init(&node->lst_self);
			return 0;
		}
	}
	pos = xmlParent->lst_sub.next;

	if (!node )
		return -1;

	while (pos != &xmlParent->lst_sub) {
		ndxml *sub_xml = list_entry(pos, struct tagxml, lst_self);
		pos = pos->next;
		if (sub_xml == node){
			ret = 0;
			break;
		}

	}
	if (ret == -1)	{
		return ret;
	}
	list_del_init(&node->lst_self);
	node->parent = NULL;
	xmlParent->sub_num--;
	return 0;
}
int ndxml_delxml(ndxml *node, ndxml *xmlParent)
{
	if (!xmlParent) {
		xmlParent = node->parent;
	}

	if (xmlParent)	{
		if (0 == ndxml_remove(node, xmlParent)) {
			dealloc_xml(node);
			return 0;
		}

	}
	else {
		list_del(&node->lst_self);
		dealloc_xml(node);
	}
	return -1;
}

//����һ���ӽڵ�
ndxml *ndxml_refsub(ndxml *root, const char *name) 
{
	ndxml *sub_xml; 
	struct list_head *pos = root->lst_sub.next ;
	while (pos!=&root->lst_sub) {
		sub_xml = list_entry(pos,struct tagxml, lst_self) ;
		pos = pos->next ;
		if (0==ndstricmp((char*)name,sub_xml->name)){
			return sub_xml ;
		}
	}
	return NULL ;
}

//����һ���ӽڵ�
ndxml *ndxml_refsubi(ndxml *root, int index) 
{
	int i = 0 ;
	ndxml *sub_xml; 
	struct list_head *pos = root->lst_sub.next ;

	while (pos!=&root->lst_sub) {
		sub_xml = list_entry(pos,struct tagxml, lst_self) ;
		pos = pos->next ;
		//if (0==ndstricmp(name,sub_xml->name)){
		if(i==index){
			return sub_xml ;
		}
		++i ;
	}
	return NULL ;
}

//�õ�xml��ֵ
const char *ndxml_getval(ndxml *node)
{
	if(node->value && node->value[0])
		return node->value ;
	else
		return NULL ;
}

char *ndxml_getval_buf(ndxml *node, char *buf, size_t size)
{
	if(node->value && node->value[0])
		return strncpy(buf,node->value, size) ;
	else
		return NULL ;
}

int ndxml_getval_int(ndxml *node)
{
	if(node->value && node->value[0])
		return ndstr_atoi_hex(node->value );
	else
		return 0 ;
}

float ndxml_getval_float(ndxml *node)
{
	if(node->value && node->value[0])
		return (float)atof(node->value );
	else
		return 0 ;
}
//�õ�����ֵ
struct ndxml_attr *ndxml_getattrib(ndxml *node ,const  char *name)
{
	struct ndxml_attr *attr ;
	struct list_head *pos = node->lst_attr.next;
	while (pos!= &node->lst_attr){
		attr = list_entry(pos,struct ndxml_attr, lst) ;
		pos=pos->next ;
		if(0==ndstricmp(name, (char*)(attr+1))) {
			return attr ;
		}
	}
	return NULL ;
}

struct ndxml_attr  *ndxml_getattribi(ndxml *node, int index)
{
	int i=0 ;
	struct ndxml_attr *attr ;
	struct list_head *pos = node->lst_attr.next;
	while (pos!= &node->lst_attr){
		attr = list_entry(pos,struct ndxml_attr, lst) ;
		pos=pos->next ;
		//if(0==ndstricmp(name,attr->name)) {
		if(i==index){
			return attr ;
		}
		++i ;
	}
	return NULL ;

}

//////////////////////////////////////////////////////////////////////////
//��xml����һ������,
struct ndxml_attr  *ndxml_addattrib(ndxml *parent, const char *name, const char *value)
{
	struct ndxml_attr *attrib_node;
	if(!name || !value)
		return NULL ;
	attrib_node = alloc_attrib_node(name, value) ;
	if(attrib_node) {
		list_add_tail(&attrib_node->lst, &parent->lst_attr) ;
		(parent->attr_num)++ ;
	}
	return attrib_node ;

}


int ndxml_setattrval(ndxml *parent, const char *name, const char *value)
{
	int len ;
	struct ndxml_attr  * attr ;

	if( !name) {
		return -1 ;
	}
	if(!value || (len  = (int)strlen(value)) == 0) {
		return ndxml_delattrib(parent, name) ;
	}

	attr =ndxml_getattrib(parent,  name);

	if(!attr) {
		struct ndxml_attr  * attr =ndxml_addattrib(parent,name, value)  ;
		if(attr) {
			return 0 ;
		}
		return -1 ;
	}
	
	++len ;
	if(len > attr->value_size) {
		struct ndxml_attr *attrib_node;
		attrib_node = alloc_attrib_node((char*)(attr + 1), value) ;
		if(!attrib_node) {
			return -1 ;
		}
		list_add(&attrib_node->lst, &attr->lst);

		list_del(&attr->lst) ;
		dealloc_attrib_node(attr);
	}
	else {
		strcpy((char*)(attr + 1)+attr->name_size, value) ;
	}
	return 0;
}
int ndxml_setattrvali(ndxml *parent, int index, const char *value)
{
	
	int len ;
	struct ndxml_attr  * attr ;

	if( index <0 || index>= parent->attr_num) {
		return -1 ;
	}
	if(!value || (len  =(int) strlen(value)) == 0) {
		return ndxml_delattribi(parent, index) ;
	}

	attr =ndxml_getattribi(parent,  index);

	if(!attr) {
		return -1 ;
	}
	
	++len;
	if(len > attr->value_size) {
		struct ndxml_attr *attrib_node;
		attrib_node = alloc_attrib_node((char*)(attr + 1), value) ;
		if(!attrib_node) {
			return -1 ;
		}
		list_add(&attrib_node->lst, &attr->lst);

		list_del(&attr->lst) ;
		dealloc_attrib_node(attr);
		return 0 ;
	}
	else {
		strcpy((char*)(attr + 1)+attr->name_size, value) ;
	}
	return 0;
}
//��xml����һ���ӽڵ���Ҫ�����½ڵ�����ֺ�ֵ,�����½ڵ��ַ
ndxml *ndxml_addsubnode(ndxml *parent, const char *name, const char *value)
{
	ndxml *xmlnode = _create_xmlnode(name, value) ;
	if(xmlnode){
		list_add_tail(&xmlnode->lst_self, &parent->lst_sub);
		xmlnode->parent = parent;
		parent->sub_num++ ;
	}
	return xmlnode ;
}


int ndxml_setval_int(ndxml *node, int val)
{
	char buf[20];
	snprintf(buf, sizeof(buf), "%d", val);
	return ndxml_setval(node, buf);
}

int ndxml_setval_float(ndxml *node, float val)
{
	char buf[20];
	snprintf(buf,sizeof(buf), "%f", val);
	return ndxml_setval(node, buf);

}
//����XML��ֵ
int ndxml_setval(ndxml *node , const char *val)
{
	int len;
	if(!val)
		return -1 ;

	len = (int) strlen(val) ;
	if(len==0)
		return -1 ;

	++len;
	if(node->value){
		if(len>node->val_size) {
			char *tmp ;
			
			len += 4 ; len &= ~3;
			tmp = malloc(len);
			if(!tmp)
				return -1 ;
			free(node->value) ;
			node->value = tmp ;
		}
	}
	else {
		len += 4 ; len &= ~3;
		node->value = malloc(len);
		if(!node->value)
			return -1 ;
	}
	strcpy(node->value,val) ;
	return 0 ;
}
//ɾ��һ�����Խڵ�
int ndxml_delattrib(ndxml *parent, const char *name)
{
	struct ndxml_attr *attr= ndxml_getattrib(parent , name);
	if(!attr)
		return -1 ;
	list_del(&attr->lst) ;
	parent->attr_num-- ;
	dealloc_attrib_node(attr);
	return 0 ;

}
int ndxml_delattribi(ndxml *parent, int index) 
{
	struct ndxml_attr *attr= ndxml_getattribi(parent , index);
	if(!attr)
		return -1 ;
	list_del(&attr->lst) ;
	parent->attr_num-- ;
	dealloc_attrib_node(attr);
	return 0 ;
}
//ɾ��һ���ӽڵ�
int ndxml_delsubnode(ndxml *parent, const char *name)
{
	ndxml *node = ndxml_refsub(parent,name) ;
	if(!node)
		return -1 ;
	list_del(&node->lst_self) ;
	parent->sub_num-- ;
	dealloc_xml(node);
	return 0 ;
}

int ndxml_delsubnodei(ndxml *parent, int index) 
{
	ndxml *node = ndxml_refsubi(parent,index) ;
	if(!node)
		return -1 ;
	list_del(&node->lst_self) ;
	parent->sub_num-- ;
	dealloc_xml(node);
	return 0 ;
}
//////////////////////////////////////////////////////////////////////////
//ȥ��ע��
static const char* parse_marked(const char *xmlbuf, int size, const char **error_addr) 
{
	const char *pstart = xmlbuf ;
	const char *paddr;
	
	while(pstart< xmlbuf+size) {
		*error_addr = pstart ;
		//paddr = strchr(pstart, '<') ;
		paddr =  ndstr_first_valid(pstart) ;
		if (!paddr || *paddr != '<') {
			//*error_addr = pstart ;
			return NULL ;
		}

		if(!paddr || paddr >= xmlbuf+size) {
			*error_addr = 0 ;
			return NULL ;
		}
		if('?'==paddr[1]) {
			paddr = strstr(paddr+2, "?>") ;
			if(!paddr || paddr >= xmlbuf+size) {
				return NULL ;
			}
			paddr += 2 ;
		}
		else if(paddr[1]=='!' ) {
			if(paddr[2]=='-' && paddr[3]=='-') {
				paddr = strstr(paddr+4, "-->") ;
				if(!paddr || paddr >= xmlbuf+size) {
					return NULL ;
				}
				paddr += 3 ;
			}
			else {
				return NULL ;
			}
		}
		else {
			*error_addr = NULL ;
			return paddr ;
		}
		
		pstart = paddr ;
	}
	return NULL ;
}
//���ڴ���н�����һ��XML�ڵ�
ndxml *parse_xmlbuf(const char *xmlbuf, int size, const char **parse_end, const char **error_addr)
{
	ndxml *xmlnode =NULL ;
	const char *paddr ;//, *error_addr =NULL;
	char buf[1024] ;
	
	paddr = parse_marked(xmlbuf, size, error_addr)  ;
	if(!paddr){
		*parse_end = NULL ;
		if (!*error_addr) {
			*error_addr = xmlbuf ;
		}
		
		return NULL ;
	}
	//if(XML_T_END==*((short int*)paddr)) {
	if (_is_mark_start(paddr)) {
		*parse_end = paddr ;
		*error_addr = NULL ;
		return NULL ;
		//goto READ_END ;
	}
	++paddr ;

	paddr = ndstr_first_valid(paddr) ;
	if(!paddr) {
		*parse_end = NULL ;
		return NULL ;
	}
	//check valid 
	if(*paddr=='>' || *paddr=='<' || *paddr=='\"' || *paddr=='/') {
		*parse_end = NULL ;
		*error_addr = paddr ;
		return NULL ;
	}

	xmlnode = alloc_xml() ;
	if(!xmlnode) {
		*parse_end = NULL ;
		return NULL ;
	}
	paddr = ndstr_parse_word(paddr, buf) ;		//xml node name
	strncpy(xmlnode->name, buf, MAX_XMLNAME_SIZE) ;

	paddr = ndstr_first_valid(paddr) ;
	//read attribe
	while(*paddr) {
		struct ndxml_attr *attrib_node ;
		char attr_name[MAX_XMLNAME_SIZE] ;
		//if(*((short int*)paddr)==XML_H_END ) {
		if (_is_mark_end(paddr)) {
			//���xml�ڵ������,Ӧ�÷�����
			*parse_end = paddr + 2 ;
			return xmlnode ;
		}
		else if('>'==*paddr) {
			//�ڵ�ͷ����,�˳����Զ�ȡѭ��
			paddr++ ;
			break ;
		}
		//read attrib name 
		paddr = ndstr_parse_word(paddr, attr_name) ;
		paddr = strchr(paddr,_ND_QUOT) ;
		if(!paddr) {
			dealloc_xml(xmlnode) ;
			*parse_end = NULL ;
			return NULL ;
		}
		++paddr ;
		//read attrib VALUE
		paddr = ndstr_str_end(paddr,buf, _ND_QUOT) ;
		++paddr ;
		
		attrib_node = alloc_attrib_node(attr_name, buf) ;
		if(attrib_node) {
			list_add_tail(&attrib_node->lst, &xmlnode->lst_attr) ;
			(xmlnode->attr_num)++ ;
		}
		paddr = ndstr_first_valid(paddr) ;
	}
	
	//read value and sub-xmlnode
	paddr = ndstr_first_valid(paddr) ;
	//if(XML_T_END==*((short*)paddr)) {
	if (_is_mark_start(paddr)) {
		//xml node end </
		goto READ_END ;
	}
	else if('<'==*paddr) {
		//ʹ�õݹ�ȥ������xml node
		while (*paddr)	{
			char *parsed ;
			int  left_size = (int) (size -( paddr - xmlbuf) );
			ndxml *new_xml = parse_xmlbuf(paddr, left_size, &parsed, error_addr) ;
			if(new_xml) {
				list_add_tail(&new_xml->lst_self, &xmlnode->lst_sub);
				new_xml->parent = xmlnode;
				xmlnode->sub_num++ ;
			}
			else if(*error_addr && NULL==parsed) {
				if (!error_addr) {
					*error_addr = (char *) paddr;
				}
				//parse error 
				dealloc_xml(xmlnode) ;
				*parse_end = NULL ;
				return NULL ;
			}
			paddr = ndstr_first_valid(parsed) ;
			//if(XML_T_END==*((short*)paddr)) {
			if (_is_mark_start(paddr)) {
				goto READ_END ;
			}

		}
	}
	else {
		//read xml value 
		size_t val_size ;
		const char *tmp  = ndstr_reverse_chr(paddr, '>', xmlbuf) ;
		if(!tmp){
			tmp = paddr ;
		}
		else {
			++tmp ;
		}
		paddr = ndstr_str_end(tmp,buf, '<') ;		//��ȡxmlֵ,һֱ��"<"����
		val_size = paddr - tmp ;
		
		//store value 
		val_size += 4 ;val_size &= ~3 ;

		xmlnode->value = malloc(val_size) ;
		if(xmlnode->value) {
			strcpy(xmlnode->value, buf) ;
			xmlnode->val_size = val_size ;
		}
		else {
			dealloc_xml(xmlnode) ;
			*parse_end = NULL ;
			return NULL ;
		}
	}
	
	//read end
READ_END :
	{
		char end_name[MAX_XMLNAME_SIZE] ;
		
		//if(XML_T_END != *((short*)paddr)) {
		if (!_is_mark_start(paddr)) {
			//��������,û������������־
			if(xmlnode)
				dealloc_xml(xmlnode) ;
			*parse_end = NULL ;
			*error_addr = paddr ;
			return NULL ;
		}
		
		//check xml end
		paddr += 2 ; //skip </
		paddr = ndstr_first_valid(paddr) ;
		
		paddr = ndstr_parse_word(paddr,end_name) ;
		if(ndstricmp(xmlnode->name, end_name)) {
			//��������,û������������־,�����ǽ�����־д����
			dealloc_xml(xmlnode) ;
			*parse_end = NULL ;
			*error_addr = paddr ;
			return NULL ;
		}
		paddr = strchr(paddr,'>') ;
		paddr++ ;
		*parse_end = paddr ;
	} 
	return xmlnode ;
}

//����һ���ڵ��ڴ�
ndxml *alloc_xml()
{
	ndxml *node = malloc(sizeof(ndxml)) ;
	if(node) {
		init_xml_node(node) ;
	}
	return node ;
}

void _release_xml(ndxml *node)
{
	struct list_head *pos ;	
	//dealloc value 
	if(node->value) {
		free(node->value) ;
		node->value = 0 ;
	}

	//dealloc attribute
	pos = node->lst_attr.next ;
	while(pos != &node->lst_attr) {
		struct ndxml_attr *attrnode ;
		attrnode = list_entry(pos, struct ndxml_attr, lst) ;
		pos = pos->next ;
		dealloc_attrib_node(attrnode) ;
	}

	//dealloc sub-xmlnode 
	pos = node->lst_sub.next ;
	while(pos != &node->lst_sub) {
		struct tagxml  *_xml ;
		_xml = list_entry(pos, struct tagxml, lst_self) ;
		pos = pos->next ;
		list_del(&_xml->lst_self) ;
		dealloc_xml(_xml) ;
	}

}

//�ͷ�һ��XML�ڵ��������Դ
void  dealloc_xml(ndxml *node)
{
	_release_xml(node);
	free(node);
}


//����һ�����Խڵ���ڴ�
struct ndxml_attr *alloc_attrib_node(const char *name, const char *value)
{
	char *p ;
	struct ndxml_attr *pnode ;
	int len = (int) strlen(name) ;
    int val_reallen =(int) strlen(value) ;
	int val_len = val_reallen ;
	//if(!len || !val_len)  {
	//	return NULL;
	//}
	len += 4 ;
	len &= ~3 ;
    
    val_len += 4 ;
    val_len &= ~3 ;
    
	pnode = malloc(sizeof(struct ndxml_attr) + len + val_len) ;
	if(!pnode) {
		return NULL ;
	}
	init_xml_attr(pnode) ;

	pnode->name_size = len ;
	pnode->value_size = val_len ;
	//pnode->name = (char*) (pnode + 1) ;
	//pnode->value = pnode->name + len ;
	p = (char*) (pnode + 1) ;
	strcpy(p , name) ;

	p += len ;
    if (val_reallen > 0) {
        strcpy(p, value) ;
    }
    else {
        *p = 0;
    }

	return pnode ;
}

//�ͷ�һ�����Խڵ��ڴ���Դ
void dealloc_attrib_node(struct ndxml_attr *pnode)
{
	list_del(&pnode->lst);
	free(pnode) ;
}

//create a xml node input name and value 
ndxml *_create_xmlnode(const char *name, const char *value)
{
	ndxml *xmlnode ;
	
	if(!name)
		return NULL ;

	xmlnode = alloc_xml() ;
	if(!xmlnode) {
		return NULL ;
	}
	strncpy(xmlnode->name,	name,MAX_XMLNAME_SIZE) ;
	if(value) {
		int len = (int) strlen(value) ;
		if(len> 0) {
			len += 4 ;
			len &= ~3 ;
			xmlnode->value = malloc(len) ;
			if(xmlnode->value){
				strcpy(xmlnode->value, value) ;	
				xmlnode->val_size = len ;
			}else {
				free(xmlnode) ;
				return NULL ;
			}
		}
	}
	
	return xmlnode ;
}

//#define INDENT(fp, n) while(n--) {	\
//	fprintf(fp,"\t"); } 

static __INLINE__ void indent(FILE *fp, int deep)
{
	while(deep-- > 0) {
		fprintf(fp,"\t"); 
	}
}
//��xmlд���ļ���
//@deep �ڵ�����
int xml_write(ndxml *xmlnode, FILE *fp , int deep)
{
	struct list_head *pos ;

	indent(fp,deep) ;
	fprintf(fp, "<%s", xmlnode->name) ;
	
	//save attribute to file 
	pos = xmlnode->lst_attr.next ;
	while (pos != &xmlnode->lst_attr){
		struct ndxml_attr *xml_attr = list_entry(pos, struct ndxml_attr, lst) ;
        char *attr_val1 = (char*)(xml_attr + 1) +xml_attr->name_size ;
		pos = pos->next ;
        if( attr_val1[0] ) {
			fprintf(fp, " %s=\"%s\"", (char*)(xml_attr + 1),attr_val1) ;
        }
        else {
            fprintf(fp, " %s=\"\"", (char*)(xml_attr + 1)) ;
        }
	}
	
	//save value of sub-xmlnode
	if(xmlnode->value && xmlnode->value[0]) {
		fprintf(fp, ">%s</%s>\n", xmlnode->value, xmlnode->name) ;
	}
	else if (xmlnode->sub_num>0){
		fprintf(fp, ">\n") ;

		pos = xmlnode->lst_sub.next ;
		while (pos != &xmlnode->lst_sub){
			ndxml *subxml = list_entry(pos, struct tagxml, lst_self) ;
			pos = pos->next ;
			xml_write(subxml, fp , deep+1);
		}
		indent(fp,deep) ;
		fprintf(fp, "</%s>\n", xmlnode->name) ;
	}
	else {
		fprintf(fp, "/>\n") ;
	}
	return 0 ;
}

void _errlog (const char *errdesc)
{
	fprintf(stderr,"%s", errdesc) ;	
}

int ndxml_output(ndxml *node, FILE *pf)
{
	return xml_write(node, pf, 0);
}

