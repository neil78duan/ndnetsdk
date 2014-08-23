
topdir=(shell pwd)
VPATH = .

include ./Rules.make

SUBDIRS =  nd_common nd_crypt nd_net nd_srvcore ndapplib

all: subdirs

subdirs:
	for n in $(SUBDIRS); do $(MAKE) -C $$n || exit 1; done

cleangame:
	rm -f *. 
	for n in $(SUBDIRS); do $(MAKE) -C $$n clean; done
	
clean:
	rm -f *.o *~
	for n in $(SUBDIRS); do $(MAKE) -C $$n clean; done

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

