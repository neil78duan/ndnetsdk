# c test  Makefile 
# 2005-11-6 
# neil duan 


include $(NDHOME)/src/Rules.make


MYLIBS := $(LIBOUTPUT)/lib$(SRV_LIB).a
LFLAGS += -L$(LIBOUTPUT) -l$(SRV_LIB)

PROJS  = $(WORKDIR)/unit_test

#OBJS   :=  maintest.o alloc.o tbintree.o mutex.o thsrv.o thmsg.o test_udp.o

SRC := $(shell  ls | grep '\.c\>' )

OBJS := $(patsubst %.c, %.o,$(SRC)  )

CFLAGS += -I$(TOPDIR)/include 


all: $(PROJS)
			
$(PROJS): $(OBJS) $(MYLIBS)
	$(CC) -o  $(PROJS)  $(LIBS) $(OBJS) $(LDFLAGS)  $(LFLAGS)
.c.o:
	$(CC) $(INCLUDE) $(CFLAGS) -o $*.o $<
.cpp.o:
	$(GPP) $(INCLUDE) $(CFLAGS) -o $*.o $<


clean:
	rm -f *.o  $(PROJS) $(OBJS:.o=.d)
	
