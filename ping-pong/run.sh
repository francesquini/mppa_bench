#!/bin/bash

k1-jtag-runner --multibinary=output/bin/ping-pong.img --exec-multibin=IODDR0:master > tmp.txt
cut -d " " -f3 tmp.txt > result/data.csv
rm tmp.txt

