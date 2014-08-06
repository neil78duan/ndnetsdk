/* file : _cbuf.h
 * header file of buffer manager for c language 
 * version 1.0 
 * author : neil 
 * 2005-11-22 
 */

/*
 * 当用udp实现类tcp功能的时候需要这样的缓冲,所以重写一个代替
 * c++ 的class (ND_CRecyBuf)
 */

#ifndef _CBUF_H_
#define _CBUF_H_

enum{ C_BUF_SIZE = 0x10000} ;	
typedef struct nd_recbuf
{
	char *m_pstart, *m_pend ;	
#ifdef ND_DEBUG
	int m_nInit ;		//是否初始化
#endif
	char m_buf[C_BUF_SIZE] ;
}ndrecybuf_t ;	

enum _eNDRecyRead{
	EBUF_ALL=0,	//读取如果缓冲数据少于指定长度,则读取缓冲中所有数据.写入时缓冲不够则填满缓冲
	EBUF_SPECIFIED	//读取(写入)指定长,如果数据(空间)不过返回
};

static __INLINE__ size_t cbuf_capacity(ndrecybuf_t *pbuf) 
{
	return (size_t)C_BUF_SIZE;
}	//得到容量

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

//返回数据存放的顺序 0 start < end 
static __INLINE__ int cbuf_order(ndrecybuf_t *pbuf)	
{
	return (pbuf->m_pend>=pbuf->m_pstart) ;
}

ND_COMMON_API void ndcbuf_sub_data(ndrecybuf_t *pbuf,size_t len);
ND_COMMON_API size_t ndcbuf_datalen(ndrecybuf_t *pbuf); //返回数据的长度
ND_COMMON_API size_t ndcbuf_freespace(ndrecybuf_t *pbuf);//得到空闲缓冲长度

//读写数据
//return value -1error else return data length of  read/write 
ND_COMMON_API int ndcbuf_read(ndrecybuf_t *pbuf,void *buf, size_t size,int flag);
ND_COMMON_API int ndcbuf_write(ndrecybuf_t *pbuf,void *data, size_t datalen,int flag);

/************************************************************************/
/* define line io buffer                                                */
/************************************************************************/

struct nd_linebuf
{
	size_t buf_capacity ;
	char *__start, *__end ;	
	//char __buf[C_BUF_SIZE] ;	
	char *__buf;
};

//定义线性缓冲头
struct line_buf_hdr
{
	size_t buf_capacity ;
	char *__start, *__end ;	
};
static __INLINE__ size_t _lbuf_capacity(struct nd_linebuf *pbuf) 
{
	return (size_t)pbuf->buf_capacity;
}	//得到容量


static __INLINE__ size_t _lbuf_datalen(struct nd_linebuf *pbuf)
{
	return (size_t)(pbuf->__end - pbuf->__start) ;
}

static __INLINE__ size_t _lbuf_free_capacity(struct nd_linebuf *pbuf) 
{
	return _lbuf_capacity(pbuf) - _lbuf_datalen(pbuf);
}	//得到容量


static __INLINE__ void _lbuf_reset(struct nd_linebuf *pbuf)
{
	pbuf->__start = pbuf->__buf;
	pbuf->__end = pbuf->__buf ;	
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
ND_COMMON_API void _lbuf_add_data(struct nd_linebuf *pbuf,size_t len);
//读写数据
ND_COMMON_API int _lbuf_read(struct nd_linebuf *pbuf,void *buf, size_t size,int flag);
ND_COMMON_API int _lbuf_write(struct nd_linebuf *pbuf,void *data, size_t datalen,int flag);

ND_COMMON_API void _lbuf_sub_data(struct nd_linebuf *pbuf,size_t len);
//把数据移动到buf head
ND_COMMON_API void _lbuf_move_ahead(struct nd_linebuf *pbuf) ;
ND_COMMON_API void _lbuf_tryto_move_ahead(struct nd_linebuf *pbuf) ;
ND_COMMON_API int _lbuf_init(struct nd_linebuf *pbuf, size_t data_size) ;
ND_COMMON_API  void _lbuf_destroy(struct nd_linebuf *pbuf);
ND_COMMON_API  int _lbuf_realloc(struct nd_linebuf *pbuf, size_t newsize);
//////////////////////////////////////////////////////////////////////////

/*
定义一下宏的目的是为了能够操作任意大小的缓冲。
只要符合一下格式即可
struct nd_linebuf_xx
{
	struct line_buf_hdr  hdr ;
	char __buf[xx] ;	
};

*/
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

#define ndlbuf_write(pbuf,buf, size, flag)			_lbuf_write((struct nd_linebuf*) (pbuf),buf, size, flag)

#define ndlbuf_sub_data(pbuf, len) _lbuf_sub_data((struct nd_linebuf*) (pbuf), len)

#define ndlbuf_move_ahead(pbuf) _lbuf_move_ahead((struct nd_linebuf*) (pbuf))

#define ndlbuf_tryto_move_ahead(pbuf) _lbuf_tryto_move_ahead((struct nd_linebuf*) (pbuf))

#endif 

