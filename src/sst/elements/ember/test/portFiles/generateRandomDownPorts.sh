#!/bin/bash

for i in `seq 1 10`;
do  
  shuf -n 238 ports.txt | tr " " "\n" >> 1percent_${i}.txt
  shuf -n 1190 ports.txt | tr " " "\n" >> 5percent_${i}.txt
  shuf -n 2380 ports.txt | tr " " "\n" >> 10percent_${i}.txt
done
