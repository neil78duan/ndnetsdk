/* file : recbuf.c
 *  recycle buf for c language
 * version 1.0 
 * author : neil 
 * 2005-11-22
 */


#include "nd_common/nd_common.h"
#include "nd_common/nd_recbuf.h"

//��д���ݵ�ֱ��ģʽ,���ֻ����һ���߳���ʹ��buf
//������_cbuf_write/_cbuf_read����д�������Ч��
//���ǲ��Ƽ�ʹ��
//����������д���ݵĵײ��������
int _cbuf_write(ndrecybuf_t *pbuf,char *data, size_t len);
//�ӻ������ж�ȡ����
//buf���ݱ�����ĵ�ַ,size��Ҫ��ȡ�ַ��ĳ���
//return value ��ȡ���ݵĳ���
int _cbuf_read(ndrecybuf_t *pbuf,char *buf, size_t size);

//__INLINE__ char *cbuf_addr(ndrecybuf_t *pbuf) {return pbuf->m_buf ;}
static __INLINE__ char *cbuf_tail(ndrecybuf_t *pbuf)
	{return (char*)pbuf->m_buf+(size_t)C_BUF_SIZE;}	//����β��

//�������ݴ�ŵ�˳��0 start < end 
//__INLINE__ int cbuf_order(ndrecybuf_t *pbuf)	
//{return (pbuf->m_pend>=pbuf->m_pstart) ;}


//�������ݵĳ���
size_t ndcbuf_datalen(ndrecybuf_t *pbuf)
{
	nd_assert(pbuf->m_nInit) ;
	if(cbuf_order(pbuf)) {
		return (size_t)(pbuf->m_pend - pbuf->m_pstart) ;
	}
	else {
		return C_BUF_SIZE - (size_t)(pbuf->m_pstart-pbuf->m_pend) ;
	}
}

//�õ����л��峤��
//�ܴ洢���ݵ�����ʼ�ձ�capacity ��һ��,�������һ���Ϊ�ǿյ�
size_t ndcbuf_freespace(ndrecybuf_t *pbuf)
{
	size_t ret  ;
	if(cbuf_order(pbuf)) {
		ret =  cbuf_capacity(pbuf) - (size_t)(pbuf->m_pend - pbuf->m_pstart);
	}
	else {
		ret = (size_t)(pbuf->m_pstart - pbuf->m_pend)  ;
	}
	return ret -1 ;
}

//��д����
int ndcbuf_read(ndrecybuf_t *pbuf,void *buf, size_t size,int flag)
{
	size_t datalen = ndcbuf_datalen(pbuf) ;
	if(EBUF_SPECIFIED==flag) {
		if(datalen<size){
			return -1 ;
		}
	}
	else {
		if(datalen<size)
			size = datalen ;
	}
	return _cbuf_read(pbuf,buf,size) ;
}

int ndcbuf_write(ndrecybuf_t *pbuf,void *data, size_t datalen,int flag)
{
	size_t spacelen = ndcbuf_freespace(pbuf) ;
	if(EBUF_SPECIFIED==flag) {
		if(spacelen<datalen){
			return -1 ;
		}
	}
	else {
		if(spacelen<datalen)
			datalen = spacelen;
	}
	return _cbuf_write(pbuf,data,datalen) ;
}

void ndcbuf_sub_data(ndrecybuf_t *pbuf,size_t len)
{
	pbuf->m_pstart += len ;
	if(pbuf->m_pstart==pbuf->m_pend )
		ndcbuf_reset(pbuf);
}
//����������д���ݵĵײ��������
int _cbuf_write(ndrecybuf_t *pbuf,char *data, size_t len)
{
	size_t ret = len ;
	size_t  taillen ;
	/*size_t spaceLen = cbuf_freespace(pbuf) ;
	if(len>spaceLen) {
		//�ȴ��������ݱ�����
		//need to do something !
		return -1 ;
	}
	*/
	nd_assert(pbuf->m_nInit) ;
	//���幻��,д������
	taillen = (size_t)(cbuf_tail(pbuf)-pbuf->m_pend );
	if(taillen>=len){
		//�ռ乻�� ����Ҫ��ͷ
		memcpy((void*)pbuf->m_pend,(void*)data,len ) ;
		pbuf->m_pend += len ;
		if(pbuf->m_pend>=cbuf_tail(pbuf)){
			pbuf->m_pend = pbuf->m_buf ;
		}
		
	}
	else {
		//���ݿ�ʼѭ��
		memcpy((void*)pbuf->m_pend,(void*)data,taillen ) ;
		len -= taillen ;
		data += taillen ;
		memcpy((void*)pbuf->m_buf,(void*)data, len) ;
		pbuf->m_pend = pbuf->m_buf+len ;
	}
	return (int)ret ;
}

