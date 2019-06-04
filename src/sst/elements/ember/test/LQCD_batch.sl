#!/bin/bash

#SBATCH -q regular    
#SBATCH -N 1
#SBATCH -t 02:00:00
#SBATCH -C haswell
#SBATCH -L SCRATCH
#SBATCH -J LQCD
#SBATCH -A nstaff
#SBATCH -o %x_%j.out.txt


module use $HOME
module load mod_sst
module load mod_sst

export OMP_NUM_THREADS=1

srun -N1 -n8 -c8 sst ./emberLoad.py --model-options="--simConfig LQCD_sim --param merlin:merlinParams.rt_filename=${SCRATCH}/LQCD_DF_16384_rt.txt --param merlin:merlinParams.route=0"

srun -N1 -n8 -c8 sst ./emberLoad.py --model-options="--simConfig LQCD_sim --param merlin:merlinParams.rt_filename=${SCRATCH}/LQCD_DF_16384_rt.txt --param merlin:merlinParams.route=1"

srun -N1 -n8 -c8 sst ./emberLoad.py --model-options="--simConfig LQCD_sim --param merlin:merlinParams.rt_filename=${SCRATCH}/LQCD_DF_16384_rt.txt --param merlin:merlinParams.route=2"

srun -N1 -n8 -c8 sst ./emberLoad.py --model-options="--simConfig LQCD_sim --param merlin:merlinParams.rt_filename=${SCRATCH}/LQCD_DF_16384_rt.txt --param merlin:merlinParams.route=3"


