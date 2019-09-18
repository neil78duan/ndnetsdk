	

home_dir = $(shell pwd)
MY_ARCH = $(shell uname -m)
MY_OS_kernel = $(shell uname -s | tr '[A-Z]' '[a-z]')
SUB_AIM_DIR = $(MY_OS_kernel)_$(MY_ARCH)
BIN_AIM = ./bin/$(SUB_AIM_DIR)


SUBDIRS = src demo

all: debug

debug:
	for n in $(SUBDIRS); do $(MAKE) -C $$n debug || exit 1; done

release:
	for n in $(SUBDIRS); do $(MAKE) -C $$n release || exit 1; done

dll: dll-debug

dll-debug:
	for n in $(SUBDIRS); do $(MAKE) -C $$n dll-debug || exit 1; done
dll-release:
	for n in $(SUBDIRS); do $(MAKE) -C $$n dll-release || exit 1; done


clean-dll:
	cd src ;make clean
	cd ./lib/$(SUB_AIM_DIR) ; rm *.so ; rm *.dylib

clean:
	for n in $(SUBDIRS); do $(MAKE) -C $$n clean ; done

update:
	git commit -m "auto commit " -a; git pull

commit:
	make update ; git push

install-dll:
	cd src ; make install  DEBUG="n" BUILD_DLL="y"
	
uninstall:
	cd src; make uninstall

checkthem:
	for n in $(SUBDIRS); do $(MAKE) -C $$n checkthem ; done

check:
	for n in $(SUBDIRS); do $(MAKE) -C $$n check; done

objs:
	for n in $(SUBDIRS); do $(MAKE) -C $$n objs; done

run:
	$(BIN_AIM)/srvDemo -f ./cfg/config.xml -c test_srv_config

config:
	chmod u+x ./config.sh ; ./config.sh

runtest:
	./bin/Test

