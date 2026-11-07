#!/bin/bash

if (test "$1") && (test "$2") && (test "$3"); then
    git clone -n "$1" "$3" || \
    exit -4
    echo "clone $1 ok."
    cd "$3" && \
    git checkout "$2" || \
    exit -5
    echo "checkout $2 ok."
    test "$4" || \
    return 0
    cd "$3" && \
    make && make install "PREFIX=$3/${DEPENDS_BUILD_LOCAL}" || \
    exit -6
    echo "install $3 ok."
fi