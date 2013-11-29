#!/bin/bash

printf "Running stream on cluster\n"
k1-jtag-runner --multibinary=stream_master_slave.mpk --exec-multibin=IODDR0:master

printf "Running stream on IO-node\n"
k1-jtag-runner --multibinary=stream_master.mpk --exec-multibin=IODDR0:stream
