#!/bin/sh

WINDRES=/usr/bin/i686-w64-mingw32-windres
BUILD_DIR=./build_win
SRC_DIR=..

mkdir -p ${BUILD_DIR}/connect
cp ${SRC_DIR}/asnlib/*.[ch] ${BUILD_DIR}
cp ${SRC_DIR}/connect/*.[ch] ${BUILD_DIR}
cp ${SRC_DIR}/corelib/*.[ch] ${BUILD_DIR}
cp ${SRC_DIR}/corelib/ncbilcl.* ${BUILD_DIR}
cp ${SRC_DIR}/vibrant/*.[ch] ${BUILD_DIR}
cp ${SRC_DIR}/connect/*.h ${BUILD_DIR}/connect
cp ${SRC_DIR}/ssclog/*.[ch] ${BUILD_DIR}
cp ${SRC_DIR}/include/*.h ${BUILD_DIR}
cp -r ${SRC_DIR}/tables ${BUILD_DIR}
cp ${SRC_DIR}/scripts/ssclogo.ico ${BUILD_DIR}
cp ${SRC_DIR}/scripts/ssclogo.rc ${BUILD_DIR}
cp ${SRC_DIR}/scripts/makemingwssc ${BUILD_DIR}/makefile
cd ${BUILD_DIR}
${WINDRES} -i ssclogo.rc -o ssclogo.o
