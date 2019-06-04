#!/bin/bash


module use $HOME
module load mod_sst

export OMP_NUM_THREADS=1


#name of motif
app=allPingPong
#size of network being simulated
netSize=8k

echo "adaptive" >> ${netSize}_LF_${app}_base.out
srun -N2 -n32 -c2 sst ./emberLoad.py --model-options="--simConfig ${app}Sim --param merlin:merlinParams.rt_filename=${SCRATCH}/${app}_DF_${netSize}_rt1.txt --param merlin:merlinParams.runtype=2 --param merlin:merlinParams.route=5" >> ${netSize}_LF_${app}_base.out


################# 1 percent

#percentage of link failures
pct=1

for i in `seq 1 5`;
do
FILE="/global/homes/t/tconnors/sst/sst-elements/src/sst/elements/ember/test/portFiles/${pct}percent_${netSize}_${i}.txt"

#echo $i
srun -N2 -n32 -c2 sst ./emberLoad1.py --model-options="--simConfig ${app}Sim --stats ${netSize}_${app}_${pct}percent_${i} --param merlin:merlinParams.rt_filename=${SCRATCH}/${app}_DF_${netSize}_rt1.txt --param merlin:merlinParams.runtype=1 --param merlin:merlinParams.route=5 --param merlin:merlinParams.downPort_filename=${FILE}" >> ${netSize}_LF_${app}_${i}.out

#process stats output
cat /global/cscratch1/sd/tconnors/stats/${netSize}_${app}_${pct}percent_${i}stats*.csv >> /global/cscratch1/sd/tconnors/stats/${app}/${netSize}_${pct}pct_${app}_stats_${i}.csv
rm /global/cscratch1/sd/tconnors/stats/${netSize}_${app}_${pct}percent_${i}stats*.csv


grep "minPkts" ${SCRATCH}/stats/${app}/${netSize}_${pct}pct_${app}_stats_${i}.csv >> ${SCRATCH}/stats/${app}/${netSize}_${pct}pct_${app}_minPktsTotalStats_${i}.csv
grep "valPkts" ${SCRATCH}/stats/${app}/${netSize}_${pct}pct_${app}_stats_${i}.csv >> ${SCRATCH}/stats/${app}/${netSize}_${pct}pct_${app}_valPktsTotalstats_${i}.csv
grep "valBlockedPkts" ${SCRATCH}/stats/${app}/${netSize}_${pct}pct_${app}_stats_${i}.csv >> ${SCRATCH}/stats/${app}/${netSize}_${pct}pct_${app}_valPktsBlocked_${i}.csv
grep "minBlockedPkts" ${SCRATCH}/stats/${app}/${netSize}_${pct}pct_${app}_stats_${i}.csv >> ${SCRATCH}/stats/${app}/${netSize}_${pct}pct_${app}_minPktsBlocked_${i}.csv
grep "downLinksEncountered" ${SCRATCH}/stats/${app}/${netSize}_${pct}pct_${app}_stats_${i}.csv >> ${SCRATCH}/stats/${app}/${netSize}_${pct}pct_${app}_downLinksStats_${i}.csv
grep "totalPkts" ${SCRATCH}/stats/${app}/${netSize}_${pct}pct_${app}_stats_${i}.csv >> ${SCRATCH}/stats/${app}/${netSize}_${pct}pct_${app}_totalPktstats_${i}.csv

