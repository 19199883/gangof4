#!/bin/bash

md_name="shfe_jr_mktsvc"
t_name="sh_tra_day"
md_file=$md_name"_"`date +%y%m%d`.tar.bz2
log_file=$t_name"_"`date +%y%m%d`.tar.bz2

cd /home/oliver/data/shfe_backup/day
sshpass -p "u910019explore" scp -r -P 8001 u910019@101.230.197.62:/home/u910019/trade/day/backup/$md_file ./
sshpass -p "u910019explore" scp -r -P 8001 u910019@101.230.197.62:/home/u910019/trade/day/backup/$log_file ./

# unzip and split
tar xjf $md_file
process_date=`date +%y%m%d`
mkdir -p data_split
my_md_file="my_shfe_md_20"$process_date.dat
mv Data/$my_md_file data_split/
rm -rf Data
rm *.log
cd data_split
./md_split $my_md_file rb1701 ag1612 bu1612 ni1701 au1612 ru1701 zn1610 zn1611 cu1610 cu1611 hc1610 al1610 al1611 pb1610

dest_md_file="shfe_md_all_20"$process_date.tar.bz2
tar cjf $dest_md_file $my_md_file
rm $my_md_file

dest_md_file="shfe_md_20"$process_date.tar.bz2
tar cjf $dest_md_file *.dat
rm *.dat
