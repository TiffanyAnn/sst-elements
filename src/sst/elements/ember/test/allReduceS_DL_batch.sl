#!/bin/bash

#SBATCH -q regular    
#SBATCH -N 2
#SBATCH -t 05:00:00
#SBATCH -C haswell
#SBATCH -J allReduceS
#SBATCH -o %x_%j.out
#SBATCH -L SCRATCH
#SBATCH -A nstaff


module use $HOME
module load mod_sst
module load mod_sst

export OMP_NUM_THREADS=1

app=allReduceS

echo "baseline" >> LF_allReduceSmall.out
srun -N2 -n32 -c2 sst ./emberLoad1.py --model-options="--simConfig allReduce_smallSim --stats allReduceS_test_${i} --param merlin:merlinParams.rt_filename=${SCRATCH}/allReduceSmall_DF_16384_rt.txt --param merlin:merlinParams.runtype=2 --param merlin:merlinParams.route=5" >> LF_allReduceSmall.out


################# 1 percent
echo "-----------------------------------------------" >> LF_allReduceSmall.out
echo "1 percent link failure" >> LF_allReduceSmall.out

pct=1
for i in `seq 1 10`;
do
FILE="/global/homes/t/tconnors/sst/sst-elements/src/sst/elements/ember/test/portFile/1percent_${i}.txt"

echo $i
srun -N2 -n32 -c2 sst ./emberLoad1.py --model-options="--simConfig allReduce_smallSim --stats allReduceS_1percent_${i} --param merlin:merlinParams.rt_filename=${SCRATCH}/allReduceSmall_DF_16384_rt.txt --param merlin:merlinParams.runtype=1 --param merlin:merlinParams.route=5 --param merlin:merlinParams.downPort_filename=${FILE}" >> LF_allReduceSmall.out

#process stats output
cat /global/cscratch1/sd/tconnors/stats/${app}_${pct}percent_${i}stats*.csv >> /global/cscratch1/sd/tconnors/stats/${pct}pct_${app}_stats_${i}.csv
rm /global/cscratch1/sd/tconnors/stats/${app}_${pct}percent_${i}stats*.csv


grep "minPkts" ${pct}pct_${app}_stats_${i}.csv >> ${pct}pct_${app}_minPktsTotalStats_${i}.csv
grep "valPkts" ${pct}pct_${app}_stats_${i}.csv >> ${pct}pct_${app}_valPktsTotalstats_${i}.csv
grep "valBlockedPkts" ${pct}pct_${app}_stats_${i}.csv >> ${pct}pct_${app}_valPktsBlocked_${i}.csv
grep "minBlockedPkts" ${pct}pct_${app}_stats_${i}.csv >> ${pct}pct_${app}_minPktsBlocked_${i}.csv
grep "downLinksEncountered" ${pct}pct_${app}_stats_${i}.csv >> ${pct}pct_${app}_downLinksStats_${i}.csv
grep "totalPkts" ${pct}pct_${app}_stats_${i}.csv >> ${pct}pct_${app}_totalPktstats_${i}.csv

echo ${pct}pct_${app}_${i} >> ${pct}pct_${app}_totals.csv
echo " " >> {pct}pct_${app}_totals.csv
awk -F"," '{x+=$7}END{print "minBlocked " x}' ${pct}pct_${app}_minPktsBlocked_${i}.csv >> ${pct}pct_${app}_totals.csv
awk -F"," '{x+=$7}END{print "valBlocked " x}'  ${pct}pct_${app}_valPktsBlocked_${i}.csv >> ${pct}pct_${app}_totals.csv
awk -F"," '{x+=$7}END{print "minPktsTotal " x}' ${pct}pct_${app}_minPktsTotalStats_${i}.csv >> ${pct}pct_${app}_totals.csv
awk -F"," '{x+=$7}END{print "valPktsTotal " x}' ${pct}pct_${app}_valPktsTotalstats_${i}.csv >> ${pct}pct_${app}_totals.csv
awk -F"," '{x+=$7}END{print "downLinksEncountered " x}' ${pct}pct_${app}_downLinksStats_${i}.csv >> ${pct}pct_${app}_totals.csv
awk -F"," '{x+=$7}END{print "totalPkts " x}' ${pct}pct_${app}_totalPktstats_${i}.csv >> ${pct}pct_${app}_totals.csv
echo "" >> {pct}pct_${app}_totals.csv

done

################# 2.5 percent
echo " "
echo "-----------------------------------------------" >> LF_allReduceSmall.out
echo "2.5 percent link failure" >> LF_allReduceSmall.out

pct=2.5
for i in `seq 1 10`;
do
FILE="/global/homes/t/tconnors/sst/sst-elements/src/sst/elements/ember/test/portFile/2.5percent_${i}.txt"

echo $i
srun -N2 -n32 -c2 sst ./emberLoad1.py --model-options="--simConfig allReduce_smallSim --stats allReduceS_1percent_${i} --param merlin:merlinParams.rt_filename=${SCRATCH}/allReduceSmall_DF_16384_rt.txt --param merlin:merlinParams.runtype=1 --param merlin:merlinParams.route=5 --param merlin:merlinParams.downPort_filename=${FILE}" >> LF_allReduceSmall.out

#process stats output
cat /global/cscratch1/sd/tconnors/stats/${app}_${pct}percent_${i}stats*.csv >> /global/cscratch1/sd/tconnors/stats/${pct}pct_${app}_stats_${i}.csv
rm /global/cscratch1/sd/tconnors/stats/${app}_${pct}percent_${i}stats*.csv


