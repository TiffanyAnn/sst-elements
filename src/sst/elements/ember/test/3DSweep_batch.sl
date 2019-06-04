#!/bin/bash

#SBATCH -q regular    
#SBATCH -N 1
#SBATCH -t 07:30:00
#SBATCH -C haswell
#SBATCH -L SCRATCH
#SBATCH -J 3D_sweep
#SBATCH -o %x_%j.out


module use $HOME
module load mod_sst
module load mod_sst

export OMP_NUM_THREADS=1

srun -N1 -n8 -c8 sst ./emberLoad.py --model-options="--simConfig 3D_sweepSim --param merlin:merlinParams.rt_filename=${SCRATCH}/3d_sweep_DF_16384_rt.txt --param merlin:merlinParams.route=0"

srun -N1 -n8 -c8 sst ./emberLoad.py --model-options="--simConfig 3D_sweepSim --param merlin:merlinParams.rt_filename=${SCRATCH}/3d_sweep_DF_16384_rt.txt --param merlin:merlinParams.route=1"

srun -N1 -n8 -c8 sst ./emberLoad.py --model-options="--simConfig 3D_sweepSim --param merlin:merlinParams.rt_filename=${SCRATCH}/3d_sweep_DF_16384_rt.txt --param merlin:merlinParams.route=2"

srun -N1 -n8 -c8 sst ./emberLoad.py --model-options="--simConfig 3D_sweepSim --param merlin:merlinParams.rt_filename=${SCRATCH}/3d_sweep_DF_16384_rt.txt --param merlin:merlinParams.route=3" 
