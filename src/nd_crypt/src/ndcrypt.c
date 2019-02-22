/* file ndcrypt.c
 * implemention user function interface of nd crypt
 * neil duan
 * 2008-5
 */

#include "nd_common/nd_common.h"
#include "nd_crypt/nd_crypt.h"


/*
#if defined _DEBUG || defined DEBUG
#pragma comment(lib, "nd_common_dbg.lib")
#else 
#pragma comment(lib, "nd_common.lib")

#endif
*/

/* stuff src 
 * input :  src stuffed data 
 			dataln current data length
 			stufflen target data len 
 * return : data start address 
 */
#define STUFF_ELE	(char)0x20
char *crypt_stuff(char *src, int datalen, int stufflen )
{
	int i ;
	char *p = src ;
	if( stufflen <1 || stufflen <= datalen){
		return 0 ;
	}
	src += datalen ;
	stufflen -= datalen ;
	//memset(src, STUFF_ELE , stufflen - datalen ) ;	//stuff STUFF_ELE to buffer between src+datalen and src+datalen
	//p[stufflen-1] = (char)(stufflen - datalen) ;	//recrod stuff length 
	
	for(i=0; i<stufflen -1; i++){
		*src = STUFF_ELE ;src++ ;
	}
	*src = stufflen ;	//recrod stuff length 
	return p ;
}


/* encrypt/decrypt
 * input : @data input data
 *			@len data length
 *			@key crypt key
 * output : @data crypted data 
 * if the data length is not match with crypt unity size , it will  fill the spaces (ansi-code = 0x20) to the tail of data.
 * 
 * return value :on error return 0, else return data length of encrypted
 * 
 */
int nd_TEAencrypt(unsigned char *data, int data_len, tea_k *key) 
{
	int vlen = sizeof(tea_v) ;			
	int stuff_len ,i,n,new_len;
	
	tea_v *v = (tea_v *)data;

	stuff_len = data_len % vlen ;
	n = data_len / vlen ;
	
	if(stuff_len){
		++n ;
		new_len = n * vlen;
		crypt_stuff((char*)data, data_len, new_len ) ;
	}
	else {
		new_len = data_len ;
	}
	for(i=0; i<n; i++){
		tea_enc(key, v) ;
		++v ;
	}
	return new_len ;
}

int nd_TEAdecrypt(unsigned char *data, int data_len, tea_k *key) 
{
	int vlen = sizeof(tea_v) ;			
	int i,n;
	
	tea_v *v = (tea_v *)data;
	
#if 1 //defined _DEBUG || defined DEBUG
	int stuff_len = data_len % vlen ;
	if(stuff_len){
		//nd_assert(0);		
		return -1 ;
	}
#endif

	n = data_len / vlen ;
	
	for(i=0; i<n; i++){
		tea_dec(key, v) ;
		++v ;
	}
	
	return data_len ;
}

int nd_TEAGenKey(tea_k *key, char *seed)
{
	//nd_assert(sizeof(tea_k)==16) ;
	MD5CryptStr16(seed,(char*)key) ; 
	return 16 ;
}
/* convert crypt string to output string 
 * add by duan !
 */
char* MD5ToString(unsigned char src[16], unsigned char desc[33])
{
	unsigned int i ;
	unsigned char *p = desc ;
	for(i=0; i<16; i++)
	{
		ndsprintf((char*)p, "%02x",src[i]) ;
		p += 2 ;
	}
	desc[32] = 0 ;
	return (char*)desc ;
}


char *MD5CryptStr16(const char *input, char output[16]) 
{
	MD5_CTX context;
	unsigned int len = (unsigned int) ndstrlen (input);
	
	MD5Init (&context);
	MD5Update (&context, (unsigned char*)input, len);
	MD5Final ((unsigned char*)output, &context);
	
	return output ;
}

char *MD5CryptStr32(const char *in_text, char output[33])
{
	unsigned int len = (unsigned int)ndstrlen(in_text);
	char tmp[16];
	MD5_CTX context;

	MD5Init(&context);
	MD5Update(&context, (unsigned char*)in_text, len);
	MD5Final((unsigned char*)tmp, &context);

	MD5ToString((unsigned char*)tmp,(unsigned char*) output);
	return output;

}

