/* file nd_xml.c
 * implement xml parser of nd engine 
 * version 1.0 
 * 2008-8-24 
 * all right reserved by duan xiuyun 
 */

#include "nd_common/nd_common.h"

#define MAX_FILE_SIZE 4*4096
#define XML_H_END   0x3e2f			// /> xml node end mark
#define XML_T_END   0x2f3c			// </ xml end node mark,高字节在高地址 /地址高于 >

ndxml *parse_xmlbuf(char *xmlbuf, int size,char **parse_end, char **error_addr) ;
ndxml *alloc_xml();
void  dealloc_xml(ndxml *node );
struct ndxml_attr *alloc_attrib_node(const char *name, const char *value);
void dealloc_attrib_node(struct ndxml_attr *pnode);
int xml_write(ndxml *xmlnode, FILE *fp , int deep) ;
ndxml *_create_xmlnode(const char *name, const char *value)  ;

static void _errlog (const char *errdesc) ;
static void show_xmlerror(const char *file, const char *error_addr, const char *xmlbuf, size_t size) ;

static xml_errlog __xml_logfunc = _errlog ;

int ndxml_load(const char *file,ndxml_root *xmlroot)
{
	int data_len,buf_size ;
	FILE *fp;
	char *text_buf= NULL , *parse_addr;

	fp = fopen(file, "r+b") ;
	if(!fp) {
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	buf_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if(buf_size==0) {
		fclose(fp) ;
		return -1 ;
	}
	buf_size += 1 ;
	text_buf =(char*) malloc(buf_size) ;
	
	if(!text_buf){
		fclose(fp) ;
		return -1 ;
	}
	data_len = (int) fread(text_buf,1, buf_size, fp) ;
	if(data_len==0 || data_len>= buf_size) {
		fclose(fp) ;
		free(text_buf) ;
		return -1 ;

	}
	text_buf[data_len] = 0 ;
	fclose(fp) ;
	
	//ndxml_root *xmlroot;
	ndxml_initroot(xmlroot) ;
	parse_addr = text_buf ;
	do {
		char *error_addr = 0;
		ndxml *xmlnode = parse_xmlbuf(parse_addr, data_len, &parse_addr, &error_addr);
		if(xmlnode) {
			list_add_tail(&xmlnode->lst_self, &xmlroot->lst_xml);
			xmlroot->num++ ;
		}
		else if(error_addr) {
			ndxml_destroy(xmlroot) ;
			show_xmlerror(file, error_addr, text_buf, (size_t) data_len) ;
			return -1;
		}
	} while(parse_addr && *parse_addr);

	free(text_buf) ;
	return 0 ;

}

void ndxml_destroy(ndxml_root *xmlroot)
{
	ndxml *sub_xml; 
	struct list_head *pos = xmlroot->lst_xml.next ;
	while (pos!=&xmlroot->lst_xml) {
		sub_xml = list_entry(pos,struct tagxml, lst_self) ;
		pos = pos->next ;
		dealloc_xml(sub_xml) ;
		
	}
	ndxml_initroot(xmlroot) ;
}

//显示xml错误
void show_xmlerror(const char *file, const char *error_addr, const char *xmlbuf, size_t size)
{
	int line = 0 ;
	char *pline, *pnext ;
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
//			size_t l_size ;
//			char line_text[512] ;
			//found error code 
			pline = ndstr_first_valid(pline) ;

//			l_size = pnext - pline ;
//			l_size = min(l_size, 512) ;
			//strncpy(line_text, pline, l_size) ;
//			ndstr_nstr_end(pline, line_text, '\n', 512) ;
			//snprintf(errbuf, 1024, "error %s %d lines [%s]", file, line, line_text) ;
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
    struct list_head *pos = xmlroot->lst_xml.next ;
    
    fp = fopen(file, "w") ;
    if(!fp) {
        return -1;
    }
    if (header) {
        fprintf(fp, "<? %s ?>\n", header) ;
    }
    
    while (pos!=&xmlroot->lst_xml) {
        sub_xml = list_entry(pos,struct tagxml, lst_self) ;
        pos = pos->next ;
        xml_write(sub_xml,fp, 0) ;
        
    }
    
    fclose(fp) ;
    
    return 0;
    
}

int ndxml_save(ndxml_root *xmlroot, const char *file)
{
    return ndxml_save_ex(xmlroot, file, NULL) ;
}


int ndxml_merge(ndxml_root *host, ndxml_root *merged) 
{
	if(merged->num > 0) {
		list_join(&merged->lst_xml, &host->lst_xml) ;
		host->num += merged->num ;

		ndxml_initroot(merged);
	}
	return 0 ;
}

ndxml *ndxml_getnode(ndxml_root *xmlroot,const  char *name)
{
	ndxml *sub_xml; 
	struct list_head *pos = xmlroot->lst_xml.next ;
	while (pos!=&xmlroot->lst_xml) {
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
	struct list_head *pos = xmlroot->lst_xml.next ;

	while (pos!=&xmlroot->lst_xml) {
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
		list_add_tail(&xmlnode->lst_self, &xmlroot->lst_xml);
		xmlroot->num++ ;
	}
	return xmlnode ;
}

int ndxml_delnode(ndxml_root *xmlroot,const  char *name)
{
	ndxml *node = ndxml_getnode(xmlroot,name) ;
	if(!node)
		return -1 ;
	list_del(&node->lst_self) ;
	xmlroot->num-- ;
	dealloc_xml(node);
	return 0 ;
}
int ndxml_delnodei(ndxml_root *xmlroot, int index) 
{
	ndxml *node = ndxml_getnodei(xmlroot,index) ;
	if(!node)
		return -1 ;
	list_del(&node->lst_self) ;
	xmlroot->num-- ;
	dealloc_xml(node);
	return 0 ;
}
//引用一个子节点
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

//引用一个子节点
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

//得到xml的值
char *ndxml_getval(ndxml *node)
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
		return atoi(node->value );
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
//得到属性值
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
//给xml增加一个属性,
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
//给xml增加一个子节点需要输入新节点的名字和值,返回新节点地址
ndxml *ndxml_addsubnode(ndxml *parent, const char *name, const char *value)
{
	ndxml *xmlnode = _create_xmlnode(name, value) ;
	if(xmlnode){
		list_add_tail(&xmlnode->lst_self, &parent->lst_sub);
		parent->sub_num++ ;
	}
	return xmlnode ;
}

//设置XML的值
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
//删除一个属性节点
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
//删除一个子节点
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
//去掉注释
static char* parse_marked(char *xmlbuf, int size, char **error_addr) 
{
	char *pstart = xmlbuf ;
	char *paddr ;
	
	while(pstart< xmlbuf+size) {
		*error_addr = pstart ;
		paddr = strchr(pstart, '<') ;

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
//从内存块中解析出一个XML节点
ndxml *parse_xmlbuf(char *xmlbuf, int size, char **parse_end, char **error_addr)
{
	ndxml *xmlnode =NULL ;
	char *paddr ;//, *error_addr =NULL;
	char buf[1024] ;
	
	paddr = parse_marked(xmlbuf, size, error_addr)  ;
	if(!paddr){
		*parse_end = NULL ;
		return NULL ;
	}
	if(XML_T_END==*((short int*)paddr)) {
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
		if(*((short int*)paddr)==XML_H_END ) {
			//这个xml节点结束了,应该返回了
			*parse_end = paddr + 2 ;
			return xmlnode ;
		}
		else if('>'==*paddr) {
			//节点头结束,退出属性读取循环
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
	if(XML_T_END==*((short*)paddr)) {
		//xml node end </
		goto READ_END ;
	}
	else if('<'==*paddr) {
		//使用递归去解析子xml node
		while (*paddr)	{
			char *parsed ;
			int  left_size = (int) (size -( paddr - xmlbuf) );
			ndxml *new_xml = parse_xmlbuf(paddr, left_size, &parsed, error_addr) ;
			if(new_xml) {
				list_add_tail(&new_xml->lst_self, &xmlnode->lst_sub);
				xmlnode->sub_num++ ;
			}
			else if(*error_addr || NULL==parsed) {
				//parse error 
				dealloc_xml(xmlnode) ;
				*parse_end = NULL ;
				return NULL ;
			}
			paddr = ndstr_first_valid(parsed) ;
			if(XML_T_END==*((short*)paddr)) {
				goto READ_END ;
			}

		}
	}
	else {
		//read xml value 
		size_t val_size ;
		char *tmp  ;

		tmp = ndstr_reverse_chr(paddr, '>', xmlbuf) ;
		if(!tmp){
			tmp = paddr ;
		}
		else {
			++tmp ;
		}
		paddr = ndstr_str_end(tmp,buf, '<') ;		//读取xml值,一直到"<"结束
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
		
		if(XML_T_END != *((short*)paddr)) {
			//解析出错,没有遇到结束标志
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
			//解析出错,没有遇到结束标志,或者是结束标志写错了
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

//申请一个节点内存
ndxml *alloc_xml()
{
	ndxml *node = malloc(sizeof(ndxml)) ;
	if(node) {
		init_xml_node(node) ;
	}
	return node ;
}

//释放一个XML节点的所以资源
void  dealloc_xml(ndxml *node )
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

	free(node) ;
}

//申请一个属性节点的内存
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

//释放一个属性节点内存资源
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
//把xml写到文件中
//@deep 节点的深度
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
