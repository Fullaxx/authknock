#!/bin/bash

set -e

OPT="-O2"
DBG="-ggdb3 -DDEBUG"
CFLAGS="-Wall"
OPTCFLAGS="${CFLAGS} ${OPT}"
DBGCFLAGS="${CFLAGS} ${DBG}"

if [ -r /usr/lib64/libpthread.a ]; then
  SLIBDIR="/usr/lib64"
elif [ -r /usr/lib/x86_64-linux-gnu/libpthread.a ]; then
  SLIBDIR="/usr/lib/x86_64-linux-gnu"
else
  echo "I don't know where to find your static libs"
  echo "Manually set SLIBDIR"
  exit 1
fi

rm -rf *.exe *.dbg o
mkdir o
for SC in *.c; do
  gcc ${OPTCFLAGS} -c ${SC} -o o/`basename ${SC}`.o
done

if [ -f ${SLIBDIR}/libpthread.a ] && [ ${SLIBDIR}/libsodium.a ]; then
  gcc -static o/gen_keypair.c.o \
  ${SLIBDIR}/{libsodium,libpthread}.a \
  -o gen_keypair.static.exe

  gcc -static o/{ip_knock,getopts,futils}.c.o \
  ${SLIBDIR}/{libsodium,libpthread,libnet}.a \
  -o ip_knock.static.exe

  if [ -f ${SLIBDIR}/libsodium.a ]; then
    gcc -static o/{ip_receptor,getopts,futils,dissect,payload}.c.o \
    ${SLIBDIR}/{libsodium,libpthread,libpcap}.a \
    -o ip_receptor.static.exe
  fi
fi

strip *.exe
rm -rf o
