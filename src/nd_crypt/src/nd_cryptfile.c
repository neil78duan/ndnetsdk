/* file nd_cryptfile.c
 *
 * define file encrypt / decrypt
 *
 * create by duan
 *
 * 2012/7/20 10:25:01
 */

#include "nd_crypt/nd_cryptfile.h"
#include "nd_crypt/tea.h"
#include "nd_common/nd_common.h"

int _ndc_fflush(NDC_FILE *pf, int is_end)  ;
#define CRYPT_VAL_SIZE sizeof(tea_v)

NDC_FILE *ndc_fopen_r(const char *filename, const char *passwd) 
{
	size_t len ;
	long fsize ;
	char *p ;
	tea_v tail ;
	NDC_FILE *pndcf = (NDC_FILE*)malloc(sizeof(NDC_FILE)) ;
	if (!pndcf){
		return NULL ;
	}

	pndcf->page_pos = 0;
	pndcf->cur_file_pos = 0;	
	pndcf->file_end = 0;
	pndcf->isload = 0;
	pndcf->iswrite = 0 ;
	pndcf->size_inbuf = 0;
	pndcf->is_flush = 0;

	nd_TEAGenKey(&pndcf->passwd,(char*) passwd) ;
	pndcf->pf = fopen(filename, "rb") ;
	if(!pndcf->pf) {
		free(pndcf);
		return NULL ;
	}

	fseek(pndcf->pf, 0, SEEK_END);
	fsize = ftell(pndcf->pf);	

	if(fsize==0 || (fsize % CRYPT_VAL_SIZE)!=0) {
		fclose(pndcf->pf) ;
		free(pndcf);
		return 0;
	}
	fseek(pndcf->pf, fsize - CRYPT_VAL_SIZE, SEEK_SET);
	len = fread(&tail,1,CRYPT_VAL_SIZE, pndcf->pf ) ;
	if(len != CRYPT_VAL_SIZE) {
		fclose(pndcf->pf) ;
		free(pndcf);
		return 0 ;
	}
	tea_dec(&pndcf->passwd, &tail) ;
	p = (char*) &tail ;
	p += CRYPT_VAL_SIZE - 1;
	if((unsigned char)*p >(unsigned char) CRYPT_VAL_SIZE) {
		fclose(pndcf->pf) ;
		free(pndcf);
		return 0 ;
	}
	fseek(pndcf->pf, 0, SEEK_SET);	
	pndcf->file_end = fsize - *p ;

	return pndcf;
}


int load_page( NDC_FILE *pf )
{
	fseek(pf->pf,(long) pf->cur_file_pos, SEEK_SET);	
	pf->size_inbuf = fread(pf->page_buf, 1, NDC_FILE_BUF, pf->pf) ;
	if (0==pf->size_inbuf){
		return 0 ;
	}

	nd_TEAdecrypt((unsigned char*)pf->page_buf,(int) pf->size_inbuf, &pf->passwd)  ;
	pf->isload = 1 ;
	pf->cur_file_pos = ftell(pf->pf);
// 
// 	if (pf->cur_file_pos > pf->file_end) {
// 		pf->size_inbuf -=  pf->cur_file_pos - pf->file_end ;
// 	}
	return (int) pf->size_inbuf;
}

size_t ndc_fread_node(void *buf, size_t elementsize, NDC_FILE *pf ) 
{
	size_t len = elementsize ;
	char *p = buf ;
	size_t left_data = pf->file_end - (pf->cur_file_pos - pf->size_inbuf) - pf->page_pos ;
	
	if (left_data < elementsize) {
		return 0;
	}
RE_LOAD:

	if (pf->isload == 0 || pf->size_inbuf == pf->page_pos){
		if(0==load_page(pf) ) {
			return p - (char*) buf ;
		}
	}	
	while(len-- > 0) {
		*p++ = pf->page_buf[pf->page_pos++] ;
		if (pf->page_pos >= pf->size_inbuf)	{
			pf->isload  = 0 ;
			pf->size_inbuf = 0;
			pf->page_pos = 0 ;
			if(len > 0){
				goto RE_LOAD ;
			}
		}
	}
	return p - (char*) buf ;
	
}

size_t ndc_fread(void *buf, size_t elementsize, size_t count,NDC_FILE *pf ) 
{
	size_t i=0;
	char *p = (char*)buf ;
	for(i=0;i<count; i++) {
		if (0==ndc_fread_node(p, elementsize,pf ) )	{
			return i ;
		}
		p+= elementsize ;
	}
	return i ;
}


NDC_FILE *ndc_fopen_w(const char *filename, const char *passwd) 
{
	NDC_FILE *pndcf = (NDC_FILE*)malloc(sizeof(NDC_FILE)) ;
	if (!pndcf){
		return NULL ;
	}

	pndcf->page_pos = 0;
	pndcf->cur_file_pos = 0;	
	pndcf->file_end = 0;
	pndcf->isload = 0;
	pndcf->iswrite = 1 ;
	pndcf->size_inbuf = 0;
	pndcf->is_flush = 0;

	nd_TEAGenKey(&pndcf->passwd,(char*) passwd) ;
	pndcf->pf = fopen(filename, "wb") ;
	if (!pndcf->pf){
		free(pndcf) ;
		return NULL;
	}
	return pndcf;
}
int write2file(void *buf, int count,  NDC_FILE *pf)
{
	int ret = 0;
	size_t i;
	tea_v v ;
	tea_v *src = (tea_v*)buf ;
	for(i=0; i<count; i++) {
		v = *src ;
		src++ ;
		tea_enc(&pf->passwd, &v) ;
		ret +=(int) fwrite(&v,1, CRYPT_VAL_SIZE, pf->pf	) ;
	}
	return ret ;
} 

