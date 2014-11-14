#!/bin/bash

printf "Running stream on cluster\n"
k1-jtag-runner --multibinary=output/bin/stream_master_slave.img --exec-multibin=IODDR0:master

printf "Running stream on IO-node\n"
k1-jtag-runner --multibinary=output/bin/stream_master_single.img --exec-multibin=IODDR0:master_single
