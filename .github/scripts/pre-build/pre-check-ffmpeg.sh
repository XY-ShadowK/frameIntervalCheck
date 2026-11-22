#!/bin/bash

echo "FFNVCODEC_CFLAG_NEED = ${FFNVCODEC_CFLAG_NEED} Length: ${#FFNVCODEC_CFLAG_NEED}"
FFNVCODEC_CFLAG="$(pkg-config --cflags ffnvcodec | xargs)"
echo "pkg-config: FFNVCODEC_CFLAG = ${FFNVCODEC_CFLAG} Length: ${#FFNVCODEC_CFLAG}"
[ "$FFNVCODEC_CFLAG" == "$FFNVCODEC_CFLAG_NEED" ] || \
exit -7
echo "pkg-config find right ffnvcodec."