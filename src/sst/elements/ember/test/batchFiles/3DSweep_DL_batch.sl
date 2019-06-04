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

srun -N3 -n32 -c2 sst ./emberLoad1.py --model-options="--simConfig 3D_sweepSim --stats 3Dsweep_1pct_${i} --param merlin:merlinParams.rt_filename=${SCRATCH}/3d_sweep_DF_16384_rt.txt --param merlin:merlinParams.runtype=1 --param merlin:merlinParams.route=5 --param merlin:merlinParams.downPort_filename=${FILE}" >> 3DSweep1.out

done

