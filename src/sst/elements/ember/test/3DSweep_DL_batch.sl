#!/bin/bash

#SBATCH -q regular    
#SBATCH -N 3
#SBATCH -t 07:25:00
#SBATCH -C haswell
#SBATCH -L SCRATCH
#SBATCH -J 3D_sweep
#SBATCH -o %x_%j.out


module use $HOME
module load mod_sst
module load mod_sst

export OMP_NUM_THREADS=1

for i in `seq 1 10`;
do
FILE="/global/homes/t/tconnors/sst/sst-elements/src/sst/elements/ember/test/portFile/1percent_${i}"

srun -N2 -n32 -c2 sst ./emberLoad.py --model-options="--simConfig 3D_sweepSim --param merlin:merlinParams.rt_filename=${SCRATCH}/3d_sweep_DF_4096_rt.txt --param merlin:merlinParams.runtype=0 --param merlin:merlinParams.route=0" >> 3DSweep_4096.out

done

