#rules of makefile 
#if this file is modified 
#please use 
#>make clean 
#>make
#to make all 

#debug flag 
DEBUG =y

#define uincode
UNICODE=n

#define use intel c compile
USE_INTEL_CC=n

#profile , hot spot find
PROFILE=y

#ifeq ($(OSTYPE),linux)
#	CFLAGS += -D__LINUX__
#elifeq ($(OSTYPE),linux-gnu)
#	CFLAGS += -D__LINUX__
#else
#	CFLAGS += -D_MAC_OS_ -D__BSD__
#endif

#CFLAGS += -c -w -O -D__LINUX__ -DND_UNIX -DBUILD_AS_STATIC_LIB -D_PG_SERVER_ -finput-charset=GBK -D_GNU_SOURCE

# for mac os x
CFLAGS += -c -w -O -D__MAC_OS__ -DND_UNIX -DBUILD_AS_STATIC_LIB -D_PG_SERVER_

# -fexec-charset=GBK 

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

ifeq ($(UNICODE),y)
	CFLAGS += -DND_UNICODE
else
endif

PLATFORM_BITS =  $(shell  getconf LONG_BIT )



TOPDIR = ../..
CURDIR = .
SRCDIR = $(CURDIR)/src
OBJDIR = $(CURDIR)/obj
OUTDIR = $(TOPDIR)/obj

ifeq ($(PLATFORM_BITS),64)
	CFLAGS += -DX86_64
	WORKDIR = $(TOPDIR)/bin64
	LIBDIR = ../../lib64
else
	WORKDIR = $(TOPDIR)/bin
	LIBDIR = ../../lib
endif

ifeq ($(USE_INTEL_CC),y)
	CC = icc 
	CPP = i++
	AR = xiar rcs
	LFLAGS += /usr/intel/cc/lib/libimf.a 
else	
	CC = cc
	CPP = c++
	AR = ar  rv
	LFLAGS += -lm 
endif