echo ${SCRATCH}/stats/${app}/${netSize}_${pct}pct_${app}_${i} >> ${SCRATCH}/stats/${app}/${netSize}_${pct}pct_${app}_totals.csv
echo " " >> ${SCRATCH}/stats/${app}/${netSize}_${pct}pct_${app}_totals.csv
awk -F"," '{x+=$7}END{print "minBlocked " x}' ${SCRATCH}/stats/${app}/${netSize}_${pct}pct_${app}_minPktsBlocked_${i}.csv >> ${SCRATCH}/stats/${app}/${netSize}_${pct}pct_${app}_totals.csv
awk -F"," '{x+=$7}END{print "valBlocked " x}'  ${SCRATCH}/stats/${app}/${netSize}_${pct}pct_${app}_valPktsBlocked_${i}.csv >> ${SCRATCH}/stats/${app}/${netSize}_${pct}pct_${app}_totals.csv
awk -F"," '{x+=$7}END{print "minPktsTotal " x}' ${SCRATCH}/stats/${app}/${netSize}_${pct}pct_${app}_minPktsTotalStats_${i}.csv >> ${SCRATCH}/stats/${app}/${netSize}_${pct}pct_${app}_totals.csv
awk -F"," '{x+=$7}END{print "valPktsTotal " x}' ${SCRATCH}/stats/${app}/${netSize}_${pct}pct_${app}_valPktsTotalstats_${i}.csv >> ${SCRATCH}/stats/${app}/${netSize}_${pct}pct_${app}_totals.csv
awk -F"," '{x+=$7}END{print "downLinksEncountered " x}' ${SCRATCH}/stats/${app}/${netSize}_${pct}pct_${app}_downLinksStats_${i}.csv >> ${SCRATCH}/stats/${app}/${netSize}_${pct}pct_${app}_totals.csv
awk -F"," '{x+=$7}END{print "totalPkts " x}' ${SCRATCH}/stats/${app}/${netSize}_${pct}pct_${app}_totalPktstats_${i}.csv >> ${SCRATCH}/stats/${app}/${netSize}_${pct}pct_${app}_totals.csv
echo "" >> ${SCRATCH}/stats/${app}/${netSize}_${pct}pct_${app}_totals.csv

done

################# 2.5 percent
#echo " "
#echo "-----------------------------------------------" >> LF_allPingPong.out
#echo "2.5 percent link failure" >> LF_allPingPong.out

#pct=2.5
#for i in `seq 1 5`;
#do
#FILE="/global/homes/t/tconnors/sst/sst-elements/src/sst/elements/ember/test/portFile/2.5percent_${i}.txt"

#echo $i
#srun -N2 -n32 -c2 sst ./emberLoad1.py --model-options="--simConfig allPingPongSim --stats allPingPong_1percent_${i} --param merlin:merlinParams.rt_filename=${SCRATCH}/allPingPong_DF_16384_rt.txt --param merlin:merlinParams.runtype=1 --param merlin:merlinParams.route=5 --param merlin:merlinParams.downPort_filename=${FILE}" >> LF_allPingPong.out

##process stats output
#cat /global/cscratch1/sd/tconnors/stats/${app}_${pct}percent_${i}stats*.csv >> /global/cscratch1/sd/tconnors/stats/${pct}pct_${app}_stats_${i}.csv
#rm /global/cscratch1/sd/tconnors/stats/${app}_${pct}percent_${i}stats*.csv

#done

################# 5 percent
#echo " "
#echo "-----------------------------------------------">> LF_allPingPong.out
#echo "5 percent link failure">> LF_allPingPong.out

#pct=5
#for i in `seq 1 10`;
#do
#FILE="/global/homes/t/tconnors/sst/sst-elements/src/sst/elements/ember/test/portFile/5percent_${i}.txt"

#echo $i
#srun -N2 -n32 -c2 sst ./emberLoad1.py --model-options="--simConfig allPingPongSim --stats allPingPong_1percent_${i} --param merlin:merlinParams.rt_filename=${SCRATCH}/allPingPong_DF_16384_rt.txt --param merlin:merlinParams.runtype=1 --param merlin:merlinParams.route=5 --param merlin:merlinParams.downPort_filename=${FILE}" >> LF_allPingPong.out

#process stats output
#cat /global/cscratch1/sd/tconnors/stats/${app}_${pct}percent_${i}stats*.csv >> /global/cscratch1/sd/tconnors/stats/${pct}pct_${app}_stats_${i}.csv
#rm /global/cscratch1/sd/tconnors/stats/${app}_${pct}percent_${i}stats*.csv


#done

