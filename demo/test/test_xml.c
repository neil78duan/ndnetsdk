/* file test_xml.c
 * test xml
 * neil duan
 * 2008-8-24
 */


#include "nd_common/nd_common.h"
#include "nd_common/nd_xml.h"

#define T_ERROR(msg) do {				\
	printf("test error [%s]\n", msg) ;	\
	return -1 ;							\
} while(0)

static __INLINE__ void _indent(int deep)
{
	while(deep-- > 0) {
		printf("\t"); 
	}
}

int print_xml(ndxml *xmlnode, int deep)
{
	ndxml *sub ;
//	struct ndxml_attr *attr ;
	int num ,i;
	char *val;
	
	_indent(deep) ;
	printf("<%s> ", ndxml_getname(xmlnode)) ;

	num =ndxml_getattr_num( xmlnode ) ;
	for (i=0; i<num; i++){
		printf(",%s=%s", ndxml_getattr_name(xmlnode,i), ndxml_getattr_vali(xmlnode,i)) ;
	}
	printf("\n") ;
	//printf sub node 
	
	num =ndxml_getsub_num( xmlnode ) ;
	for (i=0; i<num; i++){
		sub = ndxml_refsubi(xmlnode,i) ;
		if(sub)
			print_xml(sub, deep+1);
	}

	val = ndxml_getval(xmlnode) ;
	if(val) {
		_indent(deep) ;
		printf("value=%s\n", val) ;
	}
	
	_indent(deep) ;
	printf("</%s>\n", ndxml_getname(xmlnode)) ;
	return 0 ;
}

int test1()
{
	int i, num ;
	ndxml_root xml;
	int ret = ndxml_load("test_xmlsave.xml", &xml) ;
	if(0!=ret) {
		T_ERROR("load xml from file") ;
	}

	num = ndxml_num(&xml) ;
	for (i=0; i<num; i++){
		ndxml *node = ndxml_getnodei(&xml,i) ;
		if(node)
			print_xml(node, 0) ;

	}

	ndxml_save(&xml, "test_xmlback.xml") ;
	ndxml_destroy(&xml);
	return 0;

}


int test2() 
{
	ndxml *xmlnode, *sub ;
	ndxml_root xml;
	ndxml_initroot(&xml) ;

	xmlnode = ndxml_addnode(&xml, "header", NULL) ;
	if(!xmlnode)
		T_ERROR("add xml node") ; ;
	
	if(!ndxml_addattrib(xmlnode, "name", "根节点1") ) {
		T_ERROR("add xml attribute") ; ;
	}
	
	if(!ndxml_addattrib(xmlnode, "number", "2") ) {
		T_ERROR("add xml attribute") ; ;
	}

	
	if(0!=ndxml_setattrval(xmlnode, "number", "1234567890") ) {
		T_ERROR("set xml attribute value") ; ;
	}

	sub = ndxml_addsubnode(xmlnode, "node1", "hello world!") ;
	if(!sub) {
		T_ERROR("add xml node") ; ;
	}

	ndxml_setval(sub,"你好我是段段!") ;

	
	sub = ndxml_addsubnode(xmlnode, "node2", NULL) ;
	if(!sub) {
		T_ERROR("add xml node") ; ;
	}
	if(!ndxml_addsubnode(sub,"sub1", "sub1 value")){
		T_ERROR("add xml node") ; ;
	}

//	if(0!=ndxml_delsubnode(xmlnode,"node2")){
//		T_ERROR("add xml node") ;
//	}

	ndxml_save(&xml, "test_xmlsave.xml") ;
	ndxml_destroy(&xml);
	return 0;
}


int xml_test()
{
	if(test2() )
		return -1;
	return test1();

}