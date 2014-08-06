# c test  Makefile 
# 2005-11-6 
# neil duan 

include ../Rules.make

PROJS  = $(WORKDIR)/srv_test

SRC := srv_test.cpp
OBJS   :=  srv_test.o

CFLAGS += -I$(TOPDIR)/include

ifeq ($(DEBUG),y)
MYLIBS := $(LIBDIR)/libnd_appcpp_dbg.a $(LIBDIR)/libnd_app_dbg.a $(LIBDIR)/libnd_srvcore_dbg.a \
	$(LIBDIR)/libnd_net_dbg.a $(LIBDIR)/libnd_crypt_dbg.a $(LIBDIR)/libnd_common_dbg.a
LFLAGS += -L$(LIBDIR) -lnd_appcpp_dbg -lnd_app_dbg -lnd_srvcore_dbg  -lnd_net_dbg \
	-lnd_crypt_dbg -lnd_common_dbg 
else
MYLIBS := $(LIBDIR)/libnd_appcpp.a $(LIBDIR)/libnd_app.a $(LIBDIR)/libnd_srvcore.a \
	$(LIBDIR)/libnd_net.a $(LIBDIR)/libnd_crypt.a $(LIBDIR)/libnd_common.a 
LFLAGS += -L$(LIBDIR) -lnd_appcpp -lnd_app -lnd_srvcore -lnd_net -lnd_crypt -lnd_common 
endif


all: $(PROJS) 
			
$(PROJS): $(OBJS)  $(MYLIBS)
	$(CPP) -o  $(PROJS)   $(OBJS)  $(LFLAGS)

.c.o:
	$(CC) $(INCLUDE) $(CFLAGS) -o $*.o $<
.cpp.o:
	$(CC) $(INCLUDE) $(CFLAGS) -o $*.o $<
	
clean:
	rm -f *.o  $(PROJS) $(OBJS:.o=.d)
	

###############create dependence file #############
.PHONY: clean


include $(OBJS:.o=.d)

$(OBJS:.o=.d): $(SRC)
	set -e;\
	$(CC) -MM  $(INCLUDE) $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

