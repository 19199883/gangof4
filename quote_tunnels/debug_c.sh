#!/bin/bash

all_makefiles=`find ./ -name Makefile`
echo $all_makefiles
for make_file in $all_makefiles
do 
	make_file_new=$make_file"_new"
	touch $make_file_new
	echo "start processing $make_file"	
	for ln in `cat $make_file`
	do	
		sed 's/-lboost_thread/ /g' $make_file  > $make_file_new
	done
	
	mv $make_file_new $make_file

done 


