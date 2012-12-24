#!/bin/bash

TARGET=/usr/local


pushd .
# Agar-Headers
cd include/agar/agar
mkdir -p ${TARGET}/include
mkdir -p ${TARGET}/include/agar
mkdir -p ${TARGET}/include/agar/agar
for D in au config core dev gui math rg vg; do
    mkdir -p ${TARGET}/include/agar/agar/${D}
    pushd .
    cd ${D}
    install -m 0644 * ${TARGET}/include/agar/agar/${D}/
    popd
done
install -m 0644 *.h ${TARGET}/include/agar/agar/
popd

#Libs
pushd .
cd lib
install lib*.a ${TARGET}/lib
install lib*.so.5.0.0 ${TARGET}/lib
ldconfig
popd

# ag-config etc
pushd .
cd bin
install -m 0755 * ${TARGET}/bin
popd