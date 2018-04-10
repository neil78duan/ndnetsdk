#rules of makefile 
#if this file is modified 
#please use 
#>make clean 
#>make
#to make all 

#debug flag 
#DEBUG =y


#profile , hot spot find
#PROFILE=y

tme23456=$(DEBUG)XX
ifeq ($(tme23456),XX)
DEBUG = y
endif

tme23457=$(PROFILE)XX
ifeq ($(PROFILE),XX)
PROFILE = y
endif


ARCH_MACHINE = $(shell uname -m)
OS_kernel = $(shell uname -s | tr '[A-Z]' '[a-z]')

AIM_NAME = $(OS_kernel)_$(ARCH_MACHINE)

TOPDIR = $(NDHOME)
CURDIR = .
SRCDIR = $(CURDIR)/src
OBJDIR = $(CURDIR)/obj

WORKDIR = $(TOPDIR)/bin/$(AIM_NAME)
LIBDIR = $(TOPDIR)/lib/$(AIM_NAME)
LIBOUTPUT = $(TOPDIR)/lib

ifeq ($(OS_kernel),linux)
    CFLAGS += -D__ND_LINUX__
endif
ifeq ($(OS_kernel),darwin)
    CFLAGS += -D__ND_MAC__
	LFLAGS += -liconv

endif

CFLAGS += -c -w -O
LFLAGS +=  -lpthread  -lm


ifeq ($(DEBUG),y)
    CFLAGS +=  -g -DDEBUG  -DND_DEBUG
    ifeq ($(PROFILE),y)
        CFLAGS += -pg
    else
    endif

    CLIENT_LIB := ndclient_$(AIM_NAME)_d
    SRV_LIB := ndsdk_$(AIM_NAME)_d
	COMMON_LIB := ndcommon_$(AIM_NAME)_d
else
    CFLAGS += -DNDEBUG
    CLIENT_LIB := ndclient_$(AIM_NAME)
    SRV_LIB := ndsdk_$(AIM_NAME)
	COMMON_LIB := ndcommon_$(AIM_NAME)
endif

PLATFORM_BITS =  $(shell  getconf LONG_BIT )

ifeq ($(PLATFORM_BITS),64)
	CFLAGS += -DX86_64
else
endif


#create objdir

TMPPARAM1 = $(shell  [ -d  $(TOPDIR)/lib ] || mkdir $(TOPDIR)/lib )
TMPPARAM1 = $(shell  [ -d  $(TOPDIR)/bin ] || mkdir $(TOPDIR)/bin )

TMPPARAM1 = $(shell  [ -d  $(WORKDIR) ] || mkdir $(WORKDIR))
TMPPARAM1 = $(shell  [ -d  $(LIBDIR) ] || mkdir $(LIBDIR))

CC = cc
CPP = c++
AR = ar rv


