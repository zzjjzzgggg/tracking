#!/bin/bash
# run this file in folder build

for L in {20..80..10}
do
    ./exam_greedy_basic_bg -graph ../../checkins/bri_L${L}Kp0.001.gz \
                           -budget 10 -end_tm 5000 -L ${L}000
done
