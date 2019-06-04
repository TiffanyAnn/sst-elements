#!/bin/bash

#SBATCH -q debug    
#SBATCH -N 1
#SBATCH -t 00:20:00     #time requested
#SBATCH -C haswell
#SBATCH -L SCRATCH
#SBATCH -J allReduceSmall
#SBATCH -o %x_%j.out


module use $HOME
module load mod_sst
module load mod_sst

export OMP_NUM_THREADS=1

srun -N1 -n32 -c2 sst ./emberLoad.py --model-options="--simConfig allReduce_smallSim --param merlin:merlinParams.rt_filename=${SCRATCH}/allReduceSmall_DF_16384_rt.txt --param merlin:merlinParams.route=0"

srun -N1 -n32 -c2 sst ./emberLoad.py --model-options="--simConfig allReduce_smallSim --param merlin:merlinParams.rt_filename=${SCRATCH}/allReduceSmall_DF_16384_rt.txt --param merlin:merlinParams.route=1"
 
srun -N1 -n32 -c2 sst ./emberLoad.py --model-options="--simConfig allReduce_smallSim --param merlin:merlinParams.rt_filename=${SCRATCH}/allReduceSmall_DF_16384_rt.txt --param merlin:merlinParams.route=2"

srun -N1 -n32 -c2 sst ./emberLoad.py --model-options="--simConfig allReduce_smallSim --param merlin:merlinParams.rt_filename=${SCRATCH}/allReduceSmall_DF_16384_rt.txt --param merlin:merlinParams.route=3"
