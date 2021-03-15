#!/bin/bash

# TESTED:
# FLAVOR="fedora"; VERS="33";       docker run -it --rm -h "${FLAVOR}${VERS}" --name "${FLAVOR}${VERS}" ${FLAVOR}:${VERS}
# FLAVOR="fedora"; VERS="32";       docker run -it --rm -h "${FLAVOR}${VERS}" --name "${FLAVOR}${VERS}" ${FLAVOR}:${VERS}
# FLAVOR="fedora"; VERS="31";       docker run -it --rm -h "${FLAVOR}${VERS}" --name "${FLAVOR}${VERS}" ${FLAVOR}:${VERS}
# FLAVOR="fedora"; VERS="30";       docker run -it --rm -h "${FLAVOR}${VERS}" --name "${FLAVOR}${VERS}" ${FLAVOR}:${VERS}
# FLAVOR="fedora"; VERS="29";       docker run -it --rm -h "${FLAVOR}${VERS}" --name "${FLAVOR}${VERS}" ${FLAVOR}:${VERS}
# FLAVOR="fedora"; VERS="28";       docker run -it --rm -h "${FLAVOR}${VERS}" --name "${FLAVOR}${VERS}" ${FLAVOR}:${VERS}
# FLAVOR="debian"; VERS="bullseye"; docker run -it --rm -h "${FLAVOR}${VERS}" --name "${FLAVOR}${VERS}" ${FLAVOR}:${VERS}
# FLAVOR="debian"; VERS="buster";   docker run -it --rm -h "${FLAVOR}${VERS}" --name "${FLAVOR}${VERS}" ${FLAVOR}:${VERS}
# FLAVOR="debian"; VERS="stretch";  docker run -it --rm -h "${FLAVOR}${VERS}" --name "${FLAVOR}${VERS}" ${FLAVOR}:${VERS}
# FLAVOR="debian"; VERS="jessie";   docker run -it --rm -h "${FLAVOR}${VERS}" --name "${FLAVOR}${VERS}" ${FLAVOR}:${VERS}
# FLAVOR="ubuntu"; VERS="focal";    docker run -it --rm -h "${FLAVOR}${VERS}" --name "${FLAVOR}${VERS}" ${FLAVOR}:${VERS}
# FLAVOR="ubuntu"; VERS="bionic";   docker run -it --rm -h "${FLAVOR}${VERS}" --name "${FLAVOR}${VERS}" ${FLAVOR}:${VERS}
# FLAVOR="ubuntu"; VERS="xenial";   docker run -it --rm -h "${FLAVOR}${VERS}" --name "${FLAVOR}${VERS}" ${FLAVOR}:${VERS}

bail()
{
  echo "$1"
  exit 1
}

compile_libpcap()
{
  TMP=`mktemp -d`
  pushd ${TMP}
  curl -L https://www.tcpdump.org/release/libpcap-$1.tar.gz -o libpcap-$1.tar.gz
  tar xf libpcap-$1.tar.gz
  pushd libpcap-$1
  ./configure --disable-dbus --without-libnl
  make -j `nproc`
  cp -v libpcap.a ${CWD}/
  popd
  popd
  rm -rf ${TMP}
}

compile_libnet()
{
  TMP=`mktemp -d`
  pushd ${TMP}
  curl -L https://github.com/libnet/libnet/releases/download/v$1/libnet-$1.tar.gz -o libnet-$1.tar.gz
  tar xf libnet-$1.tar.gz
  pushd libnet-$1/
  ./configure --enable-static=yes --disable-doxygen-doc --disable-doxygen-dot --disable-doxygen-man --disable-doxygen-html
  make -j `nproc`
  cp -v src/.libs/libnet.a ${CWD}/
  popd
  popd
  rm -rf ${TMP}
}

compile_libsodium()
{
  TMP=`mktemp -d`
  pushd ${TMP}
  curl -L https://download.libsodium.org/libsodium/releases/libsodium-$1.tar.gz -o libsodium-$1.tar.gz
  tar xf libsodium-$1.tar.gz
  pushd libsodium-stable/
  ./configure --enable-minimal --enable-static=yes
  make -j `nproc`
  cp -v src/libsodium/.libs/libsodium.a ${CWD}/
  popd
  popd
  rm -rf ${TMP}
}

set -e
CWD=`pwd`

# Investigate Identity
if [ -f /etc/fedora-release ]; then
  FLAVOR="fedora"
  VERS=`grep '^VERSION_ID' /etc/os-release | cut -d= -f2`
