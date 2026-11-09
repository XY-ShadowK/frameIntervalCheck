#!/bin/bash

mkdir -p "${RELEASE_PATH}"
cd "${RELEASE_PATH}"
echo "Release Directory = ${RELEASE_PATH}"
LICENSE_LOCAL="licenses"
mkdir -p "$LICENSE_LOCAL"

MSYS2_MINGW64_PREIFX="mingw-w64-x86_64-"
MSYS2_SOURCE_REPO_PREFIX="https://repo.msys2.org/mingw/sources/mingw-w64-"
MSYS2_SOURCE_REPO_SUFFIX=".src.tar.zst"
MSYS2_MINGW64_REPO_PREFIX="https://repo.msys2.org/mingw/mingw64"
MSYS2_MINGW64_REPO_PACKAGE_SUFFIX="-any.pkg.tar.zst"

# declare -A LINUX_PKG_NAME=(
#     [zlib1g-dev]="zlib"
#     [libva-dev]="libva"
#     [libvdpau-dev]="libvdpau"
#     [libc6-dev]="glibc"
# )
declare -A LINUX_PKG_LICENSE=(
    [zlib1g-dev]="zlib/libpng License"
    [libva-dev]="Expat"
    [libvdpau-dev]="MIT"
    [libc6-dev]="LGPL-2.1-or-later AND GPL-2.0-or-later"
)

echo_append_notice() {
    echo "$1"
    echo "$1" >> NOTICE
}

echo_component_notice() {
    ((num_component++))
    echo_append_notice "${num_component}. $1"
    echo_append_notice "   Version: $2"
    echo_append_notice "   Source: $3"
    echo_append_notice "   License: $4"
    local -n licenses=$5
    echo_append_notice "   License file: ${licenses[0]}"
    if [ "${#licenses[@]}" -gt 1 ]; then
        for license in "${licenses[@]:1}"; do
            echo_append_notice "                 ${license}"
        done
    fi
    echo_append_notice ""
}

