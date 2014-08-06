# c test  Makefile 
# 2005-11-6 
# neil duan 

include ../Rules.make

PROJ1  = $(WORKDIR)/proxy
PROJ2  = $(WORKDIR)/monitor

OBJS   := monitor.o proxy_srv.o

CFLAGS += -I$(TOPDIR)/include -I$(TOPDIR)/nd_crypt/src

ifeq ($(DEBUG),y)
MYLIBS := $(LIBDIR)/libnd_appcpp_dbg.a  $(LIBDIR)/libnd_app_dbg.a $(LIBDIR)/libnd_srvcore_dbg.a  $(LIBDIR)/libnd_net_dbg.a $(LIBDIR)/libnd_crypt_dbg.a $(LIBDIR)/libnd_common_dbg.a
LFLAGS += -L$(LIBDIR)  -lnd_appcpp_dbg  -lnd_app_dbg -lnd_srvcore_dbg -lnd_net_dbg -lnd_crypt_dbg -lnd_common_dbg
else
MYLIBS :=  $(LIBDIR)/libnd_appcpp.a  $(LIBDIR)/libnd_app.a $(LIBDIR)/libnd_srvcore.a $(LIBDIR)/libnd_net.a $(LIBDIR)/libnd_crypt.a $(LIBDIR)/libnd_common.a
LFLAGS += -L$(LIBDIR) -lnd_appcpp  -lnd_app -lnd_srvcore  -lnd_net -lnd_crypt -lnd_common 
endif

LFLAGS += -lpthread 


all: $(PROJ1)  $(PROJ2)
			
$(PROJ1): proxy_srv.o $(MYLIBS)
	$(CPP) -o  $(PROJ1) proxy_srv.o $(LIBS)  $(LDFLAGS)  $(LFLAGS)
	
$(PROJ2): monitor.o $(MYLIBS)
	$(CPP) -o  $(PROJ2) monitor.o $(LIBS)  $(LDFLAGS)  $(LFLAGS) 

.c.o:
	$(CC) $(INCLUDE) $(CFLAGS) -o $*.o $<
.cpp.o:
	$(CPP) $(INCLUDE) $(CFLAGS) -o $*.o $<

clean:
	rm -f *.o  $(PROJ1) $(PROJ2) $(OBJS:.o=.d)
	