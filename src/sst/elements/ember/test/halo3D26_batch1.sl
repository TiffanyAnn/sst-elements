#!/bin/bash

#SBATCH -q regular  
#SBATCH -N 16
#SBATCH -t 18:00:00
#SBATCH -C haswell
#SBATCH -L SCRATCH
#SBATCH -J halo3D26
#SBATCH -o %x_%j.out


module use $HOME
module load mod_sst
module load mod_sst

export OMP_NUM_THREADS=1

time srun -N16 -n32 -c2 sst ./emberLoad.py --model-options="--simConfig halo3D26_sim --param merlin:merlinParams.rt_filename=${SCRATCH}/halo3D26_DF_16384_rt.txt --param:merlin:merlinParams.runtype=0 --param merlin:merlinParams.route=0"

#time srun -N16 -n32 -c2 sst ./emberLoad.py --model-options="--simConfig halo3D26_sim --param merlin:merlinParams.rt_filename=${SCRATCH}/halo3D26_DF_16384_rt.txt --param:merlin:merlinParams.runtype=0 --param merlin:merlinParams.route=1"

#time srun -N16 -n32 -c2 sst ./emberLoad.py --model-options="--simConfig halo3D26_sim --param merlin:merlinParams.rt_filename=${SCRATCH}/halo3D26_DF_16384_rt.txt --param:merlin:merlinParams.runtype=0 --param merlin:merlinParams.route=2"

#time srun -N16 -n32 -c2 sst ./emberLoad.py --model-options="--simConfig halo3D26_sim --param merlin:merlinParams.rt_filename=${SCRATCH}/halo3D26_DF_16384_rt.txt --param:merlin:merlinParams.runtype=0 --param merlin:merlinParams.route=3"

