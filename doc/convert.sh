#!/bin/sh

name=$1
dir=`echo $name | sed -e "s/\/[^\/]*$/\//"`
base1=`basename $name .3`
base2=`basename $base1 .4`
newname=`echo $dir$base2`
echo $newname
nroff -man $name | uniq > $newname
rm $name
