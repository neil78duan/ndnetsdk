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


clean:
	rm -f *.o *~
	rm ../lib/*.a
	for n in $(SUBDIRS); do $(MAKE) -C $$n clean DEBUG="n" PROFILE="n"; done
	for n in $(SUBDIRS); do $(MAKE) -C $$n clean DEBUG="y" PROFILE="y" || exit 1; done

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

