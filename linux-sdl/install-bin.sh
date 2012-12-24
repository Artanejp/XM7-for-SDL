#!/bin/bash
TARGET=/usr/local/bin

install ./xm7 ${TARGET}
install ./xm7.debug ${TARGET}

cd ../libs-linux-amd64/
./install-libs.sh
