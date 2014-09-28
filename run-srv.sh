#!/bin/sh

ARCH_MACHINE=`uname -m`
OS_kernel=`uname -s |  tr '[A-Z]' '[a-z]'`

WORKDIR="./bin"$OS_kernel"_"$ARCH_MACHINE


$WORKDIR/srvDemo -f  ./cfg/config.xml -c test_srv_config
