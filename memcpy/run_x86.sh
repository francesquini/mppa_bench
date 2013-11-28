#!/bin/bash

./bin/memcpy > tmp.txt
cut -d " " -f3 tmp.txt > output/data_x86.csv
rm tmp.txt

# ./bin/memcpy