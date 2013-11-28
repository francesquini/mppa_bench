#!/bin/bash

k1-jtag-runner --multibinary=bin/memcpy.mpk --exec-multibin=IODDR0:main > tmp.txt
cut -d " " -f3 tmp.txt > output/data_mppa.csv
rm tmp.txt
# k1-jtag-runner --multibinary=bin/memcpy.mpk --exec-multibin=IODDR0:main
