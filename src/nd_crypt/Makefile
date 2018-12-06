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

CFLAGS += -I$(TOPDIR)/include -I$(TOPDIR)/include/nd_crypt/rsah -I$(TOPDIR)/include/nd_crypt

######################

VPATH :=  $(SRCDIR)  $(OBJDIR) ./rsa

SRC := $(shell cd ./src; ls | grep '\.c\>'	)


OBJS := $(patsubst %.c, %.o,$(SRC) )  $(patsubst %.cpp, %.o,$(SRC) )

PathOBJS :=$(patsubst %.c, $(OBJDIR)/%.o, $(SRC) ) 
#####################

SRCrsa := $(shell cd ./rsa; ls | grep .c	)

OBJSrsa := $(patsubst %.c, %.o,$(SRCrsa) )  $(patsubst %.cpp, %.o,$(SRCrsa) )

PathOBJSrsa :=$(patsubst %.c, $(OBJDIR)/%.o, $(SRCrsa) ) 

OBJS += $(OBJSrsa) 
PathOBJS += $(PathOBJSrsa)

##########make 
##############################################################

ifeq ($(DEBUG),y)
	PROJS = $(LIBDIR)/libnd_crypt_dbg.a
	DLLPROJS = $(LIBDIR)/libnd_crypt_dbg.so
else 
	PROJS = $(LIBDIR)/libnd_crypt.a
	DLLPROJS = $(LIBDIR)/libnd_crypt.so
endif

ifeq ($(BUILD_DLL),y)
   PROJS = $(DLLPROJS)
   LOCAL_CFLAG =  $(CFLAGS) -fPIC
   LibFLAGS += -lnd_common_dbg -L$(LIBDIR)
else 
   LOCAL_CFLAG =  $(CFLAGS) 
endif


#all: $(PROJS)

all : $(OBJS)

$(PROJS): $(OBJS)
	$(AR) $(PROJS)  $(PathOBJS) $(LibFLAGS) 

$(DLLPROJS): $(OBJS)
	$(CC_DLL) -fPIC -o $(DLLPROJS)  $(PathOBJS) $(LibFLAGS) -liconv

.c.o:
	$(CC) $(INCLUDE) $(LOCAL_CFLAG) -o $(OBJDIR)/$*.o $<


#.cpp.o:
#	$(GPP) $(INCLUDE) $(CFLAGS) -o $(OBJDIR)/$*.o $<

clean_dll:
	rm -f $(DLLPROJS) $(PathOBJS)

clean:
	rm -f $(PROJS)  $(PathOBJS)  $(PathOBJS:.o=.d)


