#!/bin/bash

cd "${GITHUB_WORKSPACE}"

OPTION_LIST=(
    "-DWINDOWS_RUB_INCLUDE=\"${DEPENDS_BUILD_PREFIX}/${WINDOWSRUB_NAME}/include\""
    "-DCXXOPTS_ROOT=\"${DEPENDS_BUILD_PREFIX}/${CXXOPTS_NAME}\""
    "-DFFMPEG_ROOT=\"${DEPENDS_BUILD_PREFIX}/${FFMPEG_NAME}/${DEPENDS_BUILD_LOCAL}\""
)
[ "$BUILD_TARGET" == "windows" ] && \
OPTION_LIST+=( "-DCMAKE_EXE_LINKER_FLAGS_INIT=\"-static-libgcc -static-libstdc++\"" )
echo "${OPTION_LIST[@]}"

cmake -B "${BUILD_PATH}" \
"${OPTION_LIST[@]}" \
&& \
cmake --build "${BUILD_PATH}" --config Release