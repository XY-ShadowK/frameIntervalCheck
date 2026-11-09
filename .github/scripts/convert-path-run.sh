#!/bin/bash

if ! (test "$1") || ! (test "$2") ; then
    exit -1
fi

BUILD_TARGET="$1"

if [ "$BUILD_TARGET" == "windows" ]; then
    SCRIPTS_PATH="$(cygpath -u "$SCRIPTS_PATH")"
    DEPENDS_BUILD_PREFIX="$(cygpath -u "$DEPENDS_BUILD_PREFIX")"
    BUILD_PATH="$(cygpath -u "$BUILD_PATH")"
    echo "windows msys2 path converted."
    echo "SCRIPTS_PATH = ${SCRIPTS_PATH}"
    echo "DEPENDS_BUILD_PREFIX = ${DEPENDS_BUILD_PREFIX}"
    echo "BUILD_PATH = ${BUILD_PATH}"
fi

if [ -f "$2" ]; then
    source "$2"
    else
        eval "$2"
fi