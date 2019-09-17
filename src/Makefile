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

dll : dll-release
dll-debug:
	for n in $(SUBDIRS); do $(MAKE) -C $$n DEBUG="y" PROFILE="y" BUILD_DLL="y" || exit 1; done
	cp -f $(TOPDIR)/src/build-lib $(LIBDIR)/Makefile; cd $(LIBDIR) ; make DEBUG="y" PROFILE="y" BUILD_DLL="y" 

dll-release:
	for n in $(SUBDIRS); do $(MAKE) -C $$n DEBUG="n" PROFILE="n" BUILD_DLL="y" || exit 1; done
	cp -f $(TOPDIR)/src/build-lib $(LIBDIR)/Makefile; cd $(LIBDIR) ; make DEBUG="n" PROFILE="n" BUILD_DLL="y" 

clean-dll:
	for n in $(SUBDIRS); do $(MAKE) -C $$n clean-dll ; done
	
clean:
	rm -f *.o *~
	for n in $(SUBDIRS); do $(MAKE) -C $$n clean ; done

install:
	cp -f $(ND_CLIENT_DLL) $(SYS_INSTALL_PATH)
	cp -f $(ND_SRV_DLL) $(SYS_INSTALL_PATH)
	cp -f $(ND_COMMON_DLL) $(SYS_INSTALL_PATH)

uninstall:
	rm -f ../lib/*.a  ../lib/*.$(DLL_EXT_NAME)
	rm -f ../lib/$(AIM_NAME)/*.a ../lib/$(AIM_NAME)/*.$(DLL_EXT_NAME)

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

