#!/bin/bash

make clean &> /dev/null
make &> /dev/null

# run with different buffer sizes
echo "Running $0..."
first=1
for i in 4096 8192 16384 32768 65536 131072 262144 524288 1048576; do
	printf "\t Bytes per message = $i\n"
	./run.sh $i
	if [ "$first" = 1 ]; then
	    cat result/data.csv >> result/data_latency.csv
	    first=0
	else
	    sed '1d' result/data.csv >> result/data_latency.csv
	fi
done

rm -rf result/data.csv
