# This is new version of make 
# you need only copy you *.c file to ./src/ 
# and copy *.h to ./include/ 
# makefile is so smart that he can find all *.c file to compile 
# 
# But notice!!! PLEASE REMOVE c file FROM ./src that you NEEDN'T compile
# neil duan 
# 2003-8-25

include ../Rules.make


#create objdir

objdir := $(shell [ -d obj ] || mkdir obj) 


##############set flag

CFLAGS += -I$(TOPDIR)/include


######################

VPATH :=  $(SRCDIR)  $(OBJDIR)

SRC := $(shell cd ./src; ls | grep '\.c\>'	)


OBJS := $(patsubst %.c, %.o,$(SRC) )  $(patsubst %.cpp, %.o,$(SRC) )

PathOBJS :=$(patsubst %.c, $(OBJDIR)/%.o, $(SRC) ) 

##########make 
##############################################################

LibFLAGS += -L$(LIBDIR)

ifeq ($(DEBUG),y)
	PROJS = $(LIBDIR)/libnd_net_dbg.a
	DLLPROJS = $(LIBDIR)/libnd_net_dbg.so
	LibFLAGS += -lnd_common_dbg -lnd_crypt_dbg 
else 
	PROJS = $(LIBDIR)/libnd_net.a
	DLLPROJS = $(LIBDIR)/libnd_net.so
	LibFLAGS += -lnd_common -lnd_crypt 
endif

ifeq ($(BUILD_DLL),y)
   PROJS = $(DLLPROJS)
   LOCAL_CFLAG =  $(CFLAGS) -fPIC
else 
   LOCAL_CFLAG =  $(CFLAGS) 
endif


#all: $(PROJS)

all : $(OBJS)

$(PROJS): $(OBJS)
	$(AR) $(PROJS)  $(PathOBJS) 

$(DLLPROJS): $(OBJS)
	$(CC_DLL) -fPIC -o $(DLLPROJS)  $(PathOBJS) $(LibFLAGS) -liconv

.c.o:
	$(CC) $(INCLUDE) $(LOCAL_CFLAG) -o $(OBJDIR)/$*.o $<
#.cpp.o:
#	$(GPP) $(INCLUDE) $(CFLAGS) -o $(OBJDIR)/$*.o $<
	
clean:
	rm -f $(PROJS) $(DLLPROJS) $(PathOBJS)  $(PathOBJS:.o=.d)
	rm -rf $(OBJDIR)
