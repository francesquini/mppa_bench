#!/bin/bash

k1-jtag-runner --multibinary=noc-cost.mpk --exec-multibin=IODDR0:master 01 > tmp.txt
cut -d " " -f3 tmp.txt > data.csv
rm tmp.txt
