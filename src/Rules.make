#rules of makefile 
#if this file is modified 
#please use 
#>make clean 
#>make
#to make all 

#debug flag 
DEBUG =y


#profile , hot spot find
PROFILE=y

ARCH_MACHINE = $(shell uname -m)
OS_kernel = $(shell uname -s | tr '[A-Z]' '[a-z]')

AIM_NAME = $(OS_kernel)_$(ARCH_MACHINE)

ifeq ($(OS_kernel),linux)
	CFLAGS += -D__LINUX__
endif

ifeq ($(OS_kernel),darwin)
    CFLAGS += -D__MAC_OS__
endif

CFLAGS += -c -w -O  -DND_UNIX
LFLAGS +=  -lpthread  -lm

ifeq ($(DEBUG),y)
	CFLAGS +=  -g -DDEBUG  -DND_DEBUG 
	ifeq ($(PROFILE),y)
		CFLAGS += -pg
	else 
	
	endif
	
#	LFLAGS += -
else
	CFLAGS += -DNDEBUG
endif

#PLATFORM_BITS =  $(shell  getconf LONG_BIT )


TOPDIR = $(NDHOME)
CURDIR = .
SRCDIR = $(CURDIR)/src
OBJDIR = $(CURDIR)/obj

WORKDIR = $(TOPDIR)/bin/$(AIM_NAME)
LIBDIR = $(TOPDIR)/lib/$(AIM_NAME)
LIBOUTPUT = $(TOPDIR)/lib


#ifeq ($(PLATFORM_BITS),64)
#	CFLAGS += -DX86_64
#	WORKDIR = $(TOPDIR)/bin64
#	LIBDIR = $(TOPDIR)/lib64
#else
#	WORKDIR = $(TOPDIR)/bin
#	LIBDIR = $(TOPDIR)/lib
#endif


#create objdir

TMPPARAM1 = $(shell  [ -d  $(TOPDIR)/lib ] || mkdir $(TOPDIR)/lib )
TMPPARAM1 = $(shell  [ -d  $(TOPDIR)/bin ] || mkdir $(TOPDIR)/bin )

TMPPARAM1 = $(shell  [ -d  $(WORKDIR) ] || mkdir $(WORKDIR))
TMPPARAM1 = $(shell  [ -d  $(LIBDIR) ] || mkdir $(LIBDIR))

CC = cc
CPP = c++
AR = ar rv


