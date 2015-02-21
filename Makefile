

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
	
clean:
	for n in $(SUBDIRS); do $(MAKE) -C $$n clean ; done

checkthem:
	for n in $(SUBDIRS); do $(MAKE) -C $$n checkthem ; done

check:
	for n in $(SUBDIRS); do $(MAKE) -C $$n check; done

objs:
	for n in $(SUBDIRS); do $(MAKE) -C $$n objs; done

run:
	$(BIN_AIM)/srvDemo -f ./cfg/config.xml -c test_srv_config

run-cli:
	$(BIN_AIM)/client_test localhost 7828 1

config:
	./config.sh

runtest:
	./bin/Test

