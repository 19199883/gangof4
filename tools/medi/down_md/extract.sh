#!/bin/bash

###################################################
#	main.
###################################################
# global varibales
COMM_NOS="SM SF"

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

dest="dest"
mkdir $dest

for file in `ls data_*tar.bz2`
do
	echo "data_file: ${file}"
	# e.g. data_170724.tar.bz2
	tar -xjf $file
	date_suffix=${file:5:6}
	echo $date_suffix 

	# e.g. czce_md_day_170724.tar.gz
	czce_file=data/czce_md_day_*${date_suffix}.tar.gz
	tar -xzf $czce_file

	for comm_no in $COMM_NOS
	do 
		czce_md_dir=backup/czce_md_day_${date_suffix}/
		mkt_data_file=${czce_md_dir}czce_level2_*_*${comm_no}
		echo ${mkt_data_file}
		domi_consr_file=`ls -S ${mkt_data_file}*([0-9]).dat | gawk '{if (FNR == 1) print $1}'`
		 if [ -n $domi_consr_file ]
		 then
			echo "file: ${domi_consr_file}"
			cp -a $domi_consr_file  $dest
		 fi
	 done

	 rm -rf data
	 rm -rf backup
done
