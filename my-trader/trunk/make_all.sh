#!/bin/bash

TOP=./
PROJECT=

PROJECT="utils"
cd "${TOP}src/MyExchange/${PROJECT}/"
make  distclean
if (make all)
then
	echo "${PROJECT} building succeeded"
	echo ${pwd}
else
	echo "${PROJECT} building failed"
	echo <&1
	return
fi
cd ../../../

PROJECT=tinyxml
cd "${TOP}src/MyExchange/${PROJECT}/"
make  distclean
if (make  all)
then
	echo "${PROJECT} building succeeded"
	echo ${pwd}
else
	echo "${PROJECT} building failed"
	echo <&1
	return
fi
cd ../../../

PROJECT=my_exchange_entity 
cd "${TOP}src/MyExchange/${PROJECT}/"
make  distclean
if (make  all)
then
	echo "${PROJECT} building succeeded"
	echo ${pwd}
else
	echo "${PROJECT} building failed"
	echo <&1
	return
fi
cd ../../../

PROJECT=my_exchange_db 
cd "${TOP}src/MyExchange/${PROJECT}/"
make  distclean
if (make  all)
then
	echo "${PROJECT} building succeeded"
	echo ${pwd}
else
	echo "${PROJECT} building failed"
	echo <&1
	return
fi
cd ../../../

PROJECT=sm 
cd "${TOP}src/MyExchange/${PROJECT}/"
make  distclean
if (make  all)
then
	echo "${PROJECT} building succeeded"
	echo ${pwd}
else
	echo "${PROJECT} building failed"
	echo <&1
	return
fi
cd ../../../

PROJECT=QA 
cd "${TOP}src/MyExchange/${PROJECT}/"
make  distclean
if (make  all)
then
	echo "${PROJECT} building succeeded"
	echo ${pwd}
else
	echo "${PROJECT} building failed"
	echo <&1
	return
fi
cd ../../../

PROJECT=tca 
cd "${TOP}src/MyExchange/${PROJECT}/"
make  distclean
if (make  all)
then
	echo "${PROJECT} building succeeded"
	echo ${pwd}
else
	echo "${PROJECT} building failed"
	echo <&1
	return
fi
cd ../../../

PROJECT=te 
cd "${TOP}src/MyExchange/${PROJECT}/"
make  distclean
if (make  all)
then
	echo "${PROJECT} building succeeded"
	echo ${pwd}
else
	echo "${PROJECT} building failed"
	echo <&1
	return
fi
cd ../../../

PROJECT=quote_forwarder_common 
cd "${TOP}src/MyExchange/${PROJECT}/"
make distclean
if (make all)
then
	echo "${PROJECT} building succeeded"
	echo ${pwd}
else
	echo "${PROJECT} building failed"
	echo <&1
	return
fi
cd ../../../

PROJECT=quote_forwarder_all_exchanges_level1 
cd "${TOP}src/MyExchange/${PROJECT}/"
echo ${MAKE_FILE}
make  distclean
if (make all)
then
	echo "${PROJECT} building succeeded"
	make dist
else
	echo "${PROJECT} building failed"
	echo <&1
	return
fi
cd ../../../

PROJECT=minictp_lv1
cd "${TOP}src/MyExchange/${PROJECT}/"
echo ${MAKE_FILE}
make  distclean
if (make all)
then
	echo "${PROJECT} building succeeded"
	make dist
else
	echo "${PROJECT} building failed"
	echo <&1
	return
fi
cd ../../../

PROJECT=quote_forwarder_all_level2 
cd "${TOP}src/MyExchange/${PROJECT}/"
echo ${MAKE_FILE}
make  distclean
if (make all)
then
	echo "${PROJECT} building succeeded"
	make dist
else
	echo "${PROJECT} building failed"
	echo <&1
	return
fi
cd ../../../

PROJECT=minictp_level2
cd "${TOP}src/MyExchange/${PROJECT}/"
echo ${MAKE_FILE}
make  distclean
if (make all)
then
	echo "${PROJECT} building succeeded"
	make dist
else
	echo "${PROJECT} building failed"
	echo <&1
	return
fi
cd ../../../



PROJECT=quote_forwarder_cffex_level2 
cd "${TOP}src/MyExchange/${PROJECT}/"
echo ${MAKE_FILE}
make  distclean
if (make all)
then
	echo "${PROJECT} building succeeded"
	make dist
else
	echo "${PROJECT} building failed"
	echo <&1
	return
fi
cd ../../../

PROJECT=quote_forwarder_czce_level2 
cd "${TOP}src/MyExchange/${PROJECT}/"
echo ${MAKE_FILE}
make  distclean
if (make all)
then
	echo "${PROJECT} building succeeded"
	make dist
else
	echo "${PROJECT} building failed"
	echo <&1
	return
fi
cd ../../../

PROJECT=quote_forwarder_ib
cd "${TOP}src/MyExchange/${PROJECT}/"
echo ${MAKE_FILE}
make  distclean
if (make all)
then
	echo "${PROJECT} building succeeded"
	make dist
else
	echo "${PROJECT} building failed"
	echo <&1
	return
fi
cd ../../../