//�ӻ������ж�ȡ����
//buf���ݱ�����ĵ�ַ,size��Ҫ��ȡ�ַ��ĳ���
//return value ��ȡ���ݵĳ���
int _cbuf_read(ndrecybuf_t *pbuf,char *buf, size_t size)
{
	size_t ret = size ;
	size_t  taillen =0;
	/*size_t data_len = cbuf_datalen(pbuf) ;
	if(size>data_len) {
		//û����ô������,�ȴ�����д��
		//need to do something !
		return -1 ;
	}
	*/
	nd_assert(pbuf->m_nInit) ;
	//��������ô�����ݹ���
	if(cbuf_order(pbuf)){
		//����û�л�ͷ
		memcpy((void*)buf,(void*)pbuf->m_pstart,size ) ;
		pbuf->m_pstart += size ;
		if(pbuf->m_pstart==pbuf->m_pend) {
			ndcbuf_reset(pbuf) ;
		}
		return (int)size ;
	}

	//���ݿ�ʼѭ��
	taillen = (size_t)(cbuf_tail(pbuf) - pbuf->m_pstart );
	if(taillen>=size) {
		//β�����ݹ���ȡ�ĳ���
		memcpy((void*)buf,(void*)pbuf->m_pstart,size ) ;		
		pbuf->m_pstart += size ;
		if(pbuf->m_pstart>=cbuf_tail(pbuf)) {
			pbuf->m_pstart = pbuf->m_buf ;
		}
	}
	else {
		memcpy((void*)buf,(void*)pbuf->m_pstart,taillen ) ;
		size -= taillen ;
		buf += taillen ;
		memcpy((void*)buf,(void*)pbuf->m_buf, size) ;
		pbuf->m_pstart = pbuf->m_buf + size ;
		if(pbuf->m_pstart==pbuf->m_pend) {
			ndcbuf_reset(pbuf) ;
		}
	}
	return (int)ret ;
}
/************************************************************************/
/* line buff                                                            */
/************************************************************************/


int _lbuf_init(struct nd_linebuf *pbuf, size_t data_size) 
{
	pbuf->__buf = malloc(data_size) ;
	if (!pbuf->__buf){
		nd_logerror("create line_buf error cant not malloc %s\n" AND nd_last_error()) ;
		return -1 ;
	}
	pbuf->buf_capacity = data_size ;
	pbuf->auto_inc = 0;
	_lbuf_reset(pbuf);
	return 0;
}

int _lbuf_realloc(struct nd_linebuf *pbuf, size_t newsize)
{
	void *newaddr ;
	size_t dl = _lbuf_datalen(pbuf) ;

	if (pbuf->buf_capacity >= newsize) {
		nd_logerror("newsize is too small\n ");
		return -1;
	}

	newaddr = malloc(newsize) ;	
	if (!newaddr){
		nd_logerror("malloc error %s\n " AND nd_last_error() ) ;
		return -1 ;
	}
	if (dl > 0){
		dl = _lbuf_read(pbuf,newaddr,newsize, EBUF_ALL) ;		
	}
	free(pbuf->__buf) ;
	pbuf->__buf = newaddr ;
	pbuf->buf_capacity = newsize ;
	_lbuf_reset(pbuf) ;
	pbuf->__end += dl ;
	*(pbuf->__end) = 0;
	return 0;
}

