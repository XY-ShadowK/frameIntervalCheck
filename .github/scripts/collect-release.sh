#!/bin/bash

BINARY_PATH="${BUILD_PATH}"

if [ "${BUILD_TARGET}" == "windows" ]; then
    SHAREDLIB_LOCAL="bin"
    SHAREDLIB_SUFFIX=".dll"
    BINARY_SUFFIX=".exe"
    SHAREDLIB_PATH_RELEASE="${RELEASE_PATH}"
    mkdir -p "${SHAREDLIB_PATH_RELEASE}"
    else
        SHAREDLIB_LOCAL="lib"
        SHAREDLIB_SUFFIX=".so"
        BINARY_SUFFIX=""
        SHAREDLIB_PATH_RELEASE="${RELEASE_PATH}/${SHAREDLIB_LOCAL}"
        mkdir -p "${SHAREDLIB_PATH_RELEASE}"
fi

SHAREDLIB_PATH_FFMPEG="${DEPENDS_BUILD_PREFIX}/${FFMPEG_NAME}/${DEPENDS_BUILD_LOCAL}/${SHAREDLIB_LOCAL}"
SHAREDLIB_PATH_MSYS_MINGW64="/mingw64/${SHAREDLIB_LOCAL}"

copy_to_release() {
    cp -v "$1" "${RELEASE_PATH}"
}

copy_executable_release() {
    if ! (test "$1"); then
        exit -1
    fi
    local executable_dir="${BINARY_PATH}/$1${BINARY_SUFFIX}"
    copy_to_release "$executable_dir"
}

generate_sharedlib_pattern_mingw64() {
    sharedlib_pattern="$1-*${SHAREDLIB_SUFFIX}"
}

generate_sharedlib_pattern_linux() {
    sharedlib_pattern="$1${SHAREDLIB_SUFFIX}*"
}

copy_sharedlib_release() {
    if ! (test "$1") || ! (test "$2"); then
        exit -1
    fi
    eval "local sharedlib_dir=\"\${SHAREDLIB_PATH_$1}\""
    local sharedlib_pattern=""
    if [ "$BUILD_TARGET" == "windows" ]; then
        if [ "$1" == "FFMPEG" ]; then
            generate_sharedlib_pattern_mingw64 "${2#lib}"
        elif [ "$2" == "zlib" ]; then
            sharedlib_pattern="$2*${SHAREDLIB_SUFFIX}"
            else
                generate_sharedlib_pattern_mingw64 "$2"
        fi
        else
            generate_sharedlib_pattern_linux "$2"
    fi
    find "${sharedlib_dir}" -maxdepth 1 -name "${sharedlib_pattern}" -exec \
        cp -av {} "${SHAREDLIB_PATH_RELEASE}/" \;
}

mkdir -p "$RELEASE_PATH"

copy_executable_release "${PROJECT_NAME}"

copy_sharedlib_release "FFMPEG" "libavcodec"
copy_sharedlib_release "FFMPEG" "libavformat"
copy_sharedlib_release "FFMPEG" "libavutil"
copy_sharedlib_release "FFMPEG" "libswresample"

echo "$BUILD_TARGET"
[ "$BUILD_TARGET" == "windows" ] || \
return 0

copy_sharedlib_release "MSYS_MINGW64" "libgcc_s_seh"
copy_sharedlib_release "MSYS_MINGW64" "libstdc++"
copy_sharedlib_release "MSYS_MINGW64" "libwinpthread"
copy_sharedlib_release "MSYS_MINGW64" "zlib"