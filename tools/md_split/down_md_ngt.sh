#!/bin/bash

md_name="shfe_mktdata"
t_name="sh_tra_night"
md_file=$md_name"_"`date +%y%m%d`.tar.bz2
log_file=$t_name"_"`date +%y%m%d`.tar.bz2

cd /home/oliver/data/shfe_backup/ngt
sshpass -p "u910019explore" scp -r -P 8001 u910019@101.230.197.62:/home/u910019/trade/night/backup/$md_file ./
sshpass -p "u910019explore" scp -r -P 8001 u910019@101.230.197.62:/home/u910019/trade/night/backup/$log_file ./
