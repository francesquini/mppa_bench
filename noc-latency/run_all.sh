#!/bin/bash

TYPE=USE_PORTAL make &> /dev/null

printf "Running benchmark with PORTAL communication...\n"
for i in 1 2 4 8 16; do
	printf "\t Number of clusters = $i\n"
	./run.sh $i
	mv output/data.csv output/portal-1-$i.csv
done

TYPE=USE_CHANNEL make &> /dev/null

printf "Running benchmark with CHANNEL communication...\n"
for i in 1 2 4 8 16; do
	printf "\t Number of clusters = $i\n"
	./run.sh $i
	mv output/data.csv output/channel-1-$i.csv
done
