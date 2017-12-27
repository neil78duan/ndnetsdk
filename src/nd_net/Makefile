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
   PROJS = $(LIBDIR)/libnd_net_dbg.a
else 
   PROJS = $(LIBDIR)/libnd_net.a
endif

######################

VPATH :=  $(SRCDIR)  $(OBJDIR)

SRC := $(shell cd ./src; ls | grep '\.c\>'	)


OBJS := $(patsubst %.c, %.o,$(SRC) )  $(patsubst %.cpp, %.o,$(SRC) )

PathOBJS :=$(patsubst %.c, $(OBJDIR)/%.o, $(SRC) ) 

##########make 
##############################################################


all: $(PROJS)

$(PROJS): $(OBJS)
	$(AR) $(PROJS)  $(PathOBJS) $(LibFLAGS) 
	
.c.o:
	$(CC) $(INCLUDE) $(CFLAGS) -o $(OBJDIR)/$*.o $<
#.cpp.o:
#	$(GPP) $(INCLUDE) $(CFLAGS) -o $(OBJDIR)/$*.o $<
	
clean:
	rm -f $(PROJS) $(PathOBJS)  $(PathOBJS:.o=.d)
