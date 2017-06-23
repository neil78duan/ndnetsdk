//
//  ndxml_set.cpp
//  cliDemo
//
//  Created by duanxiuyun on 15-3-5.
//  Copyright (c) 2015 duanxiuyun. All rights reserved.
//


#include "nd_common/nd_common.h"
#include "nd_crypt/nd_crypt.h"

int set_xml_value(char *file, char *node_name_list, char *attr_name, char*attr_val, char *node_val)
{
    int ret;
    ndxml_root xmlfile;
    
    ndxml *xnode = 0 ;
    
    char *p = node_name_list;
    char name[128] ;
    
    ret = ndxml_load(file, &xmlfile) ;
    if(0!=ret) {
        fprintf(stderr,"load xml from %s\n", file) ;
        return -1;
    }
    
    p = (char*)ndstr_nstr_end(p, name, '.', sizeof(name)) ;
    
    if (IS_NUMERALS(name[0])) {
		xnode = ndxml_getnodei(&xmlfile, ndstr_atoi_hex(name));
    }
    else {
        xnode = ndxml_getnode(&xmlfile, name) ;
    }
    if (!xnode) {
        fprintf(stderr,"read xml-node %s error \n", name) ;
    }
    
    bool success = true ;
    while (p && *p) {
        
        if (*p == '.') {
            ++p ;
            if (!*p) {
                break ;
            }
        }
        
        p = (char*) ndstr_nstr_end(p, name, '.', sizeof(name)) ;
        
        if (IS_NUMERALS(name[0])) {
			xnode = ndxml_refsubi(xnode, ndstr_atoi_hex(name));
        }
        else {
            xnode = ndxml_refsub(xnode, name) ;
        }
        if (!xnode) {
            fprintf(stderr,"read xml-node %s error \n", name) ;
            success = false ;
            break ;
        }
        
        
    }
    if (success) {
        success = false;
        if (attr_name && attr_name[0] && attr_val && attr_val[0]) {
            ndxml_setattrval(xnode,attr_name, attr_val ) ;
            success = true ;
        }
        if (node_val && node_val[0]) {
            ndxml_setval(xnode, node_val) ;
            success = true ;
        }
        if (success) {
            ndxml_save(&xmlfile,file) ;
        }
    }
    
    ndxml_destroy(&xmlfile);
    return success ? 0: -1 ;
}

int main(int argc, char *argv[])
{
    int i ;
    char *filename = 0 ;
    char *node_list = 0 ;
    char *attr_name =0 ;
    char *attr_val = 0 ;
    char *node_val = 0 ;
    
    //get config file
    for (i=1; i<argc; i++){
        if(0 == strcmp(argv[i],"-f" ) && i< argc-1) {
           filename = argv[++i] ;
        }
        else if(0== strcmp(argv[i], "-t") && i< argc-1) {
            node_list = argv[++i] ;
        }
        
        else if(0== strcmp(argv[i], "-n") && i< argc-1) {
            attr_name = argv[++i] ;
        }
        
        else if(0== strcmp(argv[i], "-a") && i< argc-1) {
            attr_val = argv[++i] ;
        }
        
        else if(0== strcmp(argv[i], "-v") && i< argc-1) {
            node_val = argv[++i] ;
        }
    }
    
    if (!filename ||
        !node_list ||
        ((!attr_name || !attr_val) &&  !node_val)
        ) {
        
        fprintf(stderr, "usage:  -f xmlfile-name.xml -t xml.node.tree -n attribute-name -a attr-val -v xml-node-value\n") ;
        exit(-1) ;
    }
    
    
    if (0==set_xml_value(filename, node_list, attr_name, attr_val, node_val)) {
        return 0;
    }
    else {
        fprintf(stderr, "set %s %s value error\n", filename, node_list) ;
        exit(1) ;
    }
    
}

