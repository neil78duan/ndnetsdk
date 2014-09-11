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

PLATFORM_BITS =  $(shell  getconf LONG_BIT )



TOPDIR = $(NDHOME)
CURDIR = .
SRCDIR = $(CURDIR)/src
OBJDIR = $(CURDIR)/obj

ifeq ($(PLATFORM_BITS),64)
	CFLAGS += -DX86_64
	WORKDIR = $(TOPDIR)/bin64
	LIBDIR = $(TOPDIR)/lib64
else
	WORKDIR = $(TOPDIR)/bin
	LIBDIR = $(TOPDIR)/lib
endif

	
TMPPARAM1 = $(shell  [ -d  $(WORKDIR) ] || mkdir $(WORKDIR))
	
TMPPARAM1 = $(shell  [ -d  $(LIBDIR) ] || mkdir $(LIBDIR))

CC = cc
CPP = c++
AR = ar  rv


