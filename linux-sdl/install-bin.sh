#!/bin/bash
TARGET=/usr/local

install ./xm7 ${TARGET}/bin
install ./xm7.debug ${TARGET}/bin

#pushd .
#cd ../libs-linux-amd64/
#./install-libs.sh
#popd
cd ../../src
mkdir -p ${TARGET}/share
mkdir -p ${TARGET}/share/xm7

#Icons
pushd .
cd resource
install -m 0644 *.ico ${TARGET}/share/xm7/
install -m 0644 *.png ${TARGET}/share/xm7/
mkdir -p ${TARGET}/share/xm7/F-Font
cd F-Font
install -m 0644 *.ttf ${TARGET}/share/xm7/F-Font
install -m 0644 *.txt ${TARGET}/share/xm7/F-Font
popd

#I18n files.
for l in ja C; do
pushd .
cd locales/${l}
mkdir -p ${TARGET}/share/xm7/${l}
mkdir -p ${TARGET}/share/xm7/${l}/LC_MESSAGES
install -m 0644 *.po ${TARGET}/share/xm7/${l}/LC_MESSAGES
install -m 0644 *.mo ${TARGET}/share/xm7/${l}/LC_MESSAGES
popd
done
