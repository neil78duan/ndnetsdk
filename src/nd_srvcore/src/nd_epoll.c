//
//  nd_epoll
//  mtnetwork
//
//  Created by duanxiuyun on 15-3-9.
//  Copyright (c) 2015Äê duanxiuyun. All rights reserved.
//

#if defined(__ND_MAC__)
#include "nd_srvcore/nd_epoll.h"
/*
int regeister_socket(int epool_handle, int sock_fd,uint64_t udata)
{
	struct kevent changes[1];
	EV_SET(&changes[0], sock_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
	
	changes[0].udata = (void*)udata ;
	if (udata==0) {
		changes[0].udata =(void*)(uint64_t) sock_fd;
	}
	return kevent(epool_handle, changes, 1, NULL, 0, NULL);
}
*/

int epoll_create( int maxnumber )
{
	return kqueue();
}

int epoll_wait(int epfd, struct epoll_event *events,int maxevents, int timeout)
{
	struct timespec tmsp = {0, timeout *1000000} ;
	
	return  kevent(epfd, NULL, 0, (struct kevent *) events, maxevents, &tmsp);
	
}


int epoll_ctl(int epool_handle, int op, int sock_fd, struct epoll_event *event)
{
	if (op==EPOLL_CTL_ADD ) {
		if (event->events & EPOLLOUT) {
			
			struct kevent changes[1];
			EV_SET(&changes[0], sock_fd, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
			
			
			changes[0].udata =(void*) event->data.u64 ;
			if (changes[0].udata==0) {
				changes[0].udata =(void*)(uint64_t) sock_fd;
			}
			return kevent(epool_handle, changes, 1, NULL, 0, NULL);

		}
		else {
			
			struct kevent changes[1];
			EV_SET(&changes[0], sock_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
			
			changes[0].udata =(void*) event->data.u64 ;
			if (changes[0].udata==0) {
				changes[0].udata =(void*)(uint64_t) sock_fd;
			}
			return kevent(epool_handle, changes, 1, NULL, 0, NULL);

		}
		
		return 0;
	}
	else if(op==EPOLL_CTL_DEL) {
		struct kevent ke;
		EV_SET(&ke, sock_fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
		kevent(epool_handle, &ke, 1, NULL, 0, NULL);
		EV_SET(&ke, sock_fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
		kevent(epool_handle, &ke, 1, NULL, 0, NULL);
	}
	return 0;
}




#endif
