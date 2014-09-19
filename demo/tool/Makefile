# c test  Makefile 
# 2005-11-6 
# neil duan 


include $(NDHOME)/src/Rules.make

MYLIBS := $(LIBOUTPUT)/lib$(SRV_LIB).a
LFLAGS += -L$(LIBOUTPUT) -l$(SRV_LIB)

#PROJ1  = $(WORKDIR)/proxy
PROJ2  = $(WORKDIR)/monitor

OBJS   := monitor.o 

CFLAGS += -I$(TOPDIR)/include -I$(TOPDIR)/nd_crypt/src


all: $(PROJ2)
			
#$(PROJ1): proxy_srv.o $(MYLIBS)
#	$(CPP) -o  $(PROJ1) proxy_srv.o $(LIBS)  $(LDFLAGS)  $(LFLAGS)
	
$(PROJ2): monitor.o $(MYLIBS)
	$(CPP) -o  $(PROJ2) monitor.o $(LIBS)  $(LDFLAGS)  $(LFLAGS) 

.c.o:
	$(CC) $(INCLUDE) $(CFLAGS) -o $*.o $<
.cpp.o:
	$(CPP) $(INCLUDE) $(CFLAGS) -o $*.o $<

clean:
	rm -f *.o  $(PROJ1) $(PROJ2) $(OBJS:.o=.d)
	