#!/bin/bash
# run this file in folder build

make exam_greedy_basic_bg

for k in {20..100..10}
do
    ./exam_greedy_basic_bg -graph ../../checkins/bri_L10Kp0.001.gz \
                           -budget $k -end_tm 5000 -L 10000
done
