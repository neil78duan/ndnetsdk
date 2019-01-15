#!/bin/sh

#  config.sh
#  config for ndsdk
#
#  Created by duan .
#

workDir=`pwd`


create_ndhome() {
    cd $HOME
    
    if [ -f .profile ]; then
        echo "export NDHOME=\"$workDir\"" >> .profile
        source $HOME/.profile
	elif [ -f .bash_profile ]; then
        echo "export NDHOME=\"$workDir\"" >> .bash_profile
        source $HOME/.bash_profile
    else
        echo "could not create NDHOME "
        echo "PLEASE set evn $NDHOME "
        cd $workDir
        exit 1
    fi
    cd $workDir
}

# create env

if [ -z "$NDHOME" ]; then
    create_ndhome
fi

# create output dir
PLATFORM_BITS=`getconf LONG_BIT`

[ -d bin ] || mkdir bin
[ -d lib ] || mkdir lib


ARCH_MACHINE=`uname -m`
OS_kernel=`uname -s |  tr '[A-Z]' '[a-z]'`

WORKDIR="./bin/"$OS_kernel"_"$ARCH_MACHINE
LIBDIR="./lib/"$OS_kernel"_"$ARCH_MACHINE

[ -d  $WORKDIR ] || mkdir $WORKDIR
[ -d  $LIBDIR ] || mkdir $LIBDIR

echo "config SUCCESS "


