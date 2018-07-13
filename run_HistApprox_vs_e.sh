#!/bin/bash
# run this file in folder build

make exam_hist_bg exam_hist_dg

for e in 0.1 0.2 0.3
do
    # ./exam_hist_bg -graph ../../checkins/bri_L10Kp0.001.gz \
    #                -budget 10 -end_tm 5000 -eps $e -L 10000
    # ./exam_hist_bg -graph ../../checkins/gow_L10Kp0.001.gz \
    #                -budget 10 -end_tm 5000 -eps $e -L 10000

    ./exam_hist_dg -graph ../../higgs/higgs_L1Kp0.001.gz \
                   -budget 10 -end_tm 5000 -eps $e -L 1000
    ./exam_hist_dg -graph ../../hk/hk_L1Kp0.001.gz \
                   -budget 10 -end_tm 5000 -eps $e -L 1000

    ./exam_hist_dg -graph ../../stackoverflow/c2q_L10Kp0.001.gz \
                   -budget 10 -end_tm 5000 -eps $e -L 10000
    ./exam_hist_dg -graph ../../stackoverflow/c2a_L10Kp0.001.gz \
                   -budget 10 -end_tm 5000 -eps $e -L 10000
done
