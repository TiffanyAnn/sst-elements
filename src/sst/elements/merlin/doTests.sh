#!/bin/bash
if [ "$#" -ne "2" ]; then
	echo usage: `basename $0` '<ShiftEP or TestEP> <N Nodes>'
	exit
fi

EP=$1
Nnodes=$2

if [ $EP = 'ShiftEP' ]; then
	orig=/Users/tconnors/sst/sst-elements/src/sst/elements/merlin/test/simple_patterns/shift.orig
	srcFile=/Users/tconnors/sst/sst-elements/src/sst/elements/merlin/test/simple_patterns/shift.cc
	backUp=/Users/tconnors/sst/sst-elements/src/sst/elements/merlin/test/simple_patterns/shift.cc.orig
	outFile=/Users/tconnors/sst/sst-elements/src/sst/elements/merlin/tests/output/${Nnodes}_DF_shift1

elif [ $EP = 'TestEP' ]; then
	orig=/Users/tconnors/sst/sst-elements/src/sst/elements/merlin/test/nic.orig
	srcFile=/Users/tconnors/sst/sst-elements/src/sst/elements/merlin/test/nic.cc
	backUp=/Users/tconnors/sst/sst-elements/src/sst/elements/merlin/test/nic.cc.orig
	outFile=/Users/tconnors/sst/sst-elements/src/sst/elements/merlin/tests/output/${Nnodes}_DF_TestEndPoint
else
	echo Error: Unknown option $EP
	echo supported options for endpoint test simulations are ShiftEP or TestEP
	exit
fi
	echo
    echo using $EP with outfile `basename $outFile`
	echo
#switch to modified backup file
cp $backUp $srcFile

N=$(($Nnodes - 1))
#N=0
#M=2
#for each net_id/nodes we will run the experiment
for i in $( seq 0 $N )
do
	for j in $( seq 0 $N )
	do
    	#set the packet to track, based on net_id
    	perl -pi -e "s/TRACK_PACK/${i}/g" $srcFile
		perl -pi -e "s/TRACK_TRGT/${j}/g" $srcFile
		#sanity check output
		echo -n "Tracking: "
		grep "#define TRACK" $srcFile
		grep "#define TARGET" $srcFile
	
		OF=${outFile}_${i}

    	#build and run (assumes we are running from Merlin directory
		make >/dev/null
   		make install >/dev/null
   		sst ./tests/dragon_${Nnodes}_test2.py >> ${OF}
		echo
		echo Results written to $OF
		echo

		#just because I get sleepy and want a nap...or to slow things down.
		sleep 1s
	    #restore srcFile for next round
	    cp $backUp $srcFile
	done
done

#restore to original src file
cp $orig $srcFile
