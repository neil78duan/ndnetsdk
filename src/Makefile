VPATH = .

include ./Rules.make


SUBDIRS =  nd_common nd_crypt nd_net nd_vm nd_srvcore ndapplib ndclient


all: debug

debug:
	for n in $(SUBDIRS); do $(MAKE) -C $$n DEBUG="y" PROFILE="y" || exit 1; done
	cp -f $(TOPDIR)/src/build-lib $(LIBDIR)/Makefile; cd $(LIBDIR) ; make DEBUG="y" PROFILE="y" 

release:
	for n in $(SUBDIRS); do $(MAKE) -C $$n DEBUG="n" PROFILE="n" || exit 1; done
	cp -f $(TOPDIR)/src/build-lib $(LIBDIR)/Makefile; cd $(LIBDIR) ; make DEBUG="n" PROFILE="n"

dll : dll_debug
dll_debug:
	for n in $(SUBDIRS); do $(MAKE) -C $$n DEBUG="y" PROFILE="y" BUILD_DLL="y" || exit 1; done
	cp -f $(TOPDIR)/src/build-lib $(LIBDIR)/Makefile; cd $(LIBDIR) ; make DEBUG="y" PROFILE="y" BUILD_DLL="y" 

dll_release:
	for n in $(SUBDIRS); do $(MAKE) -C $$n DEBUG="n" PROFILE="n" BUILD_DLL="y" || exit 1; done
	cp -f $(TOPDIR)/src/build-lib $(LIBDIR)/Makefile; cd $(LIBDIR) ; make DEBUG="n" PROFILE="n" BUILD_DLL="y" 

clean:
	rm -f *.o *~
	rm -f ../lib/*.a  ../lib/*.so
	rm -f ../lib/$(AIM_NAME)/*.a ../lib/$(AIM_NAME)/*.so
	for n in $(SUBDIRS); do $(MAKE) -C $$n clean ; done

checkthem:
	for n in $(SUBDIRS); do $(MAKE) -C $$n checkthem; done

check:
	for n in $(SUBDIRS); do $(MAKE) -C $$n check; done

objs:
	for n in $(SUBDIRS); do $(MAKE) -C $$n objs; done

run:
	./bin/srv_test

runtest:
	./bin/Test

