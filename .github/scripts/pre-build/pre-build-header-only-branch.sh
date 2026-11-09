#!/bin/bash

if (test "$1") && (test "$2") && (test "$3"); then
    git clone "$1" --branch "$2" --depth 1 "$3" || \
    exit -2
    echo "clone $1 $2 ok."
    test "$4" || \
    return 0
    cd "$3" && \
    make && make install "PREFIX=$3/${DEPENDS_BUILD_LOCAL}" || \
    exit -3
    echo "install $3 ok."
fi