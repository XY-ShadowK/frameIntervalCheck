#!/bin/bash

mkdir -p "${DEPENDS_BUILD_PREFIX}"
cd "${DEPENDS_BUILD_PREFIX}"
echo "pre-build start."

# windowsRub pre-build
test "$WINDOWSRUB_NEED_INSTALL" && \
[ "$WINDOWSRUB_NEED_INSTALL" -eq "1" ] && \
source "${SCRIPTS_PATH}/${PRE_BUILD_SCRIPTS_LOCAL}/pre-build-header-only-commit.sh" \
"${WINDOWSRUB_REPO}" \
"${WINDOWSRUB_COMMIT}" \
"${DEPENDS_BUILD_PREFIX}/${WINDOWSRUB_NAME}"

# cxxopts pre-build
test "$CXXOPTS_NEED_INSTALL" && \
[ "$CXXOPTS_NEED_INSTALL" -eq "1" ] && \
source "${SCRIPTS_PATH}/${PRE_BUILD_SCRIPTS_LOCAL}/pre-build-header-only-commit.sh" \
"${CXXOPTS_REPO}" \
"${CXXOPTS_COMMIT}" \
"${DEPENDS_BUILD_PREFIX}/${CXXOPTS_NAME}"

# ffnvcodec pre-build
test "$FFNVCODEC_NEED_INSTALL" && \
[ "$FFNVCODEC_NEED_INSTALL" -eq "1" ] && \
source "${SCRIPTS_PATH}/${PRE_BUILD_SCRIPTS_LOCAL}/pre-build-header-only-branch.sh" \
"${FFNVCODEC_REPO}" \
"${FFNVCODEC_TAG}" \
"${DEPENDS_BUILD_PREFIX}/${FFNVCODEC_NAME}" \
1

# ffmpeg pre-build
test "$FFMPEG_NEED_INSTALL" && \
[ "$FFMPEG_NEED_INSTALL" -eq "1" ] && \
source "${SCRIPTS_PATH}/${PRE_BUILD_SCRIPTS_LOCAL}/pre-build-ffmpeg.sh"

echo "pre-build end."