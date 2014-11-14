#!/bin/bash

k1-jtag-runner --multibinary=output/bin/noc-latency.img --exec-multibin=IODDR0:master 0$1 > tmp.txt
cut -d " " -f3 tmp.txt > result/data.csv
rm tmp.txt
