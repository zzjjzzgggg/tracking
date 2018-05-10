#!/bin/bash
# run this file in folder build

for eps in 0.1 0.15 0.2
do
    ./exam_hist_bg -graph ../../checkins/bri_L10Kp0.001.gz \
                   -budget 10 -batch_sz 1 -end_tm 5000 -eps $eps -L 10000
    ./exam_hist_bg -graph ../../checkins/gow_L10Kp0.001.gz \
                   -budget 10 -batch_sz 1 -end_tm 5000 -eps $eps -L 10000

    ./exam_hist_dg -graph ../../higgs/higgs_L1Kp0.001.gz \
                   -budget 10 -batch_sz 1 -end_tm 5000 -eps $eps -L 1000
    ./exam_hist_dg -graph ../../hk/hk_L1Kp0.001.gz \
                   -budget 10 -batch_sz 1 -end_tm 5000 -eps $eps -L 1000

    ./exam_hist_dg -graph ../../stackoverflow/c2q_L10Kp0.001.gz \
                   -budget 10 -batch_sz 1 -end_tm 5000 -eps $eps -L 10000
    ./exam_hist_dg -graph ../../stackoverflow/c2a_L10Kp0.001.gz \
                   -budget 10 -batch_sz 1 -end_tm 5000 -eps $eps -L 10000
done
