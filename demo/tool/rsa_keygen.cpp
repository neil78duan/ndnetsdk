/* file rsa_keygen.cpp
 *
 *
 * create by duan 
 * 2014-10-11
 */


#include "nd_common/nd_common.h"
#include "nd_crypt/nd_crypt.h"

#if defined(ND_DEBUG)
#pragma comment(lib,"vs10_static_cli_dbg.lib")
#else
#pragma comment(lib,"vs10_static_cli.lib")
#endif

int out_pub_file(const char * filename,R_RSA_PUBLIC_KEY &pub_key,int ver_num,char*);
int out_priv_file(const char * filename,R_RSA_PRIVATE_KEY &priv_key,int ver_num,char*);
int TestRsa(R_RSA_PRIVATE_KEY &priv_key);

int main(int argc, char *argv[])
{
	int number_index = 1 ;
	char *_pub_file = NULL;
	char *_pri_file = NULL;
	char md5text[33] ;
	R_RANDOM_STRUCT _rand_contex = {0};
	ND_RSA_CONTEX rsa_contex ={0};
	RSA_HANDLE _h_rsa = &rsa_contex ;
	
	nd_common_init() ;
	if (argc >= 2){
		int i ;
		if (strcmp(argv[1],"-h")==0 ){
			printf("usage %s -p public_key_filename.cpp -s private_key_filename.cpp -v version_number", argv[0]);
			exit(0);
		}
		for(i=1; i<argc-1; i++ ) {
			if (strcmp(argv[i],"-p")==0)	{
				_pub_file = argv[i+1] ;
				++i ;
			}
			else if (strcmp(argv[i],"-s")==0)	{
				_pri_file = argv[i+1] ;
				++i;
			}
			else if (strcmp(argv[i],"-v")==0)	{
				number_index = atoi(argv[i+1]) ;
				++i;
			}
		}

	}

	if (!_pri_file || !_pub_file){
		printf("usage %s -p public_key_filename.cpp -s private_key_filename.cpp -v version_number", argv[0]);
		exit(1) ;
	}

	if (-1==nd_RSAInit(_h_rsa)){

		printf("Create RS KEY error\n" ) ;
		exit(1) ;
	}
	TestRsa(_h_rsa->privateKey);

	MD5CryptToStr32((char*)&_h_rsa->privateKey,sizeof(_h_rsa->privateKey),md5text);

	if (-1==out_pub_file(_pub_file,_h_rsa->publicKey,number_index,md5text)) {
		printf("out put public key to file %s error \n", _pub_file) ;
		exit(1) ;
	}

	MD5CryptToStr32((char*)&_h_rsa->publicKey,sizeof(_h_rsa->publicKey),md5text);

	if (-1==out_priv_file(_pri_file,_h_rsa->privateKey,number_index,md5text)) {
		printf("out put private key to file %s error \n", _pri_file) ;
		exit(1) ;
	}
	nd_RSAdestroy(_h_rsa);
	exit(0);
}


