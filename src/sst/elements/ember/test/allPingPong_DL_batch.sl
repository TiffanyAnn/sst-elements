#!/bin/bash

#SBATCH -q regular    
#SBATCH -N 2
#SBATCH -t 04:00:00
#SBATCH -C haswell
#SBATCH -J allPingPong
#SBATCH -o %x_%j.out
#SBATCH -L SCRATCH

module use $HOME
module load mod_sst
module load mod_sst

export OMP_NUM_THREADS=1

for i in `seq 1 10`;
do
FILE="/global/homes/t/tconnors/sst/sst-elements/src/sst/elements/ember/test/portFile/1percent_${i}"

srun -N2 -n32 -c2 sst ./emberLoad.py --model-options="--simConfig allPingPongSim --param merlin:merlinParams.runtype=1 --param merlin:merlinParams.route=5 --param merlin:merlinParams.downPort_filename=${FILE}" >> allPingPong1.out

cat /global/cscratch1/sd/tconnors/stats/stats_*.csv >> /global/cscratch1/sd/tconnors/stats/1percent_${i}stats.csv
rm /global/cscratch1/sd/tconnors/stats/stats_*.csv

done

#for i in `seq 1 10`;
#do
#FILE="/global/homes/t/tconnors/sst/sst-elements/src/sst/elements/ember/test/portFile/5percent_${i}"
#echo "-------------------------------------------------"

#srun -N1 -n32 -c2 sst ./emberLoad.py --model-options="--simConfig allPingPongSim --param merlin:merlinParams.runtype=1 --param merlin:merlinParams.route=5 --param merlin:merlinParams.downPort_filename=${FILE}" >> allPingPong5.out

#done

