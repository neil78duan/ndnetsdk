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

ifeq ($(OS_kernel),linux)
    CFLAGS += -D__ND_LINUX__
endif
ifeq ($(OS_kernel),darwin)
	CFLAGS += -D__ND_MAC__
	LFLAGS += -liconv

endif



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


CFLAGS += -c -w -O  -I$(NDHOME)/include
LFLAGS +=  -lpthread  -lm -L$(NDHOME)/lib


#create objdir

CC = cc
CPP = c++
AR = ar rv


#set obj and src path

VPATH :=  $(SRCDIR) $(OBJDIR)

tme123=$(SRCDIR)XX
ifeq ($(tme123),XX)
	#undef src
else
SRC := $(shell cd $(SRCDIR); ls | grep '\.c\>'	)
SRC_CPP := $(shell cd $(SRCDIR); ls | grep '\.cpp\>'	)
SRC_CC := $(shell cd $(SRCDIR); ls | grep '\.cc\>'	)


tme124=$(OBJDIR)XX
ifeq ($(tme124),XX)
else

OBJS := $(patsubst %.c, %.o,$(SRC) )  $(patsubst %.cpp, %.o,$(SRC_CPP) ) $(patsubst %.cc, %.o,$(SRC_CC) )
PathOBJS :=$(patsubst %.c, $(OBJDIR)/%.o, $(SRC) ) $(patsubst %.cpp, $(OBJDIR)/%.o, $(SRC_CPP) ) $(patsubst %.cc, $(OBJDIR)/%.o, $(SRC_CC) )

endif

endif





