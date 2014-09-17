#!/bin/sh

#  config.sh
#  config for ndsdk
#
#  Created by duan .
#

workDir=`pwd`


create_ndhome() {
    cd $HOME
    if [ -f .bash_profile ]; then
        echo "export NDHOME=\"$workDir\"" >> .bash_profile
    elif [ -f .profile ]; then
        echo "export NDHOME=\"$workDir\"" >> .profile
    else
        echo "could not create NDHOME "
        echo "PLEASE set evn $NDHOME "
        cd $workDir
        exit 1
    fi
    cd $workDir
}

# create env

if [ "x$NDHOME" == "x" ]; then
    create_ndhome
fi

# create output dir
PLATFORM_BITS=`getconf LONG_BIT`

[ -d bin ] || mkdir bin
[ -d lib ] || mkdir lib


ARCH_MACHINE=`uname -m`
OS_kernel=`uname -s`

WORKDIR="./bin/"$OS_kernel"_"$ARCH_MACHINE
LIBDIR="./lib/"$OS_kernel"_"$ARCH_MACHINE

[ -d  $WORKDIR ] || mkdir $WORKDIR
[ -d  $LIBDIR ] || mkdir $LIBDIR

echo "config SUCCESS "

if [ "x$NDHOME" == "x" ]; then
    echo "You need relogin and run make to install"
else
    echo "Run make to install"
fi


