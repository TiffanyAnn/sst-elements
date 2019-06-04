#!/bin/bash

#SBATCH -q regular    
#SBATCH -N 1
#SBATCH -t 00:50:00
#SBATCH -C haswell
#SBATCH -J allReduceLarge
#SBATCH -o %x_%j.out


module use $HOME
module load mod_sst
module load mod_sst

export OMP_NUM_THREADS=1

srun -N1 -n32 -c2 sst ./emberLoad.py --model-options="--simConfig allReduce_largeSim --param merlin:merlinParams.route=1"

srun -N1 -n32 -c2 sst ./emberLoad.py --model-options="--simConfig allReduce_largeSim --param merlin:merlinParams.route=2"

srun -N1 -n32 -c2 sst ./emberLoad.py --model-options="--simConfig allReduce_largeSim --param merlin:merlinParams.route=3"
