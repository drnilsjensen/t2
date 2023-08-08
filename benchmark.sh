#!/bin/bash
for i in $(seq 0 1000);
do
    echo "$(($i*1000))"
    date; ./benchmark; date
done
