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

if [ $PLATFORM_BITS -eq 64 ]; then
    [ -d bin64 ] || mkdir bin64
    [ -d lib64 ] || mkdir lib64
else
    [ -d bin ] || mkdir bin
    [ -d lib ] || mkdir lib
fi

echo "config SUCCESS "

if [ "x$NDHOME" == "x" ]; then
    echo "You need relogin and run make to install"
else
    echo "Run make to install"
fi


