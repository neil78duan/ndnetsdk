
#topdir=(shell pwd)
VPATH = .

include $(NDHOME)/src/Rules.make

SUBDIRS =  srvDemo ndclient tool 


all: debug

debug:
	for n in $(SUBDIRS); do $(MAKE) -C $$n DEBUG="y" PROFILE="y" || exit 1; done

release:
	for n in $(SUBDIRS); do $(MAKE) -C $$n DEBUG="n" PROFILE="n" || exit 1; done

dll:
	echo "make dll success!"
dll-debug:
	echo "make dll-debug success!"
dll-release:
	echo "make dll-release success!"
	
clean:
	rm -f *.o *~
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

