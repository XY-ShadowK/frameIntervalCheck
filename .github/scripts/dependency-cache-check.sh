#!/bin/bash

if ! (test "$1") || ! (test "$2") || ! (test "$3"); then
    exit -1
fi

if [ -d "$1" ]; then
    echo "$2 installed by cache."
    else
        echo "$3=1" >> "$GITHUB_ENV"
fi