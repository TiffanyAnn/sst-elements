#!/bin/bash

#SBATCH -q debug     
#SBATCH -N 1
#SBATCH -t 00:30:00     #time requested
#SBATCH -C haswell
#SBATCH -L SCRATCH


module use $HOME
module load mod_sst
module load mod_sst

export OMP_NUM_THREADS=1

srun -N1 -n1 sst ./emberLoad.py