expand_mirror_uri() {
    local uri="$1"
    if [[ "$uri" != mirror+file:* ]]; then
        echo "no need to expand mirror_uri"
        return 0
    fi

    local rest="${uri#mirror+file:}"
    local mirror_file="${rest%%/pool*}"
    local base="$(awk 'NF && $1 !~ /^#/ {print $1; exit}' "$mirror_file")"
    local rel="${uri#*$mirror_file}"
    rel="${rel#/}"
    local decoded="$(printf '%b' "${rel//%/\\x}")"
    package_source="${base%/}/${decoded}"
}

collect_echo_component_notice_github() {
    if ! (test "$1") || ! (test "$2") || ! (test "$3") || ! (test "$4"); then
        exit -1
    fi
    if (test "$5"); then
        local -n component_licenses=$5
        else
            local component_licenses=( LICENSE )
    fi
    mkdir -p "${LICENSE_LOCAL}/$1" || \
        exit -2
    for component_license in "${component_licenses[@]}"; do
        cp "${DEPENDS_BUILD_PREFIX}/$1/${component_license}" "${LICENSE_LOCAL}/$1"
    done
    local package_licenses=()
    while IFS= read -r -d '' file; do
        package_licenses+=("$file")
    done < <(find "${LICENSE_LOCAL}/$1" -maxdepth 1 -type f -print0)
    echo_component_notice \
        "$1" \
        "$2" \
        "$3" \
        "$4" \
        package_licenses
}

collect_echo_component_notice_msys2_source() {
    package_source="${MSYS2_SOURCE_REPO_PREFIX}${package_name}-${package_version}${MSYS2_SOURCE_REPO_SUFFIX}"
}

collect_echo_component_notice_msys2_mingw64() {
    package_source="${MSYS2_MINGW64_REPO_PREFIX}/${package_name}-${package_version}${MSYS2_MINGW64_REPO_PACKAGE_SUFFIX}"
}

collect_echo_component_notice_msys2() {
    local component_name="$1"
    local package_name="$(pacman -Qi $component_name | grep '^Name' | cut -d: -f2- | sed 's/^ *//')"
    local package_version="$(pacman -Qi $component_name | grep '^Version' | cut -d: -f2- | sed 's/^ *//')"
    local package_license="$(pacman -Qi $component_name | grep '^License' | cut -d: -f2- | sed 's/^ *//; s/^spdx://')"
    local package_source=""
    if (test "$2"); then
        collect_echo_component_notice_msys2_source "$1"
        else
            collect_echo_component_notice_msys2_mingw64 "$1"
    fi
    local license_dir="/mingw64/share/licenses/${package_name#${MSYS2_MINGW64_PREIFX}}"
    mkdir -p "${LICENSE_LOCAL}/${package_name}" && \
    cp -a -p "${license_dir}/." "${LICENSE_LOCAL}/${package_name}" || \
    exit -2
    local package_licenses=()
    while IFS= read -r -d '' file; do
        package_licenses+=("$file")
    done < <(find "${LICENSE_LOCAL}/${package_name}" -maxdepth 1 -type f -print0)
    echo_component_notice \
        "$package_name" \
        "$package_version" \
        "$package_source" \
        "$package_license" \
        package_licenses
}

collect_echo_component_notice_linux() {
    local component_name="$1"
    local package_name="$(dpkg-query -W -f='${Source}' "$component_name" | cut -d' ' -f1)"
    local package_version="$(dpkg-query -W -f='${source:Version}' "$component_name")"
    local package_source="$(apt-get source --print-uris "${package_name}=${package_version}" | grep -m1 "^'" | cut -d"'" -f2)"
    expand_mirror_uri "$package_source"
    test "${LINUX_PKG_LICENSE[$component_name]}" || \
        exit -3
    local package_license="${LINUX_PKG_LICENSE[$component_name]}"
    local doc_dir="/usr/share/doc/${component_name}"
    mkdir -p "${LICENSE_LOCAL}/${package_name}" && \
    cp "${doc_dir}/copyright" "${LICENSE_LOCAL}/${package_name}" || \
    exit -2
    local package_licenses=("${LICENSE_LOCAL}/${package_name}/copyright")
    echo_component_notice \
        "$package_name" \
        "$package_version" \
        "$package_source" \
        "$package_license" \
        package_licenses
}

extract_copyright() {
    if ! (test "$1") || ! (test "$2"); then
        exit -1
    fi
    awk '
        /^[[:space:]]*\/\*/ { in_block = 1; next }
        in_block && /^[[:space:]]*\*\// { exit }
        in_block { print }
    ' "$1" \
    | sed 's/^[[:space:]]*\*[[:space:]]*//' > "$2"
}

touch NOTICE
echo_append_notice "Third-Party Notices"
echo_append_notice ""
echo_append_notice "This product contains third-party components( some are c/c++ runtime or standard library implementation, system api, hardware driver api) licensed under open-source licenses."
echo_append_notice "Detailed license texts are located in the "licenses/" directory."
echo_append_notice ""
echo_append_notice "==============================================================================="
echo_append_notice ""

num_component=0

collect_echo_component_notice_github \
    "$WINDOWSRUB_NAME" \
    "none (commit ${WINDOWSRUB_COMMIT})" \
    "$WINDOWSRUB_REPO" \
    "$WINDOWSRUB_LICENSE"

collect_echo_component_notice_github \
    "$CXXOPTS_NAME" \
    "none (commit ${CXXOPTS_COMMIT})" \
    "$CXXOPTS_REPO" \
    "$CXXOPTS_LICENSE"

FFMPEG_LICENSES=(
    "build/LICENSE.md"
    "build/COPYING.LGPLv2.1"
)
collect_echo_component_notice_github \
    "$FFMPEG_NAME" \
    "${FFMPEG_TAG}" \
    "$FFMPEG_REPO" \
    "$FFMPEG_LICENSE" \
    FFMPEG_LICENSES

extract_copyright "${DEPENDS_BUILD_PREFIX}/${FFNVCODEC_NAME}/${DEPENDS_BUILD_LOCAL}/include/ffnvcodec/dynlink_cuda.h" "${DEPENDS_BUILD_PREFIX}/${FFNVCODEC_NAME}/nv-codec-headers.txt"
FFNVCODEC_LICENSES=("nv-codec-headers.txt")
collect_echo_component_notice_github \
    "$FFNVCODEC_NAME" \
    "${FFNVCODEC_TAG}" \
    "$FFNVCODEC_REPO" \
    "$FFNVCODEC_LICENSE" \
    FFNVCODEC_LICENSES

if [ $BUILD_TARGET == "windows" ]; then
    collect_echo_component_notice_msys2 \
        zlib \
        1
    collect_echo_component_notice_msys2 \
        mingw-w64-x86_64-libwinpthread
    collect_echo_component_notice_msys2 \
        mingw-w64-x86_64-crt
    collect_echo_component_notice_msys2 \
        mingw-w64-x86_64-libgcc
    collect_echo_component_notice_msys2 \
        mingw-w64-x86_64-libstdc++
    else
        for f in /etc/apt/sources.list /etc/apt/sources.list.d/*.list; do
            [ -f "$f" ] && \
            sudo sed -i '/^deb\b/{p;s/^deb/deb-src/}' "$f"
        done && \
        for f in /etc/apt/sources.list.d/*.sources; do
            [ -f "$f" ] && \
            sudo sed -i 's/^Types: deb$/Types: deb deb-src/' "$f"
        done && \
        sudo apt-get update || \
        exit -4
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
echo_append_notice "For more information about these licenses, please refer to the files in the \"licenses/\" directory."
