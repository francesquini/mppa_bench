#!/bin/bash

k1-jtag-runner --multibinary=bin/noc-latency.mpk --exec-multibin=IODDR0:master 0$1 > tmp.txt
cut -d " " -f3 tmp.txt > output/data.csv
rm tmp.txt
# k1-jtag-runner --multibinary=bin/noc-latency.mpk --exec-multibin=IODDR0:master 0$1
