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

#define user intel c compile
USE_INTEL_CC=n

#ifeq ($(OSTYPE),linux)
#	CFLAGS += -D__LINUX__
#elifeq ($(OSTYPE),linux-gnu)
#	CFLAGS += -D__LINUX__
#else
#	CFLAGS += -D__BSD__
#endif

CFLAGS += -c -O -D__LINUX__ -DND_UNIX
LFLAGS +=  -lpthread -lrt

ifeq ($(DEBUG),y)
	CFLAGS +=  -g -DDEBUG  -DND_DEBUG 
	ifeq ($(PROFILE),y)
		CFLAGS += -pg
	else 
	endif
	LFLAGS += -pg
else
	CFLAGS += -DNDEBUG
endif

ifeq ($(UNICODE),y)
	CFLAGS += -DND_UNICODE
else
endif


TOPDIR = ../..
CURDIR = .
SRCDIR = $(CURDIR)/src
OBJDIR = $(CURDIR)/obj
OUTDIR = $(TOPDIR)/obj

ifeq ($(DEBUG),y)
WORKDIR = $(TOPDIR)/bin
LIBDIR = $(TOPDIR)/lib
else
WORKDIR = $(TOPDIR)/bin
LIBDIR = $(TOPDIR)/lib
endif

ifeq ($(USE_INTEL_CC),y)
	CC = icc 
	CPP = i++
	AR = xiar rcs
	LFLAGS += /usr/intel/cc/lib/libimf.a 
else	
	CC = gcc 
	CPP = g++
	AR = ar  rv
	LFLAGS += -lm 
endif

