#!/bin/bash

k1-jtag-runner --multibinary=output/bin/memcpy.img --exec-multibin=IODDR0:master > tmp.txt
cut -d " " -f3 tmp.txt > output/data_mppa.csv
rm tmp.txt