elif [ -f /etc/debian_version ]; then
  FLAVOR=`grep '^ID=' /etc/os-release | cut -d= -f2`
  VERS=`cat /etc/debian_version | cut -d/ -f1`
elif [ -f /etc/rapidlinux-version ]; then
  FLAVOR="rapidlinux"
else
  bail "I dont know who you are"
fi

# Get Ubuntu Version
if [ ${FLAVOR} == "ubuntu" ]; then
  VERS=`grep '^VERSION_ID' /etc/os-release | cut -d\" -f2`
fi

# Install Prerequisites
case ${FLAVOR} in
      fedora) yum install -y glibc-static gcc make curl flex bison xz findutils libsodium-devel libpcap-devel libnet-devel;;
      debian) apt-get update; apt-get install -y build-essential curl flex bison libsodium-dev libpcap-dev libnet1-dev;;
      ubuntu) apt-get update; apt-get install -y build-essential curl flex bison libsodium-dev libpcap-dev libnet1-dev;;
  rapidlinux) echo "I came prepared!";;
           *) bail "I dont know who you are";;
esac

# Define SLIBDIR
case ${FLAVOR} in
      fedora) SLIBDIR="/usr/lib64";;
      debian) SLIBDIR="/usr/lib/x86_64-linux-gnu";;
      ubuntu) SLIBDIR="/usr/lib/x86_64-linux-gnu";;
  rapidlinux) SLIBDIR="/usr/lib64";;
           *) SLIBDIR=`find /usr -type f -name libpthread.a -exec dirname {} \;`;;
esac

# Grab RapidLinux Static Libs
if [ ${FLAVOR} == "rapidlinux" ]; then
  cp -v ${SLIBDIR}/libpthread.a ./
  cp -v ${SLIBDIR}/libsodium.a ./
  cp -v ${SLIBDIR}/libnet.a ./
  cp -v ${SLIBDIR}/libpcap.a ./
  exit 0
fi

# Grab Ubuntu Static Libs
if [ ${FLAVOR} == "ubuntu" ]; then
  cp -v ${SLIBDIR}/libpthread.a ./
  cp -v ${SLIBDIR}/libsodium.a ./
  cp -v ${SLIBDIR}/libnet.a ./
  cp -v ${SLIBDIR}/libpcap.a ./
  exit 0
fi

# Grab Debian Static Libs
if [ ${FLAVOR} == "debian" ]; then
  if [ ${VERS} == "bullseye" ]; then
    compile_libpcap "1.9.1"
  else
    cp -v ${SLIBDIR}/libpcap.a ./
  fi
  cp -v ${SLIBDIR}/libpthread.a ./
  cp -v ${SLIBDIR}/libsodium.a ./
  cp -v ${SLIBDIR}/libnet.a ./
  exit 0
fi

# Make Fedora Static Libs
if [ ${FLAVOR} == "fedora" ] && [ ${VERS} == "33" ]; then
  cp -v ${SLIBDIR}/libpthread.a ./
  compile_libsodium "1.0.18-stable"
  compile_libnet "1.2"
  compile_libpcap "1.10.0"
  exit 0
fi

if [ ${FLAVOR} == "fedora" ] && [ ${VERS} == "32" ]; then
  cp -v ${SLIBDIR}/libpthread.a ./
  compile_libsodium "1.0.18-stable"
  compile_libnet "1.2"
  compile_libpcap "1.10.0"
  exit 0
fi

if [ ${FLAVOR} == "fedora" ] && [ ${VERS} == "31" ]; then
  cp -v ${SLIBDIR}/libpthread.a ./
  compile_libsodium "1.0.18-stable"
  compile_libnet "1.1.6"
  compile_libpcap "1.9.1"
  exit 0
fi

if [ ${FLAVOR} == "fedora" ] && [ ${VERS} == "30" ]; then
  cp -v ${SLIBDIR}/libpthread.a ./
  compile_libsodium "1.0.18-stable"
  compile_libnet "1.1.6"
  compile_libpcap "1.9.1"
  exit 0
fi

if [ ${FLAVOR} == "fedora" ] && [ ${VERS} == "29" ]; then
  cp -v ${SLIBDIR}/libpthread.a ./
  compile_libsodium "1.0.18-stable"
  compile_libnet "1.1.6"
  compile_libpcap "1.9.1"
  exit 0
fi

if [ ${FLAVOR} == "fedora" ] && [ ${VERS} == "28" ]; then
  cp -v ${SLIBDIR}/libpthread.a ./
  compile_libsodium "1.0.17-stable"
  compile_libnet "1.1.6"
  compile_libpcap "1.9.0"
  exit 0
fi
