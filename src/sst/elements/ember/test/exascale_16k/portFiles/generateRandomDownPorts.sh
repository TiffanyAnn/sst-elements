#!/bin/bash

for i in `seq 1 10`;
do  
  shuf -n 79 global_ports.txt | tr " " "\n" >> 1percent_${i}.txt
  shuf -n 198 global_ports.txt | tr " " "\n" >> 2.5percent_${i}.txt
  shuf -n 396 global_ports.txt | tr " " "\n" >> 5percent_${i}.txt
  shuf -n 793 global_ports.txt | tr " " "\n" >> 10percent_${i}.txt
done
