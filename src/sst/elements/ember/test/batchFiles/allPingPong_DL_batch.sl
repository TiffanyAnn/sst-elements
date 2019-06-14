#!/bin/bash

#SBATCH -q regular    
#SBATCH -N 2
#SBATCH -t 03:55:00
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
FILE="${HOME}/sst/sst-elements/src/sst/elements/ember/test/portFile/1percent_${i}"

srun -N2 -n32 -c2 sst ./emberLoad1.py --model-options="--simConfig allPingPongSim --stats allPingPong_1percent_${i} --param merlin:merlinParams.rt_filename=${SCRATCH}/allPingPong_DF_16384_rt.txt --param merlin:merlinParams.runtype=1 --param merlin:merlinParams.route=5 --param merlin:merlinParams.downPort_filename=${FILE}" >> allPingPong1pct.out

done

