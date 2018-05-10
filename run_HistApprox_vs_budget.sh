#!/bin/bash
# run this file in folder build

make exam_hist_bg exam_hist_dg

eps=0.1
for k in {30..100..10}
do
    ./exam_hist_bg -graph ../../checkins/bri_L10Kp0.001.gz \
                   -budget $k -end_tm 5000 -eps $eps -L 10000

    continue
    ./exam_hist_bg -graph ../../checkins/gow_L10Kp0.001.gz \
                   -budget $k -batch_sz 1 -end_tm 5000 -eps $eps -L 10000

    ./exam_hist_dg -graph ../../higgs/higgs_L1Kp0.001.gz \
                   -budget $k -batch_sz 1 -end_tm 5000 -eps $eps -L 1000
    ./exam_hist_dg -graph ../../hk/hk_L1Kp0.001.gz \
                   -budget $k -batch_sz 1 -end_tm 5000 -eps $eps -L 1000

    ./exam_hist_dg -graph ../../stackoverflow/c2q_L10Kp0.001.gz \
                   -budget $k -batch_sz 1 -end_tm 5000 -eps $eps -L 10000
    ./exam_hist_dg -graph ../../stackoverflow/c2a_L10Kp0.001.gz \
                   -budget $k -batch_sz 1 -end_tm 5000 -eps $eps -L 10000
done
