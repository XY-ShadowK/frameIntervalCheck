#!/bin/sh

if ! test $1; then
    exit -1
    else
        BUILD_TARGET=$1
fi

cd ${DEPENDS_BUILD_PREFIX}

# windowsRub pre-build
test $WINDOWSRUB_NEED_INSTALL && \
source ${SCRIPTS_DIR}/${PRE_BUILD_SCRIPTS_LOCAL}pre-build-header-only-commit.sh \
${WINDOWSRUB_REP} \
${WINDOWSRUB_COMMIT} \
${DEPENDS_BUILD_PREFIX}/${WINDOWSRUB_LOCAL}

# cxxopts pre-build
test $CXXOPTS_NEED_INSTALL && \
source ${SCRIPTS_DIR}/${PRE_BUILD_SCRIPTS_LOCAL}pre-build-header-only-commit.sh \
${CXXOPTS_REP} \
${CXXOPTS_COMMIT} \
${DEPENDS_BUILD_PREFIX}/${CXXOPTS_LOCAL}

# ffnvcodec pre-build
test $FFNVCODEC_NEED_INSTALL && \
source ${SCRIPTS_DIR}/${PRE_BUILD_SCRIPTS_LOCAL}pre-build-header-only-branch.sh \
${FFNVCODEC_REP} \
${FFNVCODEC_TAG} \
${DEPENDS_BUILD_PREFIX}/${FFNVCODEC_LOCAL} \
1

# ffmpeg pre-build
test $FFMPEG_NEED_INSTALL && source ${SCRIPTS_DIR}/${PRE_BUILD_SCRIPTS_LOCAL}pre-build-ffmpeg.sh