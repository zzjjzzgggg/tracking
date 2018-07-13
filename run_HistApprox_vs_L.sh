#!/bin/bash
# run this file in folder build

for L in {90..100..10}
do
    ./exam_hist_bg -graph ../../checkins/bri_L${L}Kp0.001.gz \
                   -budget 10 -end_tm 5000 -eps 0.2 -L ${L}000
    ./exam_hist_bg -graph ../../checkins/gow_L${L}Kp0.001.gz \
                   -budget 10 -end_tm 5000 -eps 0.2 -L ${L}000
done
