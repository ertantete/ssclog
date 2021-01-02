#!/bin/sh

BUILD_DIR=./build_asn2xml
SRC_DIR=..

mkdir -p ${BUILD_DIR}/connect

cp ${SRC_DIR}/asnlib/*.[ch] ${BUILD_DIR}
cp ${SRC_DIR}/connect/*.[ch] ${BUILD_DIR}
cp ${SRC_DIR}/corelib/*.[ch] ${BUILD_DIR}
cp ${SRC_DIR}/corelib/ncbilcl.* ${BUILD_DIR}
cp ${SRC_DIR}/connect/*.h ${BUILD_DIR}/connect
cp ${SRC_DIR}/ssclog/*.[ch] ${BUILD_DIR}
cp ${SRC_DIR}/include/*.h ${BUILD_DIR}
cp -r ${SRC_DIR}/tables ${BUILD_DIR}
cp ${SRC_DIR}/scripts/makeasn2xml ${BUILD_DIR}/makefile
cd ${BUILD_DIR}