void _lbuf_destroy(struct nd_linebuf *pbuf)
{
	if (ND_ALLOC_MM_VALID(pbuf) && ND_ALLOC_MM_VALID(pbuf->__buf)){
		free(pbuf->__buf) ;
		pbuf->__buf = 0 ;
		_lbuf_reset(pbuf);
		pbuf->buf_capacity = 0 ;
	}
}

void _lbuf_move_ahead(struct nd_linebuf *pbuf) 
{
	size_t ahead = pbuf->__start - pbuf->__buf;
	size_t len = _lbuf_datalen(pbuf);
	if(len==0){
		_lbuf_reset(pbuf);
	}
	else if(ahead >= sizeof(size_t)){			//avoid recover
		memcpy((void*)(pbuf->__buf),pbuf->__start, len) ;
		pbuf->__start = pbuf->__buf; 
		pbuf->__end = pbuf->__buf + len ;
		*(pbuf->__end) = 0;
	}
	
}

void _lbuf_tryto_move_ahead(struct nd_linebuf *pbuf) 
{
	size_t header = pbuf->__start - pbuf->__buf ;
	//size_t tailer = pbuf->__buf + lbuf_capacity(pbuf) ;
	if(header>= (_lbuf_capacity(pbuf)>>2) ) {
		_lbuf_move_ahead(pbuf) ;
	}
}

void _lbuf_sub_data(struct nd_linebuf *pbuf,size_t len)
{
	nd_assert(pbuf->__start+len <= pbuf->__end) ;
	nd_assert(pbuf->__start+len <= pbuf->__buf + _lbuf_capacity(pbuf)) ;
	pbuf->__start += len ;
	if(pbuf->__start>=pbuf->__end )
		_lbuf_reset(pbuf);
	else 
		_lbuf_tryto_move_ahead(pbuf);
}


int _lbuf_read(struct nd_linebuf *pbuf,void *buf, size_t size,int flag)
{
	size_t datalen = _lbuf_datalen(pbuf) ;
	if(EBUF_SPECIFIED==flag) {
		if(datalen<size){
			return -1 ;
		}
	}
	else {
		if(datalen<size)
			size = datalen ;
	}

	memcpy(buf,pbuf->__start, size) ;
	_lbuf_sub_data(pbuf, size) ;
	return (int)size ;

}

static int _tryto_inc_capacity(struct nd_linebuf *pbuf, size_t increase_size)
{
	size_t size_new;
	if (!pbuf->auto_inc) {
		return -1;
	}
	size_new = pbuf->buf_capacity + increase_size;
	if (size_new > pbuf->buf_capacity * 2) {
		size_new = ND_ROUNDUP8(size_new);
	}
	else {
		size_new = pbuf->buf_capacity * 2;
	}
	return ndlbuf_realloc(pbuf, size_new);
}

int _lbuf_write(struct nd_linebuf *pbuf,void *data, size_t datalen,int flag)
{
	size_t space_len = _lbuf_freespace(pbuf) ;
	if(EBUF_SPECIFIED==flag){
		if(space_len < datalen ) {
			size_t free_capacity = _lbuf_free_capacity(pbuf) ;
			if (free_capacity < datalen) {
				if(!pbuf->auto_inc || -1==_tryto_inc_capacity(pbuf,datalen)) {
					return -1;
				}
			}
			else {
				_lbuf_move_ahead(pbuf);
			}
		}
	}
	else {
		if(space_len < datalen ) {
			_lbuf_move_ahead(pbuf) ;
			space_len = _lbuf_free_capacity(pbuf) ;
			if (space_len < datalen) {
				if (-1 == _tryto_inc_capacity(pbuf, datalen)) {
					datalen = space_len;
				}
			}
				
		}
	}
	memcpy(pbuf->__end, data, datalen) ;
	//pbuf->__end += datalen ;
	_lbuf_add_data(pbuf, datalen) ;
	return (int) datalen ;
}

void _lbuf_add_data(struct nd_linebuf *pbuf,size_t len)
{
	pbuf->__end += len ;
	*(pbuf->__end) = 0;
	nd_assert(pbuf->__end <= pbuf->__buf +_lbuf_capacity(pbuf) ) ;
}
