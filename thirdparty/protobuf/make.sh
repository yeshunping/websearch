#!/bin/bash

function logexec(){
	echo "[$(date +%F\ %T)] $*"
	$*	
}

curdir=$(pwd)
basedir=$(dirname $0)
if [ "${basedir}" = "." ]; then
    basedir=$(pwd)
fi

if [ "${basedir:0:1}" != "/" ] ;then 
	basedir=$(pwd)/${basedir}
fi

logexec cd ${basedir}
logexec rm -rf ${basedir}/include ${basedir}/bin ${basedir}/lib64_*
logexec tar zxvf protobuf-2.5.0.tar.gz 

#install libunwind
logexec cd ${basedir}/protobuf-2.5.0
logexec chmod +x configure
./configure  --prefix=${basedir}/install --with-pic --with-cpp --with-zlib --enable-shared=false 
make
chmod +x install-sh
make install

#准备
logexec rm -rf ${basedir}/include
logexec cp -r ${basedir}/install/include ${basedir}/.
logexec rm -rf ${basedir}/lib64_debug ${basedir}/lib64_release
logexec cp -r ${basedir}/install/lib ${basedir}/lib64_debug
logexec cp -r ${basedir}/install/lib ${basedir}/lib64_release

logexec cp -rf ${basedir}/install/bin/ ${basedir}/

logexec rm -rf ${basedir}/install
logexec rm -rf ${basedir}/protobuf-2.5.0

logexec cd ${curdir}
exit 0

