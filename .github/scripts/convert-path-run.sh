#!/bin/bash

if ! (test "$1") || ! (test "$2") ; then
    exit -1
fi

BUILD_TARGET="$1"

if [ "$BUILD_TARGET" == "windows" ]; then
    SCRIPTS_DIR="$(cygpath -u "$SCRIPTS_DIR")"
    DEPENDS_BUILD_PREFIX="$(cygpath -u "$DEPENDS_BUILD_PREFIX")"
    BUILD_DIR="$(cygpath -u "$BUILD_DIR")"
    echo "windows msys2 path converted."
    echo "SCRIPTS_DIR = ${SCRIPTS_DIR}"
    echo "DEPENDS_BUILD_PREFIX = ${DEPENDS_BUILD_PREFIX}"
    echo "BUILD_DIR = ${BUILD_DIR}"
fi

source "$2"