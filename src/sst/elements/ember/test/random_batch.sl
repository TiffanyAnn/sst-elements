#!/bin/bash

#SBATCH -q regular    
#SBATCH -N 1
#SBATCH -t 01:30:00
#SBATCH -C haswell
#SBATCH -J random
#SBATCH -o %x_%j.out


module use $HOME
module load mod_sst
module load mod_sst

export OMP_NUM_THREADS=1

time srun -N1 -n32 -c2 sst ./emberLoad.py --model-options='--simConfig randomSim --param merlin:merlinParams.route=5 '

