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

ifeq ($(DEBUG),y)
   PROJS = $(LIBDIR)/libndclient_dbg.a
else 
   PROJS = $(LIBDIR)/libndclient.a
endif

######################

VPATH :=  ./c_api ./cpp_api

OBJS := msg_format.o nd_client.o nd_connector.o nd_exch_key.o
PathOBJS := $(OBJDIR)/msg_format.o $(OBJDIR)/nd_client.o $(OBJDIR)/nd_connector.o $(OBJDIR)/nd_exch_key.o

##############################################################


all: $(PROJS)

$(PROJS): $(OBJS)
	$(AR) $(PROJS)  $(PathOBJS) $(LibFLAGS) 
	
.c.o:
	$(CC) $(INCLUDE) $(CFLAGS) -o $(OBJDIR)/$*.o $<
.cpp.o:
	$(CPP) $(INCLUDE) $(CFLAGS) -o $(OBJDIR)/$*.o $<
	
clean:
	rm -f $(PROJS) $(PathOBJS)  $(PathOBJS:.o=.d)


