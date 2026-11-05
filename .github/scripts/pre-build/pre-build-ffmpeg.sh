#!/bin/sh

if ${BUILD_TARGET} == "windows"; then
    EXTRA_LIST=( --enable-dxva --enable-d3d11va --enable-d3d12va )
    EXTRA_CFLAGS="-O3 -ftree-vectorize -funroll-loops"
    EXTRA_LDFLAGS="-static-libgcc -static-libstdc++ -Wl,-Bstatic,--whole-archive -lwinpthread -lz -Wl,--no-whole-archive -Wl,-Bdynamic"
    else
        EXTRA_LIST=( --enable-vaapi --enable-vdpau )
        EXTRA_CFLAGS="-O3 -mtune=generic -ftree-vectorize -funroll-loops"
        EXTRA_LDFLAGS="-Wl,-Bdynamic"
fi

ffmpeg_build_path=${DEPENDS_BUILD_PREFIX}/${FFMPEG_LOCAL}/build && \
 \
git clone ${FFMPEG_REP} --branch ${FFMPEG_TAG} --depth 1 ./${FFMPEG_LOCAL} && \
 \
./configure \
--prefix=${ffmpeg_build_path} \
--bindir=${ffmpeg_build_path}/bin \
--shlibdir=${ffmpeg_build_path}/bin \
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
--enable-nvdec \
--enable-nvenc \
--enable-cuvid \
--enable-cuda \
--enable-hardcoded-tables \
"${COMMON[@]}" \
--optflags="-O3" \
--extra-cflags="${EXTRA_CFLAGS}" \
--extra-ldflags="${EXTRA_LDFLAGS}" \
&& make && make install DESTDIR=${ffmpeg_build_path} && \
mv ${ffmpeg_build_path}/bin/*.lib ${ffmpeg_build_path}/lib/