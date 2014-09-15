#!/bin/sh

srcDir=$1
aimFile=$2

workDir=`pwd`

list_file()
{
    cd $srcDir
    allFiles=`ls`

    for n in $allFiles; do
    echo "#include \"$n\"" > aimFile
    done
    cd $workDir
}

if [ ! -d $srcDir ]; then
    echo "$srcDir not exist"
    exit 1
fi


if [ ! -f $aimFile ]; then
    echo "aim file $aimFile not exist"
    exit 1
fi

list_file