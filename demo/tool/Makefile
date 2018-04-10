# c test  Makefile 
# 2005-11-6 
# neil duan 


include $(NDHOME)/src/Rules.make

MYLIBS := $(LIBOUTPUT)/lib$(COMMON_LIB).a
LFLAGS += -L$(LIBOUTPUT) -l$(COMMON_LIB)

PROJ1  = $(WORKDIR)/rsa_keygen
PROJ2  = $(WORKDIR)/monitor
PROJ3  = $(WORKDIR)/ndxmlset

OBJS   := monitor.o 

CFLAGS += -I$(TOPDIR)/include -I$(TOPDIR)/nd_crypt/src


all: $(PROJ1) $(PROJ2) $(PROJ3)
			
$(PROJ1): rsa_keygen.o $(MYLIBS)
	$(CPP) -o  $(PROJ1) rsa_keygen.o $(LIBS)  $(LDFLAGS)  $(LFLAGS)
	
$(PROJ2): monitor.o $(MYLIBS)
	$(CPP) -o  $(PROJ2) monitor.o $(LIBS)  $(LDFLAGS)  $(LFLAGS) 

$(PROJ3): ndxml_set.o $(MYLIBS)
	$(CPP) -o  $(PROJ3) ndxml_set.o $(LIBS)  $(LDFLAGS)  $(LFLAGS)

.c.o:
	$(CC) $(INCLUDE) $(CFLAGS) -o $*.o $<
.cpp.o:
	$(CPP) $(INCLUDE) $(CFLAGS) -o $*.o $<

clean:
	rm -f *.o  $(PROJ1) $(PROJ2) $(PROJ3) $(OBJS:.o=.d)
	
