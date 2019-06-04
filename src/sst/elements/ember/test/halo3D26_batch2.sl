#!/bin/bash

#SBATCH -q regular    
#SBATCH -N 1
#SBATCH -t 28:59:59
#SBATCH -C haswell
#SBATCH -L SCRATCH
#SBATCH -J halo3D26
#SBATCH -o %x_%j.out


module use $HOME
module load mod_sst
module load mod_sst

export OMP_NUM_THREADS=1

time srun -N1 -n32 -c2 sst ./emberLoad.py --model-options="--simConfig halo3D26_sim --param merlin:merlinParams.rt_filename=${SCRATCH}/halo3D26_DF_16384_rt.txt --param merlin:merlinParams.route=2"

time srun -N1 -n32 -c2 sst ./emberLoad.py --model-options="--simConfig halo3D26_sim --param merlin:merlinParams.rt_filename=${SCRATCH}/halo3D26_DF_16384_rt.txt --param merlin:merlinParams.route=3"

