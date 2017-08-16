#!/bin/bash

DEST="data"

# the directory where this script file is
function enter_cur_dir(){
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
}

function download(){
	if [ ! -d $DEST ] 
	then
		mkdir $DEST 
	fi

	SRC_STRA_LOG="/home/u910019/${TRADE_GROUP}/${DAY_NGT}/backup/${EXT}_stra_${SUFFIX}_`date +%y%m%d`.tar.${COMPRESS}"
	SRC_TICK_DATA="/home/u910019/md/download/${DAY_NGT}/backup/${EXT}_md_${DAY_NGT}_`date +%y%m%d`.tar.${COMPRESS}"

	scp  -P ${host_port} "${host_user}@${host_ip}:${SRC_STRA_LOG}" ${DEST}
	scp  -P ${host_port} "${host_user}@${host_ip}:${SRC_TICK_DATA}" ${DEST}
}


function download_zp(){
	if [ ! -d $DEST ] 
	then
		mkdir $DEST 
	fi

	SRC_STRA_LOG="~/${DAY_NGT}/backup/${EXT}_stra_${SUFFIX}_`date +%y%m%d`.tar.${COMPRESS}"

	scp  -P ${host_port} "${host_user}@${host_ip}:${SRC_STRA_LOG}" ${DEST}
}

enter_cur_dir

# download data form shfe server for medi
host_ip="101.230.197.62"
host_user="u910019"
host_port="8001"
SUFFIX="day"
DAY_NGT="day"
EXT="sh"
TRADE_GROUP="medi"
COMPRESS="bz2"
download
SRC="${DEST}/${EXT}_stra_${SUFFIX}_`date +%y%m%d`.tar.${COMPRESS}"
tar -C ${DEST} -xvjf ${SRC}
rm -r ${SRC}

host_ip="101.230.197.62"
host_user="u113169"
host_port="8001"
SUFFIX="day"
DAY_NGT="day"
EXT="sh"
COMPRESS="bz2"
download_zp
SRC_ZP="${DEST}/${EXT}_stra_${SUFFIX}_`date +%y%m%d`.tar.${COMPRESS}"
tar -C $DEST -xvjf ${SRC_ZP}
rm -r ${SRC_ZP}

tar -cvjf $SRC_ZP -C $DEST "./backup"
rm -r "${DEST}/backup"

# download data form shfe server for medi
host_ip="101.230.197.62"
host_user="u910019"
host_port="8001"
EXT="sh"
TRADE_GROUP="medi"
COMPRESS="bz2"
SUFFIX="night"
DAY_NGT="night"
download
SRC="${DEST}/${EXT}_stra_${SUFFIX}_`date +%y%m%d`.tar.${COMPRESS}"
tar -C $DEST -xvjf ${SRC}
rm -r $SRC

host_ip="101.230.197.62"
host_user="u113169"
host_port="8001"
EXT="sh"
COMPRESS="bz2"
SUFFIX="night"
DAY_NGT="night"
download_zp
SRC_ZP="${DEST}/${EXT}_stra_${SUFFIX}_`date +%y%m%d`.tar.${COMPRESS}"
tar -C $DEST -xvjf $SRC_ZP
rm -r $SRC_ZP
tar -cvjf $SRC_ZP -C $DEST "./backup"
rm -r "${DEST}/backup"

# download data form dce server for medi
host_ip="101.231.3.117"
host_user="u910019"
host_port="44152"
SUFFIX="day"
DAY_NGT="day"
EXT="dce"
TRADE_GROUP="medi"
COMPRESS="gz"
download
# download data form dce server for medi
SUFFIX="ngt"
DAY_NGT="night"
download

# download data form zce server for medi
host_ip="123.149.20.60"
host_user="u910019"
host_port="8008"
SUFFIX="day"
DAY_NGT="day"
EXT="czce"
TRADE_GROUP="medi"
COMPRESS="gz"
download

SRC="${DEST}/${EXT}_stra_${SUFFIX}_`date +%y%m%d`.tar.${COMPRESS}"
tar -C ${DEST} -xvzf ${SRC}
rm -r ${SRC}

host_ip="123.149.20.60"
host_user="u910223"
host_port="8008"
SUFFIX="day"
DAY_NGT="day"
EXT="czce"
COMPRESS="gz"
download_zp
SRC_ZP="${DEST}/${EXT}_stra_${SUFFIX}_`date +%y%m%d`.tar.${COMPRESS}"
tar -C $DEST -xvzf ${SRC_ZP}
rm -r ${SRC_ZP}

tar -cvjf $SRC_ZP -C $DEST "./backup"
rm -r "${DEST}/backup"

# download data form dce server for medi
host_ip="123.149.20.60"
host_user="u910019"
host_port="8008"
EXT="czce"
TRADE_GROUP="medi"
COMPRESS="gz"
SUFFIX="ngt"
DAY_NGT="night"
download

SRC="${DEST}/${EXT}_stra_${SUFFIX}_`date +%y%m%d`.tar.${COMPRESS}"
tar -C $DEST -xvzf ${SRC}
rm -r $SRC

host_ip="123.149.20.60"
host_user="u910223"
host_port="8008"
EXT="czce"
COMPRESS="gz"
SUFFIX="ngt"
DAY_NGT="night"
download_zp
SRC_ZP="${DEST}/${EXT}_stra_${SUFFIX}_`date +%y%m%d`.tar.${COMPRESS}"
tar -C $DEST -xvzf $SRC_ZP
rm -r $SRC_ZP
tar -cvjf $SRC_ZP -C $DEST "./backup"
rm -r "${DEST}/backup"



tar -cjf "data_`date +%y%m%d`.tar.bz2" ./data	
rm -r data
