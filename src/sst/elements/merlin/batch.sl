#!/bin/bash

#SBATCH -q regular     
#SBATCH -N 1
#SBATCH -t 08:00:00     #time requested
#SBATCH -C haswell


module use $HOME
module load mod_sst
module load mod_sst

export OMP_NUM_THREADS=1

srun -N1 -n16 -c4 sst ./tests/dragon_16384_test.py 
