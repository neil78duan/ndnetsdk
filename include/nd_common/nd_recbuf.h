/* file : _cbuf.h
 * header file of buffer manager for c language 
 * version 1.0 
 * author : neil 
 * 2005-11-22 
 */

/*
 * recircle binary buffer
 */

#ifndef _CBUF_H_
#define _CBUF_H_

enum{ C_BUF_SIZE = 0x10000} ;
typedef struct nd_recbuf
{
	char *m_pstart, *m_pend ;	
#ifdef ND_DEBUG
	int m_nInit ;		//is init
#endif
	char m_buf[C_BUF_SIZE] ;
}ndrecybuf_t ;	

enum _eNDRecyRead{
	EBUF_ALL=0,	//try to read / wirte all data
	EBUF_SPECIFIED	//try to read specified length data
};

static __INLINE__ size_t cbuf_capacity(ndrecybuf_t *pbuf) 
{
	return (size_t)C_BUF_SIZE;
}

static __INLINE__ void ndcbuf_reset(ndrecybuf_t *pbuf)
{
	pbuf->m_pstart = pbuf->m_buf;
	pbuf->m_pend = pbuf->m_buf ;	
}

static __INLINE__ void ndcbuf_init(ndrecybuf_t *pbuf) 
{
	ndcbuf_reset(pbuf);

#ifdef ND_DEBUG
	pbuf->m_nInit = 1 ; 
#endif
}

static __INLINE__ void ndcbuf_destroy(ndrecybuf_t *pbuf)
{
#ifdef ND_DEBUG
	pbuf->m_nInit = 0 ; 
#endif
}

static __INLINE__ void* ndcbuf_data_addr(ndrecybuf_t *pbuf)
{
	return pbuf->m_pstart ;
}

static __INLINE__ void ndcbuf_add_data(ndrecybuf_t *pbuf,size_t len)
{
	pbuf->m_pend += len ;
}

//get the data store in buffer , in line or circle(begin > end)
static __INLINE__ int cbuf_order(ndrecybuf_t *pbuf)	
{
	return (pbuf->m_pend>=pbuf->m_pstart) ;
}

ND_COMMON_API void ndcbuf_sub_data(ndrecybuf_t *pbuf,size_t len);
ND_COMMON_API size_t ndcbuf_datalen(ndrecybuf_t *pbuf); //get data length
ND_COMMON_API size_t ndcbuf_freespace(ndrecybuf_t *pbuf);//get free space

//read/write
//return value -1error else return data length of  read/write 
ND_COMMON_API int ndcbuf_read(ndrecybuf_t *pbuf,void *buf, size_t size,int flag);
ND_COMMON_API int ndcbuf_write(ndrecybuf_t *pbuf,void *data, size_t datalen,int flag);

/************************************************************************/
/* define line io buffer                                                */
/************************************************************************/

struct nd_linebuf
{
	int auto_inc:1;			//auto increase capacity
	int is_alloced : 1 ;	// the __buf is alloc by this line buf 
	size_t buf_capacity ;
	char *__start, *__end ;	
	char *__buf;
};

//linear buffer
struct line_buf_hdr
{
	int auto_inc;			//auto increase capacity 
	size_t buf_capacity ;
	char *__start, *__end ;	
};
static __INLINE__ size_t _lbuf_capacity(struct nd_linebuf *pbuf) 
{
	return (size_t)pbuf->buf_capacity;
}	//total capacity


static __INLINE__ size_t _lbuf_datalen(struct nd_linebuf *pbuf)
{
	return (size_t)(pbuf->__end - pbuf->__start) ;
}

static __INLINE__ size_t _lbuf_free_capacity(struct nd_linebuf *pbuf) 
{
	return _lbuf_capacity(pbuf) - _lbuf_datalen(pbuf);
}	//


static __INLINE__ void _lbuf_reset(struct nd_linebuf *pbuf)
{
	pbuf->__start = pbuf->__buf;
	pbuf->__end = pbuf->__buf ;	
	if (pbuf->__buf) {
		pbuf->__buf[0] = 0;
	}
}

static __INLINE__ void *_lbuf_data(struct nd_linebuf *pbuf)
{
	return (void*) pbuf->__start ;
}
static __INLINE__ void *_lbuf_tail(struct nd_linebuf *pbuf)
{
	return (void*) pbuf->__end ;
}
static __INLINE__ void *_lbuf_addr(struct nd_linebuf *pbuf)
{
	return (void*) pbuf->__end ;
}
static __INLINE__ void *_lbuf_raw_addr(struct nd_linebuf *pbuf)
{
	return (void*) pbuf->__buf ;
}

