#!/bin/bash
for i in $(seq 1 10); do
    #rm -fr txlvl.db/
    sync
    echo 3 > /proc/sys/vm/drop_caches
    ./fetch-lvl
done