size_t ndc_fwrite(const void *buf, size_t elementsize, size_t count,NDC_FILE *pf ) 
{
	int i, j ;
	char *src = (char*) buf ;
	//size_t buf_datalen  = 0 ;
	//size_t totalsize = elementsize * count ;
	if(!pf || !pf->pf ) {
		return 0 ;
	}
	
	for(i=0; i<count; i++ ) {
		for(j=0; j<elementsize; j++) {
			pf->page_buf[pf->size_inbuf++] = *src++ ;
			if (pf->size_inbuf >= NDC_FILE_BUF)	{
				_ndc_fflush(pf, 0) ;
			}
		}
	}
	_ndc_fflush(pf,0) ;
	return count ;
}

int ndc_fclose(NDC_FILE *pf) 
{
	if (!pf){
		return -1 ;
	}
	if (pf->pf){
		_ndc_fflush(pf,1) ;
		fclose(pf->pf) ;
	}
	free(pf) ;
	return 0;
}

int ndc_fseek(NDC_FILE*pf, long offset, int flag) 
{
	long new_pos ;
	size_t old_pos ;
	if(!pf || !pf->pf ) {
		return -1 ;
	}
	if (pf->iswrite){
		return -1;
	}
	if(flag==SEEK_SET) {
		if (offset < 0)	{
			return -1 ;
		}
		new_pos = offset ;
	}
	else if(flag==SEEK_CUR) {
		new_pos = (long)(pf->cur_file_pos + pf->page_pos) + offset ;
	}
	else if(flag==SEEK_END) {
		if (offset > 0)	{
			return -1 ;
		}
		new_pos = (long)pf->file_end + offset ;
	}
	else {
		return -1;
	}
	if (new_pos < 0){
		new_pos = 0 ;
	}
	else if(new_pos > (long)pf->file_end) {
		new_pos =(long) pf->file_end ;
	}
	old_pos = pf->cur_file_pos ;
	pf->page_pos = (size_t) (new_pos % NDC_FILE_BUF ) ;
	pf->cur_file_pos = new_pos - pf->page_pos ;
	if (old_pos != pf->cur_file_pos){
		fseek(pf->pf,(long) pf->cur_file_pos, SEEK_SET);
		pf->isload = 0 ;
	}
	return 0;
}

long ndc_ftell(NDC_FILE *pf) 
{
	if(!pf || !pf->pf) {
		return -1 ;
	}
	return (long) (pf->cur_file_pos + pf->page_pos) ;
}

int ndc_fflush(NDC_FILE *pf) 
{
	return _ndc_fflush(pf, 0) ;
}

int _ndc_fflush(NDC_FILE *pf, int is_end) 
{
	int count ;
	size_t buf_datalen ;
	if(!pf || !pf->pf) {
		return -1 ;
	}
	if (!pf->iswrite){
		return 0 ;
	}
	// there is data in buffer
	buf_datalen = pf->size_inbuf - pf->page_pos ; 
	if (buf_datalen ==0){
		pf->page_pos = 0 ;
		pf->size_inbuf = 0 ;
		if (0==is_end)	{
			return 0;
		}
	}
	count = (int) buf_datalen / CRYPT_VAL_SIZE ;
	if (count > 0){
		count = write2file(&pf->page_buf[pf->page_pos], count ,pf) ;
		pf->page_pos += count ;

		if (pf->page_pos == pf->size_inbuf)	{
			pf->page_pos = 0 ;
			pf->size_inbuf = 0 ;
		}
	}

	if (is_end) {
		char fillval =0;
		size_t fill_data ;
		buf_datalen = pf->size_inbuf - pf->page_pos ; 
		fill_data =  CRYPT_VAL_SIZE -( buf_datalen % CRYPT_VAL_SIZE );
		
		fillval = (char) fill_data ;
		while(fill_data-- > 0) {
			pf->page_buf[pf->size_inbuf++] = fillval;
		}
		count = (int) (pf->size_inbuf - pf->page_pos) / CRYPT_VAL_SIZE ;
		write2file(&pf->page_buf[pf->page_pos], count ,pf) ;
	}

	return 0;
}


void* ndc_load_file_ex(const char *file, size_t *size,const char *fileKey)
{
	size_t data_len, buf_size;
	NDC_FILE *fp;
	char *buf = NULL;

	fp = ndc_fopen_r(file, fileKey);
	if (!fp) {
		//nd_logerror("open file %s : %s\n", file, nd_last_error());
		return 0;
	}
	ndc_fseek(fp, 0, SEEK_END);
	buf_size = ndc_ftell(fp);
	ndc_fseek(fp, 0, SEEK_SET);

	if (buf_size == 0) {
		ndc_fclose(fp);
		//nd_logmsg("load file %s is empty\n", file);
		return NULL;
	}
	buf_size += 1;
	buf = (char*)malloc(buf_size);

	if (!buf){
		ndc_fclose(fp);
		return 0;
	}
	data_len = ndc_fread(buf, 1, buf_size, fp);
	if (data_len == 0 || data_len >= buf_size) {
		ndc_fclose(fp);
		free(buf);
		return 0;

	}
	buf[data_len] = 0;
	ndc_fclose(fp);
	if (size) {
		*size = data_len;
	}
	return (void*)buf;
}
