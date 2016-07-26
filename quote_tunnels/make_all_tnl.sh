#!/bin/bash

mkdir -p ../output

cd my_common
make clean
make
cp -a bin/*.so ../output

cd ..

cd my_compliance_checker
make clean
make
cp -a bin/*.so ../output

cd ..

cd my_exchange_fut_op
make clean
make
cp -a bin/*.so ../output

cd ..

cd my_exchange_stock
make clean
make
cp -a bin/*.so ../output

cd ..

cd my_tunnel_lib_ctp
make clean
make
cp -a bin/*.so ../output

cd ..

cd my_tunnel_lib_ctp_mm
make clean
make
cp -a bin/*.so ../output

cd ..

cd my_tunnel_lib_esunny
make clean
make
cp -a bin/*.so ../output

cd ..

cd my_tunnel_lib_esunny_opt
make clean
make
cp -a bin/*.so ../output

cd ..

cd my_tunnel_lib_femas
make clean
make
cp -a bin/*.so ../output

cd ..

cd my_tunnel_lib_ib_api
make clean
make
cp -a bin/*.so ../output

cd ..

cd my_tunnel_lib_kgi_fix
make clean
make
cp -a bin/*.so ../output

cd ..

cd my_tunnel_lib_lts
make clean
make
cp -a bin/*.so ../output

cd ..

cd my_tunnel_lib_sgit
make clean
make
cp -a bin/*.so ../output

cd ..

cd my_tunnel_lib_xspeed_fut
make clean
make
cp -a bin/*.so ../output

cd ..

cd my_tunnel_lib_xspeed_mm
make clean
make
cp -a bin/*.so ../output

cd ..

cd my_tunnel_lib_zeusing
make clean
make
cp -a bin/*.so ../output

cd ..


