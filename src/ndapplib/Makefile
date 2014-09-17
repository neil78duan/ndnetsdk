#make file 

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
    PROJS = $(LIBDIR)/libndapplib_dbg.a
    PROJ_CLIENT=$(LIBDIR)/libndclient_pack_dbg.a
else 
    PROJS = $(LIBDIR)/libndapplib.a
    PROJ_CLIENT=$(LIBDIR)/libndclient_pack.a
endif

######################

VPATH :=  $(SRCDIR)  $(OBJDIR)

SRC_C := $(shell  ls | grep '\.c\>' )
SRC := $(shell  ls | grep '\.cpp\>' )

OBJS := $(patsubst %.cpp, %.o,$(SRC)  )   $(patsubst %.c, %.o,$(SRC_C)  )

PathOBJS :=$(patsubst %.cpp, $(OBJDIR)/%.o, $(SRC) )  $(patsubst %.c, $(OBJDIR)/%.o, $(SRC_C) ) 

PathSRC := $(SRC)  $(SRC_C)


##########make 
##############################################################


all: $(PROJS) $(PROJ_CLIENT)
	
$(PROJS): $(OBJS)
	$(AR) $(PROJS)  $(PathOBJS) $(LibFLAGS) 

$(PROJ_CLIENT):$(OBJDIR)/nd_msgpack.o
	$(AR) $(PROJ_CLIENT)  $(OBJDIR)/nd_msgpack.o $(LibFLAGS)
                
.c.o:
	$(CC) $(INCLUDE) $(CFLAGS) -o $(OBJDIR)/$*.o $<
.cpp.o:
	$(CPP) $(INCLUDE) $(CFLAGS) -o $(OBJDIR)/$*.o $<
	

clean:
	rm -f $(PROJS) $(PathOBJS)  $(PathOBJS:.o=.d)
	

###############create dependence file #############
.PHONY: clean


include $(PathOBJS:.o=.d)

$(PathOBJS:.o=.d): $(SRC)
	set -e;\
	$(CC) -MM  $(INCLUDE) $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$


#############################################################