grep "minPkts" ${pct}pct_${app}_stats_${i}.csv >> ${pct}pct_${app}_minPktsTotalStats_${i}.csv
grep "valPkts" ${pct}pct_${app}_stats_${i}.csv >> ${pct}pct_${app}_valPktsTotalstats_${i}.csv
grep "valBlockedPkts" ${pct}pct_${app}_stats_${i}.csv >> ${pct}pct_${app}_valPktsBlocked_${i}.csv
grep "minBlockedPkts" ${pct}pct_${app}_stats_${i}.csv >> ${pct}pct_${app}_minPktsBlocked_${i}.csv
grep "downLinksEncountered" ${pct}pct_${app}_stats_${i}.csv >> ${pct}pct_${app}_downLinksStats_${i}.csv
grep "totalPkts" ${pct}pct_${app}_stats_${i}.csv >> ${pct}pct_${app}_totalPktstats_${i}.csv

echo ${pct}pct_${app}_${i} >> ${pct}pct_${app}_totals.csv
echo " " >> {pct}pct_${app}_totals.csv
awk -F"," '{x+=$7}END{print "minBlocked " x}' ${pct}pct_${app}_minPktsBlocked_${i}.csv >> ${pct}pct_${app}_totals.csv
awk -F"," '{x+=$7}END{print "valBlocked " x}'  ${pct}pct_${app}_valPktsBlocked_${i}.csv >> ${pct}pct_${app}_totals.csv
awk -F"," '{x+=$7}END{print "minPktsTotal " x}' ${pct}pct_${app}_minPktsTotalStats_${i}.csv >> ${pct}pct_${app}_totals.csv
awk -F"," '{x+=$7}END{print "valPktsTotal " x}' ${pct}pct_${app}_valPktsTotalstats_${i}.csv >> ${pct}pct_${app}_totals.csv
awk -F"," '{x+=$7}END{print "downLinksEncountered " x}' ${pct}pct_${app}_downLinksStats_${i}.csv >> ${pct}pct_${app}_totals.csv
awk -F"," '{x+=$7}END{print "totalPkts " x}' ${pct}pct_${app}_totalPktstats_${i}.csv >> ${pct}pct_${app}_totals.csv
echo "" >> {pct}pct_${app}_totals.csv

done

################# 5 percent
echo " "
echo "-----------------------------------------------">> LF_allReduceSmall.out
echo "5 percent link failure">> LF_allReduceSmall.out

pct=5
for i in `seq 1 10`;
do
FILE="/global/homes/t/tconnors/sst/sst-elements/src/sst/elements/ember/test/portFile/5percent_${i}.txt"

echo $i
srun -N2 -n32 -c2 sst ./emberLoad1.py --model-options="--simConfig allReduce_smallSim --stats allReduceS_1percent_${i} --param merlin:merlinParams.rt_filename=${SCRATCH}/allReduceSmall_DF_16384_rt.txt --param merlin:merlinParams.runtype=1 --param merlin:merlinParams.route=5 --param merlin:merlinParams.downPort_filename=${FILE}" >> LF_allReduceSmall.out

#process stats output
cat /global/cscratch1/sd/tconnors/stats/${app}_${pct}percent_${i}stats*.csv >> /global/cscratch1/sd/tconnors/stats/${pct}pct_${app}_stats_${i}.csv
rm /global/cscratch1/sd/tconnors/stats/${app}_${pct}percent_${i}stats*.csv


grep "minPkts" ${pct}pct_${app}_stats_${i}.csv >> ${pct}pct_${app}_minPktsTotalStats_${i}.csv
grep "valPkts" ${pct}pct_${app}_stats_${i}.csv >> ${pct}pct_${app}_valPktsTotalstats_${i}.csv
grep "valBlockedPkts" ${pct}pct_${app}_stats_${i}.csv >> ${pct}pct_${app}_valPktsBlocked_${i}.csv
grep "minBlockedPkts" ${pct}pct_${app}_stats_${i}.csv >> ${pct}pct_${app}_minPktsBlocked_${i}.csv
grep "downLinksEncountered" ${pct}pct_${app}_stats_${i}.csv >> ${pct}pct_${app}_downLinksStats_${i}.csv
grep "totalPkts" ${pct}pct_${app}_stats_${i}.csv >> ${pct}pct_${app}_totalPktstats_${i}.csv

echo ${pct}pct_${app}_${i} >> ${pct}pct_${app}_totals.csv
echo " " >> {pct}pct_${app}_totals.csv
awk -F"," '{x+=$7}END{print "minBlocked " x}' ${pct}pct_${app}_minPktsBlocked_${i}.csv >> ${pct}pct_${app}_totals.csv
awk -F"," '{x+=$7}END{print "valBlocked " x}'  ${pct}pct_${app}_valPktsBlocked_${i}.csv >> ${pct}pct_${app}_totals.csv
awk -F"," '{x+=$7}END{print "minPktsTotal " x}' ${pct}pct_${app}_minPktsTotalStats_${i}.csv >> ${pct}pct_${app}_totals.csv
awk -F"," '{x+=$7}END{print "valPktsTotal " x}' ${pct}pct_${app}_valPktsTotalstats_${i}.csv >> ${pct}pct_${app}_totals.csv
awk -F"," '{x+=$7}END{print "downLinksEncountered " x}' ${pct}pct_${app}_downLinksStats_${i}.csv >> ${pct}pct_${app}_totals.csv
awk -F"," '{x+=$7}END{print "totalPkts " x}' ${pct}pct_${app}_totalPktstats_${i}.csv >> ${pct}pct_${app}_totals.csv
echo "" >> {pct}pct_${app}_totals.csv

done

