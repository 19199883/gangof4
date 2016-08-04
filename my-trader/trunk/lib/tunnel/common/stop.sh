#!/bin/bash

program_name="lvl1_tradsvc"
this_dir=""
log_file=""

# the following codes obtain the directory where this script file locates. 
 this_dir=`pwd`
 dirname $0|grep "^/" >/dev/null
 if [ $? -eq 0 ];then
	 this_dir=`dirname $0`
 else
	 dirname $0|grep "^\." >/dev/null
	 retval=$?
	 if [ $retval -eq 0 ];then
		 this_dir=`dirname $0|sed "s#^.#$this_dir#"`
	 else
		 this_dir=`dirname $0|sed "s#^#$this_dir/#"`
	 fi
 fi

cd $this_dir

# backup log
log_file=$program_name"_"`date +%y%m%d`.tar.bz2
tar -cvjf $log_file   ./log *.log
mkdir -p  ../backup/
mv -f $log_file ../backup/

rm -f *.log ./log/*

#kill process and exit
pkill -9  $program_name 
