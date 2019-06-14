#!/bin/bash

for i in `seq 6 10`;
do  
  shuf -n 79 global_ports_8k.txt | tr " " "\n" >> 1percent_8k_${i}.txt
  shuf -n 198 global_ports_8k.txt | tr " " "\n" >> 2.5percent_8k_${i}.txt
  shuf -n 397 global_ports_8k.txt | tr " " "\n" >> 5percent_8k_${i}.txt
done
