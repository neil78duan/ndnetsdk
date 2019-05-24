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

SRC_C := $(shell  ls | grep '\.c\>' )
SRC := $(shell  ls | grep '\.cpp\>' )

OBJS := $(patsubst %.cpp, %.o,$(SRC)  )   $(patsubst %.c, %.o,$(SRC_C)  )

PathOBJS :=$(patsubst %.cpp, $(OBJDIR)/%.o, $(SRC) )  $(patsubst %.c, $(OBJDIR)/%.o, $(SRC_C) ) 

PathSRC := $(SRC)  $(SRC_C)
#######################################

LibFLAGS += -L$(LIBDIR)

ifeq ($(DEBUG),y)
	PROJS = $(LIBDIR)/libndclient_dbg.a
	DLLPROJS = $(LIBDIR)/libndclient_dbg.so
	LibFLAGS += -lnd_common_dbg -lnd_crypt_dbg -lnd_net_dbg -lndclient_pack_dbg
else 
	PROJS = $(LIBDIR)/libndclient.a
	DLLPROJS = $(LIBDIR)/libndclient.so
	LibFLAGS += -lnd_common -lnd_crypt  -lnd_net -lndclient_pack
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
	$(CPP_DLL) -fPIC -o $(DLLPROJS)  $(PathOBJS) $(LibFLAGS) -liconv

.c.o:
	$(CC) $(INCLUDE) $(LOCAL_CFLAG) -o $(OBJDIR)/$*.o $<

.cpp.o:
	$(CPP) $(INCLUDE) $(LOCAL_CFLAG) -o $(OBJDIR)/$*.o $<
	
clean:
	rm -f $(PROJS) $(DLLPROJS) $(PathOBJS)  $(PathOBJS:.o=.d)


