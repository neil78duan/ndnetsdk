# c test  Makefile 
# 2005-11-6 
# neil duan 

include ../Rules.make

PROJS  = $(WORKDIR)/client_test

OBJS   := conn_test.o


CFLAGS += -I$(TOPDIR)/include -I$(TOPDIR)/nd_crypt/src

ifeq ($(DEBUG),y)
MYLIBS :=  $(LIBDIR)/libnd_srvcore_dbg.a  $(LIBDIR)/libnd_net_dbg.a $(LIBDIR)/libnd_crypt_dbg.a $(LIBDIR)/libnd_common_dbg.a
LFLAGS += -L$(LIBDIR) -lnd_cliapp_dbg -lnd_srvcore_dbg -lnd_net_dbg -lnd_crypt_dbg -lnd_common_dbg
else
MYLIBS := $(LIBDIR)/libnd_srvcore.a $(LIBDIR)/libnd_net.a $(LIBDIR)/libnd_crypt.a $(LIBDIR)/libnd_common.a
LFLAGS += -L$(LIBDIR) -lnd_cliapp -lnd_srvcore  -lnd_net -lnd_crypt -lnd_common 
endif

LFLAGS += -lpthread 


all: $(PROJS)
			
$(PROJS): $(OBJS) $(MYLIBS)
	$(CC) -o  $(PROJS)  $(LIBS) $(OBJS) $(LDFLAGS)  $(LFLAGS)
#	cp -f $(PROJS) $(WORKDIR)/
.c.o:
	$(CC) $(INCLUDE) $(CFLAGS) -o $*.o $<
.cpp.o:
	$(CPP) $(INCLUDE) $(CFLAGS) -o $*.o $<


clean:
	rm -f *.o  $(PROJS) $(OBJS:.o=.d)
	

###############create dependence file #############
.PHONY: clean


include $(OBJS:.o=.d)

$(OBJS:.o=.d): $(OBJS:.o=.c)
	set -e;\
	$(CC) -MM  $(INCLUDE) $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
