# c test  Makefile 
# 2005-11-6 
# neil duan 

include $(NDHOME)/src/Rules.make

PROJS  = $(WORKDIR)/client_test
OBJS   := connector.o
SRC := connector.cpp

CFLAGS += -I$(TOPDIR)/include -DND_CLIENT_ONLY

MYLIBS := $(LIBOUTPUT)/lib$(CLIENT_LIB).a
LFLAGS += -L$(LIBOUTPUT) -l$(CLIENT_LIB)


all: $(PROJS) 
			
$(PROJS): $(OBJS)  $(MYLIBS)
	$(CPP) -o  $(PROJS)   $(OBJS)  $(LFLAGS)

.c.o:
	$(CC) $(INCLUDE) $(CFLAGS) -o $*.o $<
.cpp.o:
	$(CC) $(INCLUDE) $(CFLAGS) -o $*.o $<
	
clean:
	rm -f *.o  $(PROJS) $(OBJS:.o=.d)
	
