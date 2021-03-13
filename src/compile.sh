#!/bin/bash

# Thanks to:
# https://github.com/mwarning/libsodium-example.git

set -e

OPT="-O2"
DBG="-ggdb3 -DDEBUG"
CFLAGS="-Wall"
OPTCFLAGS="${CFLAGS} ${OPT}"
DBGCFLAGS="${CFLAGS} ${DBG}"

rm -f *.exe *.dbg

gcc -ansi ${OPTCFLAGS} gen_keypair.c -o gen_keypair.exe -lsodium
gcc -ansi ${DBGCFLAGS} gen_keypair.c -o gen_keypair.dbg -lsodium

gcc -ansi ${OPTCFLAGS} ip_knock.c getopts.c futils.c -o ip_knock.exe -lnet -lsodium
gcc -ansi ${DBGCFLAGS} ip_knock.c getopts.c futils.c -o ip_knock.dbg -lnet -lsodium

gcc ${OPTCFLAGS} ip_receptor.c getopts.c futils.c dissect.c payload.c -o ip_receptor.exe -lpcap -lsodium
gcc ${DBGCFLAGS} ip_receptor.c getopts.c futils.c dissect.c payload.c -o ip_receptor.dbg -lpcap -lsodium

strip *.exe
