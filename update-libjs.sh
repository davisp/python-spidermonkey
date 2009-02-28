#! /bin/bash -e

URL=http://ftp.mozilla.org/pub/mozilla.org/js/js-1.7.0.tar.gz

CFG=spidermonkey/`uname -s`-`uname -m`
DEST=spidermonkey/libjs
CWD=`pwd`
TMP=tmp

if [ ! -d $TMP ]; then
    mkdir $TMP
fi

if [ ! -d $DEST ]; then
    mkdir $DEST
fi

if [ ! -d $CFG ]; then
    mkdir $CFG
fi

cd $TMP && wget -N $URL && tar -xvzf js-1.7.0.tar.gz && cd ..
cd $TMP/js/src/
make -f Makefile.ref jscpucfg jskwgen
cd $CWD

for ext in `echo "c h msg tbl"`; do
    cp $TMP/js/src/*.$ext $DEST
done

rm $DEST/js.c
rm $DEST/jscpucfg.*
rm $DEST/jskwgen.*

./$TMP/js/src/jscpucfg > $CFG/jsautocfg.h
./$TMP/js/src/jskwgen > $CFG/jsautokw.h

