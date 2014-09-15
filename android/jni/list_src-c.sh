#!/bin/sh


workDir=`pwd`

list_file()
{
    if [ -d $1 ]; then
        cd $1
        allFiles=`ls`
        cd $workDir
#echo "// include $1 module "
        for n in $allFiles; do
            echo "      $1$n \\"
        done
    fi
#cd $srcDir
#    allFiles=`ls`

#    for n in $allFiles; do
#echo "#include \"$n\"" #> $aimFile
#    done
#    cd $workDir
}



for i in $* ; do
    list_file $i
done