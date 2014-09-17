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
objdir := $(shell  ls | grep obj )
ifeq ($(objdir),obj)
else
	tmpobjdir := $(shell mkdir obj)
endif

##############set flag

CFLAGS += -I$(TOPDIR)/include 

ifeq ($(DEBUG),y)
   PROJS = $(LIBDIR)/libndclient_dbg.a
else 
   PROJS = $(LIBDIR)/libndclient.a
endif

######################

VPATH :=  ./c_api ./cpp_api

#SRC := $(shell cd ./c_api; ls | grep '\.c\>'	)
#OBJS := $(patsubst %.c, %.o,$(SRC) )  $(patsubst %.cpp, %.o,$(SRC) )
#PathOBJS :=$(patsubst %.c, $(OBJDIR)/%.o, $(SRC) )

OBJS := msg_format.o nd_client.o nd_connector.o
PathOBJS := $(OBJDIR)/msg_format.o $(OBJDIR)/nd_client.o $(OBJDIR)/nd_connector.o
#####################

#SRCrsa := $(shell cd ./cpp_api; ls | grep .c	)

#OBJSrsa := $(patsubst %.c, %.o,$(SRCrsa) )  $(patsubst %.cpp, %.o,$(SRCrsa) )

#PathOBJSrsa :=$(patsubst %.c, $(OBJDIR)/%.o, $(SRCrsa) )

#OBJS += $(OBJSrsa)
#PathOBJS += $(PathOBJSrsa)

##########make 
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


###############create dependence file #############
.PHONY: clean

#PathSRC :=$(patsubst %.c, ./src/%.c, $(SRC) )
PathSRC := c_api/msg_format.cpp c_api/nd_connector.c cpp_api/nd_client.cpp

include $(PathOBJS:.o=.d)

$(PathOBJS:.o=.d): $(PathSRC)
	set -e;\
	$(CC) -MM  $(INCLUDE) $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

#############################################################

