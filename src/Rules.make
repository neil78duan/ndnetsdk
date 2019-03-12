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
LIBOUTPUT = $(TOPDIR)/lib/$(AIM_NAME)

ifeq ($(OS_kernel),linux)
    CFLAGS += -D__ND_LINUX__
	DLL_EXT_NAME := so
endif
ifeq ($(OS_kernel),darwin)
    CFLAGS += -D__ND_MAC__
	DLL_EXT_NAME := dylib
endif

CFLAGS += -c -w -O
LFLAGS +=  -lpthread  -lm -ldl -liconv

ifeq ($(DEBUG),y)
    CFLAGS +=  -g -DDEBUG  -DND_DEBUG
    ifeq ($(PROFILE),y)
        CFLAGS += -pg
    else
    endif

    CLIENT_LIB := ndclient_s_d
    SRV_LIB := ndsrv_s_d
	COMMON_LIB := ndcommon_s_d

	CLIENT_DLL := ndclient_d
	SRV_DLL := ndsrv_d
	COMMON_DLL := ndcommon_d
else
    CFLAGS += -DNDEBUG

    CLIENT_LIB := ndclient_s
    SRV_LIB := ndsrv_s
	COMMON_LIB := ndcommon_s

	CLIENT_DLL := ndclient
	SRV_DLL := ndsrv
	COMMON_DLL := ndcommon
endif

PLATFORM_BITS =  $(shell  getconf LONG_BIT )

ifeq ($(PLATFORM_BITS),64)
	CFLAGS += -DX86_64
else
endif

ND_LIBOUTPUT := $(LIBOUTPUT)
ND_CLIENT_LIB := $(ND_LIBOUTPUT)/lib$(CLIENT_LIB).a
ND_SRV_LIB := $(ND_LIBOUTPUT)/lib$(SRV_LIB).a
ND_COMMON_LIB := $(ND_LIBOUTPUT)/lib$(COMMON_LIB).a

ND_CLIENT_DLL := $(ND_LIBOUTPUT)/lib$(CLIENT_DLL).$(DLL_EXT_NAME)
ND_SRV_DLL := $(ND_LIBOUTPUT)/lib$(SRV_DLL).$(DLL_EXT_NAME)
ND_COMMON_DLL := $(ND_LIBOUTPUT)/lib$(COMMON_DLL).$(DLL_EXT_NAME)

SYS_INSTALL_PATH := /usr/lib

#create objdir

TMPPARAM1 = $(shell  [ -d  $(TOPDIR)/lib ] || mkdir $(TOPDIR)/lib )
TMPPARAM1 = $(shell  [ -d  $(TOPDIR)/bin ] || mkdir $(TOPDIR)/bin )

TMPPARAM1 = $(shell  [ -d  $(WORKDIR) ] || mkdir $(WORKDIR))
TMPPARAM1 = $(shell  [ -d  $(LIBDIR) ] || mkdir $(LIBDIR))

CPPFLAGS :=
CC = cc
CPP = c++
AR = ar rv
CC_DLL= cc -shared
CPP_DLL= c++ -shared


