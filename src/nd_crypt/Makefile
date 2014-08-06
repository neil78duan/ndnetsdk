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

CFLAGS += -I$(TOPDIR)/include -I$(TOPDIR)/include/nd_crypt/rsah -I$(TOPDIR)/include/nd_crypt

ifeq ($(DEBUG),y)
   PROJS = $(LIBDIR)/libnd_crypt_dbg.a
else 
   PROJS = $(LIBDIR)/libnd_crypt.a
endif

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


all: $(PROJS)

$(PROJS): $(OBJS)
	$(AR) $(PROJS)  $(PathOBJS) $(LibFLAGS) 
	
.c.o:
	$(CC) $(INCLUDE) $(CFLAGS) -o $(OBJDIR)/$*.o $<
#.cpp.o:
#	$(GPP) $(INCLUDE) $(CFLAGS) -o $(OBJDIR)/$*.o $<
	
clean:
	rm -f $(PROJS) $(PathOBJS)  $(PathOBJS:.o=.d)


###############create dependence file #############
.PHONY: clean

PathSRC :=$(patsubst %.c, ./src/%.c, $(SRC) ) 


include $(PathOBJS:.o=.d)

$(PathOBJS:.o=.d): $(PathSRC)
	set -e;\
	$(CC) -MM  $(INCLUDE) $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

#############################################################

