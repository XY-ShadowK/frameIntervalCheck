#!/bin/bash

cd ${BUILD_DIR}
LICENSE_DIR="licenses"
mkdir $LICENSE_DIR

MSYS2_MINGW64_PREIFX="mingw-w64-x86_64-"
MSYS2_SOURCE_REPO_PREFIX="https://repo.msys2.org/mingw/sources/mingw-w64-"
MSYS2_SOURCE_REPO_SUFFIX=".src.tar.zst"
MSYS2_MINGW64_REPO_PREFIX="https://repo.msys2.org/mingw/mingw64"
MSYS2_MINGW64_REPO_PACKAGE_SUFFIX="-any.pkg.tar.zst"

declare -A LINUX_PKG_LICENSE=(
    [libva]="Expat"
    [libvdpau]="MIT"
    [glibc]="LGPL-2.1-or-later AND GPL-2.0-or-later"
)

echo_append_notice() {
    echo $1
    echo $1 >> NOTICE
}

echo_component_notice() {
    ((num_component++))
    echo_append_notice "${num_component}. $1"
    echo_append_notice "   Version: $2"
    echo_append_notice "   Source: $3"
    echo_append_notice "   License: $4"
    local -n licenses=$5
    echo_append_notice "   License file: ${licenses[0]}"
    if [ ${#licenses[@]} -gt 1 ]; then
        for license in ${licenses[@]:1}; do
            echo_append_notice "                 ${license}"
        done
    fi
}

collect_echo_component_notice_github() {
    if ! (test $1) || ! (test $2) || ! (test $3) || ! (test $4); then
        exit -1
    fi
    if (test $5); then
        local component_licenses=$5
        else
            local component_licenses=( LICENSE )
    fi
    mkdir "${LICENSE_DIR}/${package_name}" || \
        exit -2
    for component_license in ${component_licenses[@]}; do
        cp "${DEPENDS_BUILD_PREFIX}/$1/${component_license}" "${LICENSE_DIR}/${package_name}"
    done
    local package_licenses=("${LICENSE_DIR}/${package_name}/*")
    echo_component_notice \
        $1 \
        $2 \
        $3 \
        $4 \
        $package_licenses
}

collect_echo_component_notice_msys2_source() {
    package_source="${MSYS2_SOURCE_REPOO_PREFIX}/${package_name}-${package_version}${MSYS2_SOURCE_REPO_SUFFIX}"
}

collect_echo_component_notice_msys2_mingw64() {
    package_source="${MSYS2_MINGW64_REPO_PREFIX}/${package_name}-${package_version}${MSYS2_MINGW64_REPO_PACKAGE_SUFFIX}"

collect_echo_component_notice_msys2() {
    local component_name=$1
    local package_name=$(pacman -Qi $component_name | grep '^Name' | cut -d: -f2- | sed 's/^ *//')
    local package_version=$(pacman -Qi $component_name | grep '^Version' | cut -d: -f2- | sed 's/^ *//')
    local package_license=$(pacman -Qi $component_name | grep '^License' | cut -d: -f2- | sed 's/^ *//; s/^spdx://')
    local package_source=""
    test $2 && \
    collect_echo_component_notice_msys2_mingw64 $1 || \
    collect_echo_component_notice_msys2_source $1
    local license_dir="/mingw64/share/licenses/${package_name#MSYS2_MINGW64_PREIFX}"
    mkdir "${LICENSE_DIR}/${package_name}" && \
    cp -a -p "${license_dir}/." "${LICENSE_DIR}/${package_name}" || \
    exit -2
    local package_licenses=("${LICENSE_DIR}/${package_name}/*")
    echo_component_notice \
        $package_name \
        $package_version \
        $package_source \
        $package_license \
        $package_licenses
}

collect_echo_component_notice_linux() {
    local component_name=$1
    local package_name=$(dpkg-query -W -f='${binary:Package}' $1)
    local package_version=$(dpkg-query -W -f='${Version}' $1)
    local package_source=$(apt-get source --print-urls "$1=${package_version}" | grep -m1 "^'" | cut -d"'" -f2)
    if test ${LINUX_PKG_LICENSE[$package_name]}; then
        local package_license=${LINUX_PKG_LICENSE[$package_name]}
        else
            exit -3
    fi
    local doc_dir="/usr/share/doc/${package_name}"
    mkdir "${LICENSE_DIR}/${package_name}" && \
    cp "${doc_dir}/copyright" "${LICENSE_DIR}/${package_name}" || \
    exit -2
    local package_licenses=("${LICENSE_DIR}/${package_name}/copyright")
    echo_component_notice \
        $package_name \
        $package_version \
        $package_source \
        $package_license \
        $package_licenses
}

collect_echo_component_notice() {
    [ $BUILD_TARGET == "windows" ] && \
    collect_echo_component_notice_msys2 $1 $2 || \
    collect_echo_component_notice_linux $1 $2
}

touch NOTICE
echo_append_notice "Third-Party Notices"
echo_append_notice ""
echo_append_notice "This product contains third-party components licensed under open-source licenses."
echo_append_notice "Detailed license texts are located in the "licenses/" directory."
echo_append_notice ""
echo_append_notice "==============================================================================="
echo_append_notice ""

num_component=0

collect_echo_component_notice_github \
    $WINDOWSRUB_NAME \
    "none (commit ${WINDOWSRUB_COMMIT})" \
    $WINDOWSRUB_REPO \
    $WINDOWSRUB_LICENSE

collect_echo_component_notice_github \
    $CXXOPTS_NAME \
    "none (commit ${CXXOPTS_COMMIT})" \
    $CXXOPTS_REPO \
    $CXXOPTS_LICENSE

collect_echo_component_notice_github \
    $FFMPEG_NAME \
    "none (commit ${FFMPEG_TAG})" \
    $FFMPEG_REPO \
    $FFMPEG_LICENSE

collect_echo_component_notice_github \
    $FFNVCODEC_NAME \
    "none (commit ${FFNVCODEC_TAG})" \
    $FFNVCODEC_REPO \
    $FFNVCODEC_LICENSE

if [ $BUILD_TARGET == "windows" ]; then
    collect_echo_component_notice_msys2 \
        zlib \
        1
    collect_echo_component_notice_msys2 \
        mingw-w64-x86_64-libwinpthread
    collect_echo_component_notice_msys2 \
        mingw-w64-x86_64-crt
    collect_echo_component_notice_msys2 \
        libgcc
    collect_echo_component_notice_msys2 \
        libstdc++
    else
        collect_echo_component_notice_linux \
            zlib1g-dev
        collect_echo_component_notice_linux \
            libva-dev
        collect_echo_component_notice_linux \
            libvdpau-dev
        collect_echo_component_notice_linux \
            libc6-dev
fi

echo_append_notice ""
echo_append_notice "==============================================================================="
echo_append_notice ""
echo_append_notice "For more information about these licenses, please refer to the files in the "licenses/" directory."