# c test  Makefile 
# 2005-11-6 
# neil duan 

include $(NDHOME)/src/Rules.make

MYLIBS := $(LIBOUTPUT)/lib$(SRV_LIB).a
LFLAGS += -L$(LIBOUTPUT) -l$(SRV_LIB)$(AIM_NAME)


PROJS  = $(WORKDIR)/srv_test

SRC := srv_test.cpp
OBJS   :=  srv_test.o

CFLAGS += -I$(TOPDIR)/include

all: $(PROJS) 
			
$(PROJS): $(OBJS)  $(MYLIBS)
	$(CPP) -o  $(PROJS)   $(OBJS)  $(LFLAGS)

.c.o:
	$(CC) $(INCLUDE) $(CFLAGS) -o $*.o $<
.cpp.o:
	$(CC) $(INCLUDE) $(CFLAGS) -o $*.o $<
	
clean:
	rm -f *.o  $(PROJS) $(OBJS:.o=.d)

