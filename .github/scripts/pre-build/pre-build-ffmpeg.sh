#!/bin/bash

export PKG_CONFIG_PATH="${DEPENDS_BUILD_PREFIX}/${FFNVCODEC_NAME}/build/lib/pkgconfig:$PKG_CONFIG_PATH"
echo "$PKG_CONFIG_PATH"

if [ "$BUILD_TARGET" == "windows" ]; then
    EXTRA_LIST=( --enable-dxva --enable-d3d11va --enable-d3d12va )
    EXTRA_CFLAGS="-O3 -ftree-vectorize -funroll-loops"
    EXTRA_LDFLAGS="-static-libgcc -static-libstdc++ -Wl,-Bstatic,--whole-archive -lwinpthread -lz -Wl,--no-whole-archive -Wl,-Bdynamic"
    FFNVCODEC_CFLAG_NEED="-I$(cygpath -w "${DEPENDS_BUILD_PREFIX}/${FFNVCODEC_NAME}/build/include" | tr "\\" "/")"
    else
        EXTRA_LIST=( --enable-vaapi --enable-vdpau )
        EXTRA_CFLAGS="-O3 -mtune=generic -ftree-vectorize -funroll-loops"
        EXTRA_LDFLAGS="-Wl,-Bdynamic"
        FFNVCODEC_CFLAG_NEED="-I${DEPENDS_BUILD_PREFIX}/${FFNVCODEC_NAME}/build/include"
fi

echo "FFNVCODEC_CFLAG_NEED = ${FFNVCODEC_CFLAG_NEED}"
FFNVCODEC_CFLAG="$(pkg-config --cflags ffnvcodec)"
echo "pkg-config: FFNVCODEC_CFLAG = ${FFNVCODEC_CFLAG}"
[ "$FFNVCODEC_CFLAG" == "$FFNVCODEC_CFLAG_NEED" ] || \
exit -7
echo "pkg-config find right ffnvcodec."

ffmpeg_build_path="${DEPENDS_BUILD_PREFIX}/${FFMPEG_NAME}/build" && \
 \
git clone "${FFMPEG_REPO}" --branch "${FFMPEG_TAG}" --depth 1 "${DEPENDS_BUILD_PREFIX}/${FFMPEG_NAME}" && \
 \
cd "${DEPENDS_BUILD_PREFIX}/${FFMPEG_NAME}" && \
./configure \
--prefix=${ffmpeg_build_path} \
--bindir=${ffmpeg_build_path}/bin \
--libdir=${ffmpeg_build_path}/lib \
--incdir=${ffmpeg_build_path}/include \
--pkgconfigdir=${ffmpeg_build_path}/lib/pkgconfig \
--disable-autodetect \
--enable-shared \
--disable-static \
--disable-gpl \
--disable-nonfree \
--disable-programs \
--disable-doc \
--enable-zlib \
--enable-ffnvcodec \
--enable-nvdec \
--enable-nvenc \
--enable-cuvid \
--enable-cuda \
--enable-hardcoded-tables \
"${COMMON[@]}" \
--optflags="-O3" \
--extra-cflags="${EXTRA_CFLAGS}" \
--extra-ldflags="${EXTRA_LDFLAGS}" \
&& make && make install || \
exit -8
echo "install ${FFMPEG_NAME} ${FFMPEG_TAG} ok."

# [ "$BUILD_TARGET" == "windows" ] || \
# return 0
# mv "${ffmpeg_build_path}/bin/*.lib" "${ffmpeg_build_path}/lib/" || \
# exit -9
# echo "move .lib files to /lib ok."