PROJECT=quote_forwarder_lts_level2
cd "${TOP}src/MyExchange/${PROJECT}/"
echo ${MAKE_FILE}
make  distclean
if (make all)
then
	echo "${PROJECT} building succeeded"
	make dist
else
	echo "${PROJECT} building failed"
	echo <&1
	return
fi
cd ../../../

#PROJECT=quote_forwarder_lts_option
#cd "${TOP}src/MyExchange/${PROJECT}/"
#echo ${MAKE_FILE}
#make  distclean
#if (make all)
#then
#	echo "${PROJECT} building succeeded"
#	make dist
#else
#	echo "${PROJECT} building failed"
#	echo <&1
#	return
#fi
#cd ../../../
#
#PROJECT=quote_forwarder_sec_kmds
#cd "${TOP}src/MyExchange/${PROJECT}/"
#echo ${MAKE_FILE}
#make  distclean
#if (make all)
#then
#	echo "${PROJECT} building succeeded"
#	make dist
#else
#	echo "${PROJECT} building failed"
#	echo <&1
#	return
#fi
#cd ../../../
#
#PROJECT=quote_forwarder_sgit
#cd "${TOP}src/MyExchange/${PROJECT}/"
#echo ${MAKE_FILE}
#make  distclean
#if (make all)
#then
#	echo "${PROJECT} building succeeded"
#	make dist
#else
#	echo "${PROJECT} building failed"
#	echo <&1
#	return
#fi
#cd ../../../
#

#PROJECT=quote_forwarder_shfe 
#cd "${TOP}src/MyExchange/${PROJECT}/"
#echo ${MAKE_FILE}
#make  distclean
#if (make all)
#then
#	echo "${PROJECT} building succeeded"
#	make dist
#else
#	echo "${PROJECT} building failed"
#	echo <&1
#	return
#fi
#cd ../../../

PROJECT=quote_forwarder_shfe_jr 
cd "${TOP}src/MyExchange/${PROJECT}/"
echo ${MAKE_FILE}
make  distclean
if (make all)
then
	echo "${PROJECT} building succeeded"
	make dist
else
	echo "${PROJECT} building failed"
	echo <&1
	return
fi
cd ../../../


#PROJECT=quote_forwarder_shfe_xele 
#cd "${TOP}src/MyExchange/${PROJECT}/"
#echo ${MAKE_FILE}
#make  distclean
#if (make all)
#then
#	echo "${PROJECT} building succeeded"
#	make dist
#else
#	echo "${PROJECT} building failed"
#	echo <&1
#	return
#fi
#cd ../../../

#PROJECT=quote_forwarder_stock_tdf 
#cd "${TOP}src/MyExchange/${PROJECT}/"
#echo ${MAKE_FILE}
#make  distclean
#if (make all)
#then
#	echo "${PROJECT} building succeeded"
#	make dist
#else
#	echo "${PROJECT} building failed"
#	echo <&1
#	return
#fi
#cd ../../../
#
#
#PROJECT=quote_forwarder_tap
#cd "${TOP}src/MyExchange/${PROJECT}/"
#echo ${MAKE_FILE}
#make  distclean
#if (make all)
#then
#	echo "${PROJECT} building succeeded"
#	make dist
#else
#	echo "${PROJECT} building failed"
#	echo <&1
#	return
#fi
#cd ../../../

PROJECT=mytrader_cffexlevel2_general 
cd "${TOP}src/MyExchange/${PROJECT}/"
echo ${MAKE_FILE}
make  distclean
if (make all)
then
	echo "${PROJECT} building succeeded"
	make dist
else
	echo "${PROJECT} building failed"
	echo <&1
	return
fi
cd ../../../

PROJECT=lvl1_tradsvc
cd "${TOP}src/MyExchange/${PROJECT}/"
echo ${MAKE_FILE}
make  distclean
if (make all)
then
	echo "${PROJECT} building succeeded"
	make dist
else
	echo "${PROJECT} building failed"
	echo <&1
	return
fi
cd ../../../



PROJECT=mytrader_cf_shef_ctp 
cd "${TOP}src/MyExchange/${PROJECT}/"
echo ${MAKE_FILE}
make  distclean
if (make all)
then
	echo "${PROJECT} building succeeded"
	make dist
else
	echo "${PROJECT} building failed"
	echo <&1
	return
fi
cd ../../../

PROJECT=mytrader_cf_dcelevel2_xspeed 
cd "${TOP}src/MyExchange/${PROJECT}/"
echo ${MAKE_FILE}
make  distclean
if (make all)
then
	echo "${PROJECT} building succeeded"
	make dist
else
	echo "${PROJECT} building failed"
	echo <&1
	return
fi
cd ../../../

PROJECT=mytrader_czcelevel2_esunny 
cd "${TOP}src/MyExchange/${PROJECT}/"
echo ${MAKE_FILE}
make  distclean
if (make all)
then
	echo "${PROJECT} building succeeded"
	make dist
else
	echo "${PROJECT} building failed"
	echo <&1
	return
fi
cd ../../../

PROJECT=mytrader_stock_fut 
cd "${TOP}src/MyExchange/${PROJECT}/"
echo ${MAKE_FILE}
make  distclean
if (make all)
then
	echo "${PROJECT} building succeeded"
	make dist
else
	echo "${PROJECT} building failed"
	echo <&1
	return
fi
cd ../../../


