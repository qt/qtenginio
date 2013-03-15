#!/bin/sh

export LD_LIBRARY_PATH=enginio-qt/enginio_client

for f in bin/tst_*; do
    if ! $f -maxwarnings 0; then
        echo $f failed
        exit 1
    fi
done

