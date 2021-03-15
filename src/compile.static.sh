#!/bin/bash

set -e

OPT="-O2"
DBG="-ggdb3 -DDEBUG"
CFLAGS="-Wall"
OPTCFLAGS="${CFLAGS} ${OPT}"
DBGCFLAGS="${CFLAGS} ${DBG}"

if [ ! -f libpthread.a ] || [ ! -f libsodium.a ] || [ ! -f libnet.a ] || [ ! -f libpcap.a ]; then
  ./prep_static_libs.sh
fi

rm -rf *.exe *.dbg o
mkdir o
for SC in *.c; do
  gcc ${OPTCFLAGS} -c ${SC} -o o/`basename ${SC}`.o
done

gcc -static o/gen_keypair.c.o {libsodium,libpthread}.a -o gen_keypair.static.exe
gcc -static o/{ip_knock,getopts,futils}.c.o {libsodium,libpthread,libnet}.a -o ip_knock.static.exe
gcc -static o/{ip_receptor,getopts,futils,dissect,payload}.c.o {libsodium,libpthread,libpcap}.a -o ip_receptor.static.exe

strip *.exe
rm -rf o