/* 
 * @inlen input length
 * @input data address of input
 * @output buffer address char[16]
 */
char *MD5Crypt16(const void *input, int inlen, char output[16])
{
	MD5_CTX context;
	
	MD5Init (&context);
	MD5Update (&context, (unsigned char*)input, inlen);
	MD5Final ((unsigned char*)output, &context);
	
	return output ;
}

char *MD5Crypt32(const void *input, int inlen, char output[33])
{
	char tmp[16] ;
	MD5_CTX context;
	
	MD5Init (&context);
	MD5Update (&context, (unsigned char*)input, inlen);
	MD5Final ((unsigned char*)tmp, &context);
	
	MD5ToString((unsigned char*)tmp, (unsigned char*)output);
	return output ;
}

char *MD5file(const char *filepath, char output[33])
{
	char tmp[16];
	MD5_CTX context;
	unsigned char buf[4096];
	FILE *pf = fopen(filepath, "rb");
	if (!pf) {
		return NULL;
	}

	MD5Init(&context);
	do 	{
		size_t readlen = fread(buf, 1, sizeof(buf), pf);
		if (readlen > 0) {
			MD5Update(&context, buf, (unsigned int)readlen);
		}
	} while (0==feof(pf));	

	MD5Final((unsigned char*)tmp, &context);
	MD5ToString((unsigned char*)tmp, (unsigned char*)output);
	fclose(pf);
	return output;
}


int MD5cmp(char src[16], char desc[16])
{
	int ret = 0 ;
	UINT4 *s4 =(UINT4 *) src ;
	UINT4 *d4 =(UINT4 *) desc ;

	ret = s4[0] - d4[0] ;
	if(ret)
		return ret ;
	ret = s4[1] - d4[1] ;
	if(ret)
		return ret ;
	ret = s4[2] - d4[2] ;
	if(ret)
		return ret ;
	ret = s4[3] - d4[3] ;
	return ret ;
}

//base 64

const char _nd_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#define END_OF_BASE64_ENCODED_DATA           ('=')
#define BASE64_END_OF_BUFFER                 (0xFD)
#define BASE64_IGNORABLE_CHARACTER           (0xFE)
#define BASE64_UNKNOWN_VALUE                 (0xFF)
#define BASE64_NUMBER_OF_CHARACTERS_PER_LINE (72)

