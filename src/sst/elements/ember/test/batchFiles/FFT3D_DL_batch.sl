#!/bin/bash

#SBATCH -q regular    
#SBATCH -N 2
#SBATCH -t 03:30:00
#SBATCH -C haswell
#SBATCH -L SCRATCH
#SBATCH -J FFT3D
#SBATCH -o %x_%j.out


module use $HOME
module load mod_sst
module load mod_sst

export OMP_NUM_THREADS=1

for i in `seq 1 10`;
do
FILE="/global/homes/t/tconnors/sst/sst-elements/src/sst/elements/ember/test/portFile/1percent_${i}"

srun -N2 -n32 -c2 sst ./emberLoad1.py --model-options="--simConfig FFT3D_sim --stats FFT3D_1pct${i} --param merlin:merlinParams.rt_filename=${SCRATCH}/FFT3D_DF_16384_rt.txt --param merlin:merlinParams.runtype=1 --param merlin:merlinParams.route=5 --param merlin:merlinParams.downPort_filename=${FILE}" >> FFT3D1.out

done