static __INLINE__ size_t _lbuf_freespace(struct nd_linebuf *pbuf)
{
	nd_assert(pbuf->__end <= pbuf->__buf+_lbuf_capacity(pbuf));
	return (size_t) (pbuf->__buf + _lbuf_capacity(pbuf) - pbuf->__end) ;
}
static __INLINE__ void ndlbuf_auto_inc_enable(struct nd_linebuf *pbuf)
{
	pbuf->auto_inc = 1;
}

static __INLINE__ int ndlbuf_is_auto_inc(struct nd_linebuf *pbuf)
{
	return pbuf->auto_inc ;
}

static __INLINE__ void ndlbuf_auto_inc_disable(struct nd_linebuf *pbuf)
{
	pbuf->auto_inc = 0;
}

static __INLINE__ void ndlbuf_set_zero_tail(struct nd_linebuf *pbuf)
{
	if (pbuf->__end < pbuf->__buf + pbuf->buf_capacity) {
		*(pbuf->__end) = 0;
	}
}

ND_COMMON_API void _lbuf_add_data(struct nd_linebuf *pbuf,size_t len);
//read/write
ND_COMMON_API int _lbuf_read(struct nd_linebuf *pbuf,void *buf, size_t size,int flag);
ND_COMMON_API int _lbuf_write(struct nd_linebuf *pbuf,void *data, size_t datalen,int flag);

ND_COMMON_API void _lbuf_sub_data(struct nd_linebuf *pbuf,size_t len);
//move data to buffer head
ND_COMMON_API void _lbuf_move_ahead(struct nd_linebuf *pbuf) ;
ND_COMMON_API void _lbuf_tryto_move_ahead(struct nd_linebuf *pbuf) ;
ND_COMMON_API int _lbuf_init(struct nd_linebuf *pbuf, size_t data_size) ;
ND_COMMON_API  void _lbuf_destroy(struct nd_linebuf *pbuf);
ND_COMMON_API  int _lbuf_realloc(struct nd_linebuf *pbuf, size_t newsize);
//////////////////////////////////////////////////////////////////////////


#define ndlbuf_capacity(pbuf)		_lbuf_capacity((struct nd_linebuf*) (pbuf))
#define ndlbuf_datalen(pbuf)		_lbuf_datalen((struct nd_linebuf*) (pbuf))  
#define ndlbuf_free_capacity(pbuf)	_lbuf_free_capacity((struct nd_linebuf*) (pbuf))
#define ndlbuf_reset(pbuf)			_lbuf_reset((struct nd_linebuf*) (pbuf))
#define ndlbuf_init(pbuf, size)		_lbuf_init((struct nd_linebuf*) (pbuf), size)
#define ndlbuf_realloc(pbuf, size)  _lbuf_realloc(pbuf, size)
#define ndlbuf_destroy(pbuf)		_lbuf_destroy((struct nd_linebuf*) (pbuf))
#define ndlbuf_data(pbuf)			_lbuf_data((struct nd_linebuf*) (pbuf))

#define ndlbuf_tail(pbuf)			_lbuf_tail((struct nd_linebuf*) (pbuf))

#define ndlbuf_addr(pbuf)			_lbuf_addr((struct nd_linebuf*) (pbuf))
#define ndlbuf_raw_addr(pbuf)		_lbuf_raw_addr((struct nd_linebuf*) (pbuf))
#define ndlbuf_freespace(pbuf)		_lbuf_freespace((struct nd_linebuf*) (pbuf))
#define ndlbuf_add_data(pbuf, len)	_lbuf_add_data((struct nd_linebuf*)((pbuf)), len)
#define ndlbuf_read(pbuf, buf, size, flag )			_lbuf_read((struct nd_linebuf*) (pbuf),buf, size, flag)

#define ndlbuf_write(pbuf,data, size, flag)			_lbuf_write((struct nd_linebuf*) (pbuf),data, size, flag)

#define ndlbuf_sub_data(pbuf, len) _lbuf_sub_data((struct nd_linebuf*) (pbuf), len)

#define ndlbuf_move_ahead(pbuf) _lbuf_move_ahead((struct nd_linebuf*) (pbuf))

#define ndlbuf_tryto_move_ahead(pbuf) _lbuf_tryto_move_ahead((struct nd_linebuf*) (pbuf))

#endif 