int base64_encode( const char * source, int len, char * destination_string )
{

	int loop_index                = 0;
	int number_of_bytes_to_encode = len;

	NDUINT8 byte_to_add = 0;
	NDUINT8 byte_1      = 0;
	NDUINT8 byte_2      = 0;
	NDUINT8 byte_3      = 0;

	NDUINT32 number_of_bytes_encoded = (NDUINT32) ( (double) number_of_bytes_to_encode / (double) 0.75 ) + 1;
	char * destination;

	number_of_bytes_encoded += (NDUINT32)( ( ( number_of_bytes_encoded / BASE64_NUMBER_OF_CHARACTERS_PER_LINE ) + 1 ) * 2 );

	destination = destination_string;

	number_of_bytes_encoded = 0;

	while( loop_index < number_of_bytes_to_encode ) {
		// Output the first byte
		byte_1 = source[ loop_index ];
		byte_to_add = _nd_alphabet[ ( byte_1 >> 2 ) ];

		destination[ number_of_bytes_encoded ] =  byte_to_add ;
		number_of_bytes_encoded++;

		loop_index++;

		if ( loop_index >= number_of_bytes_to_encode ) {
			// We're at the end of the data to encode
			byte_2 = 0;
			byte_to_add = _nd_alphabet[ ( ( ( byte_1 & 0x03 ) << 4 ) | ( ( byte_2 & 0xF0 ) >> 4 ) ) ];

			destination[ number_of_bytes_encoded ] = byte_to_add;
			number_of_bytes_encoded++;

			destination[ number_of_bytes_encoded ] =  END_OF_BASE64_ENCODED_DATA;
			number_of_bytes_encoded++;

			destination[ number_of_bytes_encoded ] =  END_OF_BASE64_ENCODED_DATA;

			destination[ number_of_bytes_encoded + 1 ] = 0;

			return 0;
		}
		else{
			byte_2 = source[ loop_index ];
		}

		byte_to_add = _nd_alphabet[ ( ( ( byte_1 & 0x03 ) << 4 ) | ( ( byte_2 & 0xF0 ) >> 4 ) ) ];

		destination[ number_of_bytes_encoded ] = byte_to_add;
		number_of_bytes_encoded++;

		loop_index++;

		if ( loop_index >= number_of_bytes_to_encode )
		{
			// We ran out of bytes, we need to add the last half of byte_2 and pad
			byte_3 = 0;

			byte_to_add = _nd_alphabet[ ( ( ( byte_2 & 0x0F ) << 2 ) | ( ( byte_3 & 0xC0 ) >> 6 ) ) ];

			destination[ number_of_bytes_encoded ] = byte_to_add;
			number_of_bytes_encoded++;

			destination[ number_of_bytes_encoded ] = END_OF_BASE64_ENCODED_DATA;

			destination[ number_of_bytes_encoded + 1 ] = 0;

			return 0;
		}
		else
		{
			byte_3 = source[ loop_index ];
		}

		loop_index++;

		byte_to_add = _nd_alphabet[ ( ( ( byte_2 & 0x0F ) << 2 ) | ( ( byte_3 & 0xC0 ) >> 6 ) ) ];

		destination[ number_of_bytes_encoded ] = byte_to_add;
		number_of_bytes_encoded++;

		byte_to_add = _nd_alphabet[ ( byte_3 & 0x3F ) ];

		destination[ number_of_bytes_encoded ] = byte_to_add;
		number_of_bytes_encoded++;

		if ( ( number_of_bytes_encoded % BASE64_NUMBER_OF_CHARACTERS_PER_LINE ) == 0 )
		{
			destination[ number_of_bytes_encoded ] = 13;		//return
			number_of_bytes_encoded++;

			destination[ number_of_bytes_encoded ] = 10;		//new line
			number_of_bytes_encoded++;
		}
	}

	destination[ number_of_bytes_encoded ] = END_OF_BASE64_ENCODED_DATA;

	destination[ number_of_bytes_encoded + 1 ] = 0;

	return 0;
}


int base64_decode( unsigned char *input, unsigned int input_len, unsigned char *output, unsigned int *output_len )
{
	static char inalphabet[256], decoder[256];
	int i, bits, c = 0, char_count, errors = 0;
	unsigned int input_idx = 0;
	unsigned int output_idx = 0;
	
	for (i = (sizeof _nd_alphabet) - 1; i >= 0 ; i--) {
		inalphabet[(int)_nd_alphabet[i]] = 1;
		decoder[(int)_nd_alphabet[i]] = i;
	}
	
	char_count = 0;
	bits = 0;
	for( input_idx=0; input_idx < input_len ; input_idx++ ) {
		c = input[ input_idx ];
		if (c == '=')
			break;
		if (c > 255 || ! inalphabet[c])
			continue;
		bits += decoder[c];
		char_count++;
		if (char_count == 4) {
			output[ output_idx++ ] = (bits >> 16);
			output[ output_idx++ ] = ((bits >> 8) & 0xff);
			output[ output_idx++ ] = ( bits & 0xff);
			bits = 0;
			char_count = 0;
		} else {
			bits <<= 6;
		}
	}
	
	if( c == '=' ) {
		switch (char_count) {
			case 1:
				errors++;
				break;
			case 2:
				output[ output_idx++ ] = ( bits >> 10 );
				break;
			case 3:
				output[ output_idx++ ] = ( bits >> 16 );
				output[ output_idx++ ] = (( bits >> 8 ) & 0xff);
				break;
		}
	} else if ( input_idx < input_len ) {
		if (char_count) {
			errors++;
		}
	}
	
	if (output_len) {
		*output_len = output_idx;
	}
	
	return errors ? -1 : 0;
}