#define OUT_BUF_TEXT(_pf, _bufname)				\
	fprintf(_pf, "\t// data of " #_bufname ) ;	\
	fprintf(_pf, "\n\t{ 0x%x", _bufname[0] ) ;	\
	for (int i=1; i<sizeof(_bufname); i++){		\
		fprintf(_pf, ", 0x%x", _bufname[i]) ;	\
		if (i%10 == 0){							\
			fprintf(_pf, "\n\t") ;				\
		}										\
	}											\
	fprintf(pf, "}") 

int out_pub_file(const char * filename,R_RSA_PUBLIC_KEY &pub_key,int ver_num, char *md5text_privkey)
{
	FILE *pf = fopen(filename, "w") ;
	if (!pf){
		return -1 ;
	}
	fprintf(pf, "/* file %s \n", filename) ;
	fprintf(pf, " * RSA public key \n * create by auto_tool please DO NOT modify \n"
		" * datetime: %s\n" , nd_get_datatimestr() ) ;
	fprintf(pf, " */ \n\n") ;


	fprintf(pf, "#include \"nd_crypt/nd_crypt.h\"\n") ;
	fprintf(pf, "#include \"nd_crypt/nd_pubkey.h\"\n") ;


	fprintf(pf, "/* publish version */\nint __g_pub_publis_num=%d ; \n\n", ver_num) ;

	fprintf(pf, "/* md5 of private key  */\nchar *__g_privkey_md5=\"%s\" ; \n\n\n", md5text_privkey) ;

	fprintf(pf, "/* RSA public key  data */ \n") ;
	fprintf(pf, "R_RSA_PUBLIC_KEY __nd_pub_key = \n{\t%d,//bits of key\n" ,pub_key.bits) ;
	OUT_BUF_TEXT(pf, pub_key.modulus) ;
	fprintf(pf, ",\n\t "  )  ;
	OUT_BUF_TEXT(pf, pub_key.exponent) ;

	fprintf(pf, "\n} ;\n\n") ;
	fprintf(pf, "/* RSA public key  data --end */ \n\n") ;

	fprintf(pf, "int nd_get_certificate_version(void)\n{\n\treturn __g_pub_publis_num ;\n}\n\n") ;
	fprintf(pf, "char *nd_get_privatekey_md5(void)\n{\n\treturn __g_privkey_md5 ;\n}\n\n" ) ;
	fprintf(pf, "char* nd_calc_publickey_md5(char text[33])\n{\n\treturn MD5CryptToStr32((char*)&__nd_pub_key, sizeof(__nd_pub_key), text) ;\n}\n\n") ;
	fprintf(pf, "R_RSA_PUBLIC_KEY *nd_get_publickey(void) \n{\n\treturn &__nd_pub_key ;\n}\n\n") ;
	fclose(pf) ;
	return 0 ;
}

int out_priv_file(const char * filename,R_RSA_PRIVATE_KEY &priv_key,int ver_num,char *md5text_pubkey)
{
	FILE *pf = fopen(filename, "w") ;
	if (!pf){
		return -1 ;
	}

	fprintf(pf, "/* file %s \n", filename) ;
	fprintf(pf, " * RSA private key \n * create by auto_tool please DO NOT modify \n"
		" * datetime: %s\n", nd_get_datatimestr() ) ;
	fprintf(pf, " */ \n\n") ;

	fprintf(pf, "#include \"nd_crypt/nd_crypt.h\"\n") ;
	fprintf(pf, "#include \"ndapplib/applib.h\"\n") ;

	fprintf(pf, "/* publish version */\nint __g_pirv_publis_num=%d ; \n\n", ver_num) ;

	fprintf(pf, "/* md5 of private key  */\nchar *__g_pubkey_md5=\"%s\" ; \n\n\n", md5text_pubkey) ;

	fprintf(pf, "/* RSA private key  data */ \n") ;
	fprintf(pf, " R_RSA_PRIVATE_KEY __nd_pri_key = \n{\t%d,\t\t//bits of key\n", priv_key.bits) ;
	OUT_BUF_TEXT(pf, priv_key.modulus) ;
	fprintf(pf, ",\n"  )  ;
	
	OUT_BUF_TEXT(pf, priv_key.publicExponent) ;
	fprintf(pf, ",\n"  )  ;

	OUT_BUF_TEXT(pf, priv_key.exponent) ;
	fprintf(pf, ",\n\t{"  )  ;

	OUT_BUF_TEXT(pf, priv_key.prime[0]) ;
	fprintf(pf, ",\n"  )  ;
	OUT_BUF_TEXT(pf, priv_key.prime[1]) ;
	fprintf(pf, "\n\t},\n\t{"  )  ;
	OUT_BUF_TEXT(pf, priv_key.primeExponent[0]) ;
	fprintf(pf, ",\n"  )  ;
	OUT_BUF_TEXT(pf, priv_key.primeExponent[1]) ;
	fprintf(pf, "\n\t},\n"  )  ;
	OUT_BUF_TEXT(pf, priv_key.coefficient) ;


	fprintf(pf, "\n} ;\n\n") ;
	fprintf(pf, "/* RSA private key  data --end */ \n\n") ;

	fprintf(pf, "int nd_get_certificate_version(void)\n{\n\treturn __g_pirv_publis_num ;\n}\n\n") ;
	fprintf(pf, "char *nd_get_publickey_md5(void)\n{\n\treturn __g_pubkey_md5 ;\n}\n\n" ) ;
	fprintf(pf, "char* nd_calc_privatekey_md5(char text[33])\n{\n\treturn MD5CryptToStr32((char*)&__nd_pri_key, sizeof(__nd_pri_key), text) ;\n}\n\n") ;
	fprintf(pf, "R_RSA_PRIVATE_KEY *nd_get_privatekey(void) \n{\n\treturn &__nd_pri_key ;\n}\n\n") ;

	fclose(pf) ;
	return 0 ;
}


int TestRsa(R_RSA_PRIVATE_KEY &priv_key)
{
	int len1=1024, len2 =1024;
	int text_size ;
	char *p = "hello world" ;
	char buf1[1024] ,buf2[1024];

	ND_RSA_CONTEX rsa_contex = {0} ;
	memcpy((void*)&rsa_contex.privateKey, &priv_key, sizeof(rsa_contex.privateKey) );
	memcpy((void*)&rsa_contex.publicKey, &priv_key, sizeof(rsa_contex.publicKey) );
	RSAinit_random(&rsa_contex.randomStruct);

	text_size = strlen(p) ;

	//test public encrypt , private decrypt
	if (nd_RSAPublicEncrypt(buf1, &len1, p, text_size, &rsa_contex)){
		printf("RSA text error \n") ;
		getch();
		exit(1) ;
	}
	if (nd_RSAPrivateDecrypt(buf2, &len2, buf1, len1, &rsa_contex)){
		printf("RSA text error \n") ;
		getch();
		exit(1) ;
	}

	nd_assert(text_size == len2) ;
	buf2[len2] = 0 ;
	if(strcmp(p, buf2)) {
		printf("RSA text error \n") ;
		getch();
		exit(1) ;
	}

	//test private encrypt , public decrypt
	if (nd_RSAPrivateEncrypt(buf1, &len1, p, text_size, &rsa_contex)){
		printf("RSA text error \n") ;
		getch();
		exit(1) ;
	}
	if (nd_RSAPublicDecrypt(buf2, &len2, buf1, len1, &rsa_contex)){
		printf("RSA text error \n") ;
		getch();
		exit(1) ;
	}

	nd_assert(text_size == len2) ;
	buf2[len2] = 0 ;
	if(strcmp(p, buf2)) {
		printf("RSA text error \n") ;
		getch();
		exit(1) ;
	}
	return 0 ;
}