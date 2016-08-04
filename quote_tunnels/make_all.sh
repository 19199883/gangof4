#!/bin/bash

# set trader root dir (行情、通道编译结构和配置文件的发布目录)
TRADER_ROOT=/home/wangying/github/gangof4/my-trader
TRADER_LIB_ROOT=$TRADER_ROOT/trunk/lib

#os define (don't need copy ssl in CentOS)
OS_DEBIAN=debian
OS_CENTOS=centos
OS_TYPE=$OS_DEBIAN

# quote tunnel source dir (行情、通道源码的根目录路径)
QT_SOURCE_ROOT=/home/wangying/github/gangof4/quote_tunnels

# quote tunnel source dir (行情、通道使用的库的路径，需要一起发布到trader)
QT_LIB_BIN_DIR=$QT_SOURCE_ROOT/lib/bin

cd $QT_SOURCE_ROOT

#copy include header files
#my_common header files
QT_CMN_INCLUDE=$TRADER_LIB_ROOT/my_common
mkdir -p $QT_CMN_INCLUDE
cp -a my_common/include/*.h $QT_CMN_INCLUDE

#tunnel header files
QT_TNL_INCLUDE=$TRADER_LIB_ROOT/tunnel/include/
mkdir -p $QT_TNL_INCLUDE
cp -a include_tunnel/*.h $QT_TNL_INCLUDE

#quote header files
QT_QUOTE_INCLUDE=$TRADER_LIB_ROOT/quote/include/
mkdir -p $QT_QUOTE_INCLUDE
cp -a $QT_SOURCE_ROOT/include_quote/include_level1/*.h $QT_QUOTE_INCLUDE
cp -a $QT_SOURCE_ROOT/include_quote/include_all_level2/*.h $QT_QUOTE_INCLUDE
cp -a $QT_SOURCE_ROOT/include_quote/include_cffex_level2/*.h $QT_QUOTE_INCLUDE
cp -a $QT_SOURCE_ROOT/include_quote/include_czce_level2/*.h $QT_QUOTE_INCLUDE
cp -a $QT_SOURCE_ROOT/include_quote/include_dce_level2/*.h $QT_QUOTE_INCLUDE
cp -a $QT_SOURCE_ROOT/include_quote/include_shfe_my/*.h $QT_QUOTE_INCLUDE
cp -a $QT_SOURCE_ROOT/include_quote/include_ib/*.h $QT_QUOTE_INCLUDE
cp -a $QT_SOURCE_ROOT/include_quote/include_stock/*.h $QT_QUOTE_INCLUDE
cp -a $QT_SOURCE_ROOT/include_quote/include_sgit/*.h $QT_QUOTE_INCLUDE
cp -a $QT_SOURCE_ROOT/include_quote/include_tap/*.h $QT_QUOTE_INCLUDE
cp -a $QT_SOURCE_ROOT/include_quote/include_datasource/*.h $QT_QUOTE_INCLUDE

#make and copy common so
QT_COMMON=$TRADER_LIB_ROOT/common
mkdir -p $QT_COMMON
cp -a $QT_LIB_BIN_DIR/my/libqtm* $QT_COMMON
cp -a $QT_LIB_BIN_DIR/my/libmymarketdata.so $QT_COMMON
cp -a $QT_LIB_BIN_DIR/my/libtimeprober.so $QT_COMMON
cp -a $QT_LIB_BIN_DIR/my/libtoe_hijack.so $QT_COMMON

if [ "$OS_TYPE" = "$OS_DEBIAN" ]; then
	cp -a $QT_LIB_BIN_DIR/libssl* $QT_COMMON
fi

cd $QT_SOURCE_ROOT/my_common
make clean
make
cp -a bin/*.so $QT_COMMON

cd $QT_SOURCE_ROOT/my_compliance_checker
make clean
make
cp -a bin/*.so $QT_COMMON

cd $QT_SOURCE_ROOT/my_exchange_fut_op
sed -i 's;ASYNC_CANCEL:=y;ASYNC_CANCEL:=n;' Makefile
make clean
make
sed -i 's;ASYNC_CANCEL:=n;ASYNC_CANCEL:=y;' Makefile
make clean
make
sed -i 's;ASYNC_CANCEL:=y;ASYNC_CANCEL:=n;' Makefile
cp -a bin/*.so $QT_COMMON

cd $QT_SOURCE_ROOT/my_exchange_stock
make clean
make
cp -a bin/*.so $QT_COMMON

#my_tunnel_lib_citics
cd $QT_SOURCE_ROOT/my_tunnel_lib_citics
make clean
make
QT_TUNNEL_DIST=$TRADER_LIB_ROOT/tunnel/citics
mkdir -p $QT_TUNNEL_DIST
cp -a bin/*.so $QT_TUNNEL_DIST
cp -a config/* $QT_TUNNEL_DIST
cp -a $QT_LIB_BIN_DIR/citics_hs/libCITICs_HsT2Hlp.so $QT_TUNNEL_DIST
cp -a $QT_LIB_BIN_DIR/citics_hs/libt2sdk64.so $QT_TUNNEL_DIST

#ctp tunnel
cd $QT_SOURCE_ROOT/my_tunnel_lib_ctp
make clean
make
QT_TUNNEL_DIST=$TRADER_LIB_ROOT/tunnel/ctp
mkdir -p $QT_TUNNEL_DIST
cp -a bin/*.so $QT_TUNNEL_DIST
cp -a config/* $QT_TUNNEL_DIST
cp -a $QT_LIB_BIN_DIR/ctp/libthosttraderapi.so $QT_TUNNEL_DIST

#minictp tunnel
cd $QT_SOURCE_ROOT/my_tunnel_lib_minictp
make clean
make
QT_TUNNEL_DIST=$TRADER_LIB_ROOT/tunnel/minictp
mkdir -p $QT_TUNNEL_DIST
cp -a bin/*.so $QT_TUNNEL_DIST
cp -a config/* $QT_TUNNEL_DIST
cp -a $QT_LIB_BIN_DIR/minictp/*.so $QT_TUNNEL_DIST

#ctp  test tunnel (for option and market making)
cd $QT_SOURCE_ROOT/my_tunnel_lib_ctp_mm
make clean
make
QT_TUNNEL_DIST=$TRADER_LIB_ROOT/tunnel/ctp_mm
mkdir -p $QT_TUNNEL_DIST
cp -a bin/*.so $QT_TUNNEL_DIST
cp -a config/* $QT_TUNNEL_DIST
cp -a $QT_LIB_BIN_DIR/ctp_20151215/libthosttraderapi.so $QT_TUNNEL_DIST
cp -a $QT_LIB_BIN_DIR/ctp_20151215/libthostmduserapi.so $QT_TUNNEL_DIST

# esunny tunnel
cd $QT_SOURCE_ROOT/my_tunnel_lib_esunny
make clean
make
QT_TUNNEL_DIST=$TRADER_LIB_ROOT/tunnel/esunny
mkdir -p $QT_TUNNEL_DIST
cp -a bin/*.so $QT_TUNNEL_DIST
cp -a config/* $QT_TUNNEL_DIST
cp -a $QT_LIB_BIN_DIR/esunny_20150320/libTapTradeAPI.so $QT_TUNNEL_DIST

# esunny test tunnel (for option and market making)
cd $QT_SOURCE_ROOT/my_tunnel_lib_esunny_opt
make clean
make
QT_TUNNEL_DIST=$TRADER_LIB_ROOT/tunnel/esunny_opt
mkdir -p $QT_TUNNEL_DIST
cp -a bin/*.so $QT_TUNNEL_DIST
cp -a config/* $QT_TUNNEL_DIST
cp -a $QT_LIB_BIN_DIR/esunny/libTapTradeAPI.so $QT_TUNNEL_DIST

# femas tunnel
cd $QT_SOURCE_ROOT/my_tunnel_lib_femas
make clean
make
QT_TUNNEL_DIST=$TRADER_LIB_ROOT/tunnel/femas
mkdir -p $QT_TUNNEL_DIST
cp -a bin/*.so $QT_TUNNEL_DIST
cp -a config/* $QT_TUNNEL_DIST
cp -a $QT_LIB_BIN_DIR/femas/libUSTPtraderapi.so $QT_TUNNEL_DIST

# ib api tunnel
cd $QT_SOURCE_ROOT/my_tunnel_lib_ib_api
make clean
make
QT_TUNNEL_DIST=$TRADER_LIB_ROOT/tunnel/ib_api
mkdir -p $QT_TUNNEL_DIST
cp -a bin/*.so $QT_TUNNEL_DIST
cp -a config/* $QT_TUNNEL_DIST

# kgi fix tunnel
cd $QT_SOURCE_ROOT/my_tunnel_lib_kgi_fix
make clean
make
QT_TUNNEL_DIST=$TRADER_LIB_ROOT/tunnel/kgi_fix
mkdir -p $QT_TUNNEL_DIST
cp -a bin/*.so $QT_TUNNEL_DIST
cp -a config/* $QT_TUNNEL_DIST
cp -a $QT_LIB_BIN_DIR/fix8/lib* $QT_TUNNEL_DIST

# lts tunnel
cd $QT_SOURCE_ROOT/my_tunnel_lib_lts
sed -i 's;CRACK:=n;CRACK:=y;' Makefile
make clean
make
sed -i 's;CRACK:=y;CRACK:=n;' Makefile
make clean
make
sed -i 's;CRACK:=n;CRACK:=y;' Makefile
QT_TUNNEL_DIST=$TRADER_LIB_ROOT/tunnel/lts
mkdir -p $QT_TUNNEL_DIST
cp -a bin/*.so $QT_TUNNEL_DIST
cp -a config/* $QT_TUNNEL_DIST
cp -a $QT_LIB_BIN_DIR/lts/lib* $QT_TUNNEL_DIST

# rem tunnel
cd $QT_SOURCE_ROOT/my_tunnel_lib_rem
make clean
make
QT_TUNNEL_DIST=$TRADER_LIB_ROOT/tunnel/rem
mkdir -p $QT_TUNNEL_DIST
cp -a bin/*.so $QT_TUNNEL_DIST
cp -a config/* $QT_TUNNEL_DIST
cp -a $QT_LIB_BIN_DIR/shengli/libEESTraderApi.so $QT_TUNNEL_DIST

# sgit tunnel
cd $QT_SOURCE_ROOT/my_tunnel_lib_sgit
make clean
make
QT_TUNNEL_DIST=$TRADER_LIB_ROOT/tunnel/sgit
mkdir -p $QT_TUNNEL_DIST
cp -a bin/*.so $QT_TUNNEL_DIST
cp -a config/* $QT_TUNNEL_DIST
cp -a $QT_LIB_BIN_DIR/sgit4.1/libsgittradeapi.so $QT_TUNNEL_DIST

# xele tunnel
cd $QT_SOURCE_ROOT/my_tunnel_lib_xele
make clean
make
QT_TUNNEL_DIST=$TRADER_LIB_ROOT/tunnel/xele
mkdir -p $QT_TUNNEL_DIST
cp -a bin/*.so $QT_TUNNEL_DIST
cp -a config/* $QT_TUNNEL_DIST
cp -a $QT_LIB_BIN_DIR/ac_xele_trade/libXeleTdAPI64.so $QT_TUNNEL_DIST

# xspeed tunnel (future)
cd $QT_SOURCE_ROOT/my_tunnel_lib_xspeed_fut
make clean
make
QT_TUNNEL_DIST=$TRADER_LIB_ROOT/tunnel/xspeed_fut
mkdir -p $QT_TUNNEL_DIST
cp -a bin/*.so $QT_TUNNEL_DIST
cp -a config/* $QT_TUNNEL_DIST
cp -a $QT_LIB_BIN_DIR/xspeed_fut/libDFITCTraderApi.so $QT_TUNNEL_DIST

# xspeed test tunnel (for option and market making)
cd $QT_SOURCE_ROOT/my_tunnel_lib_xspeed_mm
make clean
make
QT_TUNNEL_DIST=$TRADER_LIB_ROOT/tunnel/xspeed_mm
mkdir -p $QT_TUNNEL_DIST
cp -a bin/*.so $QT_TUNNEL_DIST
cp -a config/* $QT_TUNNEL_DIST
cp -a $QT_LIB_BIN_DIR/xspeed_mm/libDFITCTraderApi.so $QT_TUNNEL_DIST
cp -a $QT_LIB_BIN_DIR/xspeed_mm/libDFITCMdApi.so $QT_TUNNEL_DIST

# zeusing tunnel
cd $QT_SOURCE_ROOT/my_tunnel_lib_zeusing
make clean
make
QT_TUNNEL_DIST=$TRADER_LIB_ROOT/tunnel/zeusing
mkdir -p $QT_TUNNEL_DIST
cp -a bin/*.so $QT_TUNNEL_DIST
cp -a config/* $QT_TUNNEL_DIST
cp -a $QT_LIB_BIN_DIR/zeusing/libzeusingtraderapi.so $QT_TUNNEL_DIST

# quote: cffex level2
cd $QT_SOURCE_ROOT/my_quote_cffex_level2_lib
QT_QUOTE_DIST=$TRADER_LIB_ROOT/quote/cffex_level2
make clean
make
mkdir -p $QT_QUOTE_DIST/config
mkdir -p $QT_QUOTE_DIST/bin
cp -a config/* $QT_QUOTE_DIST/config
cp -a bin/*.so $QT_QUOTE_DIST/bin
cp -a $QT_LIB_BIN_DIR/ac_xele_md_r839/libac_xele_md.so $QT_QUOTE_DIST/bin
cp -a $QT_LIB_BIN_DIR/ac_xele_md_r839/libXeleMdAPI64.a $QT_QUOTE_DIST/bin
cp -a $QT_LIB_BIN_DIR/femas_datafeed/libUSTPmduserapi_lnx64.so $QT_QUOTE_DIST/bin

# quote: ctp_level1
cd $QT_SOURCE_ROOT/my_quote_ctp_level1_lib
QT_QUOTE_DIST=$TRADER_LIB_ROOT/quote/ctp_level1
make clean
make
mkdir -p $QT_QUOTE_DIST/config
mkdir -p $QT_QUOTE_DIST/bin
cp -a config/* $QT_QUOTE_DIST/config
cp -a bin/*.so $QT_QUOTE_DIST/bin
cp -a $QT_LIB_BIN_DIR/ctp/libthosttraderapi.so $QT_QUOTE_DIST/bin
cp -a $QT_LIB_BIN_DIR/ctp/libthostmduserapi.so $QT_QUOTE_DIST/bin

# quote: minictp_level1
cd $QT_SOURCE_ROOT/my_quote_minictp_level1_lib
QT_QUOTE_DIST=$TRADER_LIB_ROOT/quote/minictp_level1
make clean
make
mkdir -p $QT_QUOTE_DIST/config
mkdir -p $QT_QUOTE_DIST/bin
cp -a config/* $QT_QUOTE_DIST/config
cp -a bin/*.so $QT_QUOTE_DIST/bin
cp -a $QT_LIB_BIN_DIR/minictp/*.so $QT_QUOTE_DIST/bin

# quote: ctp_level2
cd $QT_SOURCE_ROOT/my_quote_ctp_level2_lib
QT_QUOTE_DIST=$TRADER_LIB_ROOT/quote/ctp_level2
make clean
make
mkdir -p $QT_QUOTE_DIST/config
mkdir -p $QT_QUOTE_DIST/bin
cp -a config/* $QT_QUOTE_DIST/config
cp -a bin/*.so $QT_QUOTE_DIST/bin
cp -a $QT_LIB_BIN_DIR/ctp/libthosttraderapi.so $QT_QUOTE_DIST/bin
cp -a $QT_LIB_BIN_DIR/ctp/libthostmduserapi.so $QT_QUOTE_DIST/bin

# quote: minictp_level2
cd $QT_SOURCE_ROOT/my_quote_minictp_level2_lib
QT_QUOTE_DIST=$TRADER_LIB_ROOT/quote/minictp_level2
make clean
make
mkdir -p $QT_QUOTE_DIST/config
mkdir -p $QT_QUOTE_DIST/bin
cp -a config/* $QT_QUOTE_DIST/config
cp -a bin/*.so $QT_QUOTE_DIST/bin
cp -a $QT_LIB_BIN_DIR/minictp/* $QT_QUOTE_DIST/bin

# quote: czce_level2
cd $QT_SOURCE_ROOT/my_quote_czce_level2_lib
QT_QUOTE_DIST=$TRADER_LIB_ROOT/quote/czce_level2
make clean
make
mkdir -p $QT_QUOTE_DIST/config
mkdir -p $QT_QUOTE_DIST/bin
cp -a config/* $QT_QUOTE_DIST/config
cp -a bin/*.so $QT_QUOTE_DIST/bin
cp -a $QT_LIB_BIN_DIR/datafeed_zce/libZceLevel2API.so $QT_QUOTE_DIST/bin

# quote: czce_level2_mc
cd $QT_SOURCE_ROOT/my_quote_czce_level2_mc_lib
QT_QUOTE_DIST=$TRADER_LIB_ROOT/quote/czce_level2_mc
make clean
make
mkdir -p $QT_QUOTE_DIST/config
mkdir -p $QT_QUOTE_DIST/bin
cp -a config/* $QT_QUOTE_DIST/config
cp -a bin/*.so $QT_QUOTE_DIST/bin

# quote: dce_level2
cd $QT_SOURCE_ROOT/my_quote_dce_level2_lib
QT_QUOTE_DIST=$TRADER_LIB_ROOT/quote/dce_level2
make clean
make
mkdir -p $QT_QUOTE_DIST/config
mkdir -p $QT_QUOTE_DIST/bin
cp -a config/* $QT_QUOTE_DIST/config
cp -a bin/*.so $QT_QUOTE_DIST/bin
cp -a $QT_LIB_BIN_DIR/datafeed_dce/liblevel2Api.so $QT_QUOTE_DIST/bin

# quote: ib
cd $QT_SOURCE_ROOT/my_quote_ib_lib
QT_QUOTE_DIST=$TRADER_LIB_ROOT/quote/ib
make clean
make
mkdir -p $QT_QUOTE_DIST/config
mkdir -p $QT_QUOTE_DIST/bin
cp -a config/* $QT_QUOTE_DIST/config
cp -a bin/*.so $QT_QUOTE_DIST/bin

# quote: lts_level2_udp (stock)
cd $QT_SOURCE_ROOT/my_quote_lts_level2_udp_lib
QT_QUOTE_DIST=$TRADER_LIB_ROOT/quote/lts_level2_udp
make clean
make
mkdir -p $QT_QUOTE_DIST/config
mkdir -p $QT_QUOTE_DIST/bin
cp -a config/* $QT_QUOTE_DIST/config
cp -a bin/*.so $QT_QUOTE_DIST/bin
cp -a $QT_LIB_BIN_DIR/lts/lib* $QT_QUOTE_DIST/bin

# quote: lts (stock option)
cd $QT_SOURCE_ROOT/my_quote_lts_lib
QT_QUOTE_DIST=$TRADER_LIB_ROOT/quote/lts
sed -i 's;CRACK:=n;CRACK:=y;' Makefile
make clean
make
sed -i 's;CRACK:=y;CRACK:=n;' Makefile
make clean
make
sed -i 's;CRACK:=n;CRACK:=y;' Makefile
mkdir -p $QT_QUOTE_DIST/config
mkdir -p $QT_QUOTE_DIST/bin
cp -a config/* $QT_QUOTE_DIST/config
cp -a bin/*.so $QT_QUOTE_DIST/bin
cp -a $QT_LIB_BIN_DIR/lts/lib* $QT_QUOTE_DIST/bin

# quote: sec_kmds (stock in citics)
cd $QT_SOURCE_ROOT/my_quote_sec_kmds_lib
QT_QUOTE_DIST=$TRADER_LIB_ROOT/quote/sec_kmds
make clean
make
mkdir -p $QT_QUOTE_DIST/config
mkdir -p $QT_QUOTE_DIST/bin
cp -a config/* $QT_QUOTE_DIST/config
cp -a bin/*.so $QT_QUOTE_DIST/bin
cp -a $QT_LIB_BIN_DIR/kmds_sec_citics/lib* $QT_QUOTE_DIST/bin

# quote: sgit
cd $QT_SOURCE_ROOT/my_quote_sgit
QT_QUOTE_DIST=$TRADER_LIB_ROOT/quote/sgit
make clean
make
mkdir -p $QT_QUOTE_DIST/config
mkdir -p $QT_QUOTE_DIST/bin
cp -a config/* $QT_QUOTE_DIST/config
cp -a bin/*.so $QT_QUOTE_DIST/bin
cp -a $QT_LIB_BIN_DIR/sgit4.1/libsgitquotapi.so $QT_QUOTE_DIST/bin

# quote: shfe_my_jr (shfe level2 in jinrui)
cd $QT_SOURCE_ROOT/my_quote_shfe_my_jr_lib
QT_QUOTE_DIST=$TRADER_LIB_ROOT/quote/shfe_my_jr
make clean
make
mkdir -p $QT_QUOTE_DIST/config
mkdir -p $QT_QUOTE_DIST/bin
cp -a config/* $QT_QUOTE_DIST/config
cp -a bin/*.so $QT_QUOTE_DIST/bin
cp -a $QT_LIB_BIN_DIR/ctp/lib* $QT_QUOTE_DIST/bin

# quote: shfe_my_jr_new (shfe level2 in jinrui, new interface)
cd $QT_SOURCE_ROOT/my_quote_shfe_my_jr_new_lib
QT_QUOTE_DIST=$TRADER_LIB_ROOT/quote/shfe_my_jr_new
make clean
make
mkdir -p $QT_QUOTE_DIST/config
mkdir -p $QT_QUOTE_DIST/bin
cp -a config/* $QT_QUOTE_DIST/config
cp -a bin/*.so $QT_QUOTE_DIST/bin
cp -a $QT_LIB_BIN_DIR/ctp/lib* $QT_QUOTE_DIST/bin

# quote: shfe_my (shfe level2 designed by MY)
cd $QT_SOURCE_ROOT/my_quote_shfe_my_lib
QT_QUOTE_DIST=$TRADER_LIB_ROOT/quote/shfe_my
make clean
make
mkdir -p $QT_QUOTE_DIST/config
mkdir -p $QT_QUOTE_DIST/bin
cp -a config/* $QT_QUOTE_DIST/config
cp -a bin/*.so $QT_QUOTE_DIST/bin
cp -a $QT_LIB_BIN_DIR/libzmq* $QT_QUOTE_DIST/bin

# quote: shfe_my_xele (shfe level2 of xele)
cd $QT_SOURCE_ROOT/my_quote_shfe_my_xele_lib
QT_QUOTE_DIST=$TRADER_LIB_ROOT/quote/shfe_my_xele
make clean
make
mkdir -p $QT_QUOTE_DIST/config
mkdir -p $QT_QUOTE_DIST/bin
cp -a config/* $QT_QUOTE_DIST/config
cp -a bin/*.so $QT_QUOTE_DIST/bin
cp -a $QT_LIB_BIN_DIR/ac_xele_md_shfe/libac_xele_md.so $QT_QUOTE_DIST/bin

# quote: shfe_my_zz (shfe level2 of zeusing)
cd $QT_SOURCE_ROOT/my_quote_shfe_my_zz_lib
QT_QUOTE_DIST=$TRADER_LIB_ROOT/quote/shfe_my_zz
make clean
make
mkdir -p $QT_QUOTE_DIST/config
mkdir -p $QT_QUOTE_DIST/bin
cp -a config/* $QT_QUOTE_DIST/config
cp -a bin/*.so $QT_QUOTE_DIST/bin

# quote: stock_tdf (stock of tdf)
cd $QT_SOURCE_ROOT/my_quote_stock_tdf_lib
QT_QUOTE_DIST=$TRADER_LIB_ROOT/quote/stock_tdf
make clean
make
mkdir -p $QT_QUOTE_DIST/config
mkdir -p $QT_QUOTE_DIST/bin
cp -a config/* $QT_QUOTE_DIST/config
cp -a bin/*.so $QT_QUOTE_DIST/bin
cp -a $QT_LIB_BIN_DIR/tdf/lib* $QT_QUOTE_DIST/bin

# quote: tap (tap)
cd $QT_SOURCE_ROOT/my_quote_tap_lib
QT_QUOTE_DIST=$TRADER_LIB_ROOT/quote/tap
make clean
make
mkdir -p $QT_QUOTE_DIST/config
mkdir -p $QT_QUOTE_DIST/bin
cp -a config/* $QT_QUOTE_DIST/config
cp -a bin/*.so $QT_QUOTE_DIST/bin
cp -a $QT_LIB_BIN_DIR/tap/libTapQuoteAPI.so $QT_QUOTE_DIST/bin

# quote: my_datasource
# commentted by wangying on 20160725
#cd $QT_SOURCE_ROOT/my_quote_datasource_lib
#QT_QUOTE_DIST=$TRADER_LIB_ROOT/quote/my_datasource
make clean
make
mkdir -p $QT_QUOTE_DIST/config
mkdir -p $QT_QUOTE_DIST/bin
cp -a config/* $QT_QUOTE_DIST/config
cp -a bin/*.so $QT_QUOTE_DIST/bin




