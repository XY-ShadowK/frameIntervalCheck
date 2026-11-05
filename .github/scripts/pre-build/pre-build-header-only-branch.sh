#!/bin/sh

if ((test $1)) && ((test $2)) && ((test $3)); then
    git clone $1 --branch $2 --depth 1 $3 || \
    exit -1
    echo "clone $1 ok."
    test $4 && \
    make && make install DESTDIR=$3/build || \
    exit -1
    echo "install $3 ok."
fi