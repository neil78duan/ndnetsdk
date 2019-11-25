#make file 

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


##########make 
##############################################################


LibFLAGS += -L$(LIBDIR)

ifeq ($(DEBUG),y)

    PROJS = $(LIBDIR)/libndapplib_dbg.a
    PROJ_CLIENT=$(LIBDIR)/libndclient_pack_dbg.a
	DLLPROJS = $(LIBDIR)/libndapplib_dbg.so
	LibFLAGS += -lnd_common_dbg -lnd_crypt_dbg -lnd_net_dbg -lnd_srvcore_dbg
else 

    PROJS = $(LIBDIR)/libndapplib.a
    PROJ_CLIENT=$(LIBDIR)/libndclient_pack.a
	DLLPROJS = $(LIBDIR)/libndapplib.so
	LibFLAGS += -lnd_common -lnd_crypt  -lnd_net -lnd_srvcore
endif

ifeq ($(BUILD_DLL),y)
   PROJS = $(DLLPROJS)
   LOCAL_CFLAG =  $(CFLAGS) -fPIC
else 
   LOCAL_CFLAG =  $(CFLAGS) 
endif


#all: $(PROJS) $(PROJ_CLIENT)

all : $(OBJS)

$(PROJS): $(OBJS)
	$(AR) $(PROJS)  $(PathOBJS) 

$(DLLPROJS): $(OBJS)
	$(CPP_DLL) -fPIC -o $(DLLPROJS)  $(PathOBJS) $(LibFLAGS) -liconv

$(PROJ_CLIENT):$(OBJDIR)/nd_msgpack.o $(OBJDIR)/nd_datatransfer.o
	$(AR) $(PROJ_CLIENT)  $(OBJDIR)/nd_msgpack.o  $(OBJDIR)/nd_datatransfer.o

.c.o:
	$(CC) $(INCLUDE) $(LOCAL_CFLAG) -o $(OBJDIR)/$*.o $<

.cpp.o:
	$(CPP) $(INCLUDE) $(LOCAL_CFLAG) $(CPPFLAGS) -o $(OBJDIR)/$*.o $<

clean:
	rm -f $(PROJS) $(DLLPROJS) $(PathOBJS)  $(PathOBJS:.o=.d)
	rm -rf $(OBJDIR)
	
