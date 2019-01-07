//
//  myEpoll.h
//  mtnetwork
//
//  Created by duanxiuyun on 15-3-9.
//  Copyright (c) 2015duanxiuyun. All rights reserved.
//

#ifndef mtnetwork_myEpoll_h
#define mtnetwork_myEpoll_h

#if defined(__ND_MAC__)

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

typedef union epoll_data {
	void    *ptr;
	int      fd;
	uint32_t u32;
	uint64_t u64;
} epoll_data_t;

//define kevent look-like epoll_event
struct epoll_event {
	uintptr_t	ident;		/* identifier for this event */
	int16_t		events;		/* filter for event */
	uint16_t	flags;		/* general flags */
	uint32_t	fflags;		/* filter-specific flags */
	intptr_t	data_mac;		/* filter-specific data */
	epoll_data_t data;		/* opaque user data identifier */
};

#define EPOLL_CTL_ADD 1
#define EPOLL_CTL_MOD 2
#define EPOLL_CTL_DEL 3

#define  EPOLLIN 1
#define  EPOLLOUT 2
#define  EPOLLRDHUP 4
#define  EPOLLPRI 8
#define EPOLLERR 0x10
#define EPOLLHUP 0x20
#define EPOLLET 0x40
#define EPOLLONESHOT 0x80


int epoll_create( int maxnumber );
int epoll_wait(int epfd, struct epoll_event *events,int maxevents, int timeout);
int epoll_ctl(int epool_handle, int op, int sock_fd, struct epoll_event *event);

#endif


#endif
