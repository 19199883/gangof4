#!/bin/bash

all_makefiles=`find ./ -name Makefile`
echo $all_makefiles
for make_file in $all_makefiles
do 
	make_file_new=$make_file"_new"
	touch $make_file_new
	for ln in `cat $make_file`
	do
		echo "start"
		sed 's/DEBUG:=n/DEBUG:=y/g' $make_file  > $make_file_new
		echo $make_file_new
	done
done 


