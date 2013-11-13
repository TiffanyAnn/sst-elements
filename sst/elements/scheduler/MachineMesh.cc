// Copyright 2009-2013 Sandia Corporation. Under the terms
// of Contract DE-AC04-94AL85000 with Sandia Corporation, the U.S.
// Government retains certain rights in this software.
// 
// Copyright (c) 2009-2013, Sandia Corporation
// All rights reserved.
// 
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

/*
 * Machine based on a mesh structure
 */
#include "sst_config.h"
#include "MachineMesh.h"

#include <sst_config.h>

#include <vector>
#include <string>
#include <sstream>

#include "sst/core/serialization.h"

#include "Allocator.h"
#include "Machine.h"
#include "MeshAllocInfo.h"
#include "misc.h"
#include "schedComponent.h"

using namespace SST::Scheduler;

namespace SST {
    namespace Scheduler {
        class MeshLocation;
        class MeshAllocInfo;
    }
}


//constructor that takes mesh dimensions
MachineMesh::MachineMesh(int Xdim, int Ydim, int Zdim, schedComponent* sc) 
{
    schedout.init("", 8, 0, Output::STDOUT);
    xdim = Xdim;
    ydim = Ydim;
    zdim = Zdim;
    numProcs = Xdim * Ydim;
    numProcs *= Zdim;
    isFree.resize(xdim);
    for (int i = 0; i < xdim; i++) {
        isFree[i].resize(ydim);
        for (int j = 0; j < (ydim); j++) {
            isFree[i][j].resize(zdim);
            for (int k = 0; k < zdim; k++) {
                isFree[i][j][k] = true;
            }
        }
    }
    this -> sc = sc;
    reset();
    //set up output
}


//constructor for testing
//takes array telling which processors are free
MachineMesh::MachineMesh(MachineMesh* inmesh) 
{
    schedout.init("", 8, 0, Output::STDOUT);
    isFree = inmesh -> isFree;
    xdim = inmesh -> getXDim();
    ydim = inmesh -> getYDim();
    zdim = inmesh -> getZDim();

    numAvail = 0;
    for (int i = 0; i < xdim; i++) {
        for (int j = 0; j < ydim; j++) {
            for (int k = 0; k < zdim; k++) {
                if (isFree[i][j][k]) {
                    numAvail++;
                }
            }
        }
    }
}

std::string MachineMesh::getParamHelp() 
{
    return "[<x dim>,<y dim>,<z dim>]\n\t3D Mesh with specified dimensions";
}

std::string MachineMesh::getSetupInfo(bool comment)
{
    std::string com;
    if (comment) com="# ";
    else com="";
    std::stringstream ret;
    ret << com<<xdim<<"x"<<ydim<<"x"<<zdim<<" Mesh";
    return ret.str();
}

int MachineMesh::getXDim() 
{
    return xdim;
}

int MachineMesh::getYDim() 
{
    return ydim;
}

int MachineMesh::getZDim() 
{
    return zdim;
}

int MachineMesh::getMachSize() 
{
    return xdim*ydim*zdim;
}

void MachineMesh::reset() 
{
    numAvail = xdim * ydim * zdim;
    for (int i = 0; i < xdim; i++) {
        for (int j = 0; j < ydim; j++) {
            for (int k = 0; k < zdim; k++) {
                isFree[i][j][k] = true;
            }
        }
    }
}

//returns list of free processors
std::vector<MeshLocation*>* MachineMesh::freeProcessors() 
{
    std::vector<MeshLocation*>* retVal = new std::vector<MeshLocation*>();
    for (int i = 0; i < xdim; i++) {
        for (int j = 0; j < ydim; j++) {
            for (int k = 0; k < zdim; k++) {
                if (isFree[i][j][k]) {
                    retVal -> push_back(new MeshLocation(i,j,k));
                }
            }
        }
    }
    return retVal;
}

//returns list of used processors
std::vector<MeshLocation*>* MachineMesh::usedProcessors() 
{
    std::vector<MeshLocation*>* retVal = new std::vector<MeshLocation*>();
    for (int i = 0; i < xdim; i++) {
        for (int j = 0; j < ydim; j++) {
            for (int k = 0; k < zdim; k++) {
                if (!isFree[i][j][k]) {
                    retVal -> push_back(new MeshLocation(i,j,k));
                }
            }
        }
    }
    return retVal;
}

//allocate list of processors in allocInfo
void MachineMesh::allocate(AllocInfo* allocInfo) 
{
    std::vector<MeshLocation*>* procs = ((MeshAllocInfo*)allocInfo) -> processors;
    //machinemesh (unlike simplemachine) is not responsible for setting
    //which processors are used in allocInfo as it's been set by the
    //allocator already

    for (unsigned int i = 0; i < procs -> size(); i++) {
        if (!isFree[((*procs)[i]) -> x][((*procs)[i]) -> y][((*procs)[i]) -> z]) {
            schedout.fatal(CALL_INFO, 0, "Attempt to allocate a busy processor: " );
        }
        isFree[((*procs)[i]) -> x][((*procs)[i]) -> y][((*procs)[i]) -> z] = false;
    }
    numAvail  -= procs-> size();
    sc -> startJob(allocInfo);
}

void MachineMesh::deallocate(AllocInfo* allocInfo) {
    //deallocate list of processors in allocInfo

    std::vector<MeshLocation*>* procs = ((MeshAllocInfo*)allocInfo) -> processors;

    for (unsigned int i = 0; i < procs -> size(); i++) {
        if (isFree[((*procs)[i]) -> x][((*procs)[i]) -> y][((*procs)[i]) -> z]) {
            schedout.fatal(CALL_INFO, 0, "Attempt to allocate a busy processor: " );
        }
        isFree[((*procs)[i]) -> x][((*procs)[i]) -> y][((*procs)[i]) -> z] = true;
    }
    numAvail += procs -> size();
}

long MachineMesh::pairwiseL1Distance(std::vector<MeshLocation*>* locs) {
    //returns total pairwise L_1 distance between all array members
    return pairwiseL1Distance(locs, locs -> size());
}

long MachineMesh::pairwiseL1Distance(std::vector<MeshLocation*>* locs, int num) {
    //returns total pairwise L_1 distance between 1st num members of array
    long retVal = 0;

	//std::cout<<"Allocation decision for a job needs "<<num<<" nodes:"<<std::endl;	
    for (int i = 0; i < num; i++) {

		//Jie: start editing Oct2013	
		/*
		std::cout<<"node "<<i<<"; location x:"<<(*locs)[i]->x<<std::endl;	
		std::cout<<"node "<<i<<"; location y:"<<(*locs)[i]->y<<std::endl;	
		std::cout<<"node "<<i<<"; location z:"<<(*locs)[i]->z<<std::endl<<std::endl;	
		*/
		//Jie: end editing Oct2013	

        for (int j = i + 1; j < num; j++) {
            retVal += ((*locs)[i]) -> L1DistanceTo((*locs)[j]);
        }
    }

	//std::cout<<"Hello: IsFree[0][0]="<<isFree[0][0].size()<<"; isFree[0]="<<isFree[0].size()<<"; isFree="<<isFree.size()<<std::endl;

	std::string tempstring= "";
	for (unsigned int k = 0; k < isFree[0][0].size(); k++) {
		tempstring= "";
		for (int j = isFree[0].size() - 1; j >= 0; j--) {
			//std::cout<<"Hello: k="<<k<<"; j="<<j<<std::endl;
			for (unsigned int i = 0; i < isFree.size(); i++) {
				if(isFree[i][j][k]) {
					tempstring += "0";
                                } else {
					tempstring += "1";
                                }
                        }
			tempstring += "\n";
		}
    	//std::cout<<tempstring;
	}

  return retVal;
}


//Jie: start editing Oct2013	
double MachineMesh::getCoolingPower(std::vector<MeshLocation*>* locs) {


	int Putil=2000;
	int Pidle=1000;
	int busynodes=0;
	int i, j, k;

	double Tsup=15;
	double Tred=30;
	double sum_inlet=0;
	double max_inlet=0;

	double COP;
	double Pcompute;
	double Pcooling;
	double Scaling_Factor;

	std::vector<bool> allocation_index;
	std::vector<double> inlet_temperature;
	
	double DMatrix[40][40]=
	{
		{
		0.0005209651,0.0001345855,0.0000232228,0.0000000000,0.0002826398,0.0000883775,0.0000001206,0.0000000000,0.0001155686,0.0000288523,
		0.0000018381,0.0000212211,0.0000319132,0.0000429967,0.0000164412,0.0000515659,0.0000403409,0.0000787338,0.0000513258,0.0000670125,
		0.0001084163,0.0000290037,0.0000010477,0.0000000000,0.0001208821,0.0000672134,0.0000000109,0.0000000000,0.0000737144,0.0000287233,
		0.0000025074,0.0000001561,0.0000282411,0.0000416313,0.0000149163,0.0000300671,0.0000289692,0.0000293222,0.0000263615,0.0000421316
		},{
		0.0005874871,0.0002362845,0.0000617478,0.0000000000,0.0003192660,0.0001309955,0.0000003207,0.0000000000,0.0001277082,0.0000707700,
		0.0000205822,0.0000222948,0.0000920144,0.0000596966,0.0000624021,0.0000949641,0.0000899400,0.0001063615,0.0000902499,0.0001028319,
		0.0001450287,0.0000715985,0.0000019290,0.0000000000,0.0002067594,0.0000909727,0.0000000201,0.0000000000,0.0001194755,0.0000706224,
		0.0000220527,0.0000003819,0.0000674145,0.0000576189,0.0000425085,0.0000364244,0.0000523128,0.0000503959,0.0000614802,0.0000559355
		},{
		0.0006529260,0.0004166718,0.0001198895,0.0000000000,0.0004193154,0.0001780097,0.0000183185,0.0000000000,0.0001907544,0.0000751731,
		0.0000035981,0.0000238735,0.0000633959,0.0000530538,0.0000242801,0.0000627369,0.0000539398,0.0000767992,0.0000830699,0.0000817493,
		0.0002297625,0.0001369720,0.0000034346,0.0000000000,0.0002857298,0.0001362108,0.0000000359,0.0000000000,0.0001287663,0.0000750362,
		0.0000232408,0.0000007292,0.0000582852,0.0000519724,0.0000401141,0.0000382215,0.0000412158,0.0000422328,0.0000383531,0.0000546028
		},{
		0.0003741471,0.0002539244,0.0000976087,0.0000000000,0.0002224031,0.0001118427,0.0000358987,0.0000000000,0.0001108929,0.0000734910,
		0.0000392991,0.0000391606,0.0001140659,0.0000918905,0.0001086797,0.0001172189,0.0001363953,0.0001310131,0.0001289619,0.0001721749,
		0.0001054190,0.0000739943,0.0000201166,0.0000000000,0.0001513733,0.0000899829,0.0000179983,0.0000000000,0.0000857797,0.0000558063,
		0.0000234624,0.0000004888,0.0000682637,0.0000889288,0.0000697975,0.0000583028,0.0000715833,0.0000679459,0.0000775416,0.0000669727
		},{
		0.0000612065,0.0000020970,0.0000003618,0.0000000000,0.0000044038,0.0000013770,0.0000000019,0.0000000000,0.0000018007,0.0000004495,
		0.0000000286,0.0000003306,0.0000004972,0.0000006699,0.0000002562,0.0000008034,0.0000006286,0.0000012268,0.0000007997,0.0000010441,
		0.0000016892,0.0000004519,0.0000000163,0.0000000000,0.0000018835,0.0000010473,0.0000000002,0.0000000000,0.0000011485,0.0000004475,
		0.0000000391,0.0000000024,0.0000004400,0.0000006487,0.0000002324,0.0000004685,0.0000004514,0.0000004569,0.0000004107,0.0000006565
		},{
		0.0001294669,0.0000420727,0.0000013707,0.0000000000,0.0000652818,0.0000041361,0.0000000071,0.0000000000,0.0000070837,0.0000031664,
		0.0000005665,0.0000008980,0.0000123687,0.0000086352,0.0000054214,0.0000047538,0.0000344267,0.0000157062,0.0000289952,0.0000274538,
		0.0000049139,0.0000016546,0.0000000530,0.0000000000,0.0000059440,0.0000030558,0.0000000006,0.0000000000,0.0000052097,0.0000030556,
		0.0000007723,0.0000000089,0.0000101832,0.0000079883,0.0000045955,0.0000030021,0.0000100652,0.0000089210,0.0000076482,0.0000060258
		},{
		0.0002585374,0.0001630155,0.0000222732,0.0000000000,0.0001421948,0.0000658687,0.0000001157,0.0000000000,0.0000754209,0.0000113185,
		0.0000022089,0.0000023050,0.0000597280,0.0000477447,0.0000376004,0.0000510456,0.0000724608,0.0000872986,0.0000751713,0.0000876205,
		0.0000637958,0.0000452731,0.0000010701,0.0000000000,0.0000952450,0.0000457161,0.0000000112,0.0000000000,0.0000532422,0.0000287847,
		0.0000031047,0.0000002407,0.0000359819,0.0000456382,0.0000356555,0.0000119967,0.0000364592,0.0000349184,0.0000308240,0.0000429990
		},{
		0.0002874105,0.0001690015,0.0000591520,0.0000000000,0.0001488012,0.0000685346,0.0000180031,0.0000000000,0.0000807848,0.0000493822,
		0.0000203007,0.0000203735,0.0000735070,0.0000570475,0.0000612014,0.0000546018,0.0001073648,0.0001023454,0.0000865063,0.0001147975,
		0.0000675165,0.0000470983,0.0000011194,0.0000000000,0.0001170812,0.0000477327,0.0000000117,0.0000000000,0.0000575679,0.0000316364,
		0.0000039058,0.0000002504,0.0000664961,0.0000550106,0.0000412046,0.0000329193,0.0000516804,0.0000479764,0.0000588782,0.0000509140
		},{
		0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,
		0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,
		0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,
		0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000
		},{
		0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,
		0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,
		0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,
		0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000
		},{
		0.0000204031,0.0000006990,0.0000001206,0.0000000000,0.0000014681,0.0000004590,0.0000000006,0.0000000000,0.0000019242,0.0000011276,
		0.0000002738,0.0000001102,0.0000063588,0.0000045872,0.0000030850,0.0000017834,0.0000088368,0.0000260019,0.0000061790,0.0000228489,
		0.0000005650,0.0000001507,0.0000000055,0.0000000000,0.0000006280,0.0000003491,0.0000000001,0.0000000000,0.0000013998,0.0000011175,
		0.0000003293,0.0000000008,0.0000053663,0.0000040424,0.0000026341,0.0000013938,0.0000054455,0.0000051012,0.0000042406,0.0000031568
		},{
		0.0000438597,0.0000203212,0.0000005619,0.0000000000,0.0000045945,0.0000015985,0.0000000029,0.0000000000,0.0000082406,0.0000050582,
		0.0000010160,0.0000003362,0.0000318475,0.0000208295,0.0000307766,0.0000067841,0.0000794083,0.0000723794,0.0000611969,0.0000553281,
		0.0000018873,0.0000006734,0.0000000209,0.0000000000,0.0000023302,0.0000011708,0.0000000002,0.0000000000,0.0000061977,0.0000048464,
		0.0000014461,0.0000000036,0.0000268764,0.0000188970,0.0000116822,0.0000056984,0.0000271196,0.0000243712,0.0000204123,0.0000323107
		},{
		0.0000000021,0.0000000001,0.0000000000,0.0000000000,0.0000000003,0.0000000001,0.0000000000,0.0000000000,0.0000127654,0.0000067797,
		0.0000003366,0.0000000000,0.0001343023,0.0000683882,0.0000156239,0.0000046021,0.0002524887,0.0001311296,0.0000478386,0.0000185633,
		0.0000000043,0.0000000001,0.0000000000,0.0000000000,0.0000000004,0.0000000001,0.0000000000,0.0000000000,0.0000093913,0.0000066595,
		0.0000007134,0.0000000000,0.0000704611,0.0000474554,0.0000141114,0.0000035037,0.0000733663,0.0000575521,0.0000247110,0.0000138469
		},{
		0.0000000031,0.0000000001,0.0000000000,0.0000000000,0.0000000004,0.0000000001,0.0000000000,0.0000000000,0.0000082602,0.0000049892,
		0.0000004930,0.0000000000,0.0000948347,0.0000415165,0.0000126129,0.0000045308,0.0001298256,0.0000954165,0.0000421487,0.0000343272,
		0.0000000056,0.0000000002,0.0000000000,0.0000000000,0.0000000005,0.0000000001,0.0000000000,0.0000000000,0.0000064934,0.0000049722,
		0.0000009138,0.0000000000,0.0000713601,0.0000397470,0.0000115410,0.0000040182,0.0000396387,0.0000487157,0.0000388697,0.0000131405
		},{
		0.0000000097,0.0000000004,0.0000000001,0.0000000000,0.0000000012,0.0000000004,0.0000000000,0.0000000000,0.0000197052,0.0000125836,
		0.0000015622,0.0000000001,0.0001688798,0.0001110254,0.0000502401,0.0000128479,0.0002758639,0.0001900926,0.0001346245,0.0001159810,
		0.0000000166,0.0000000005,0.0000000001,0.0000000000,0.0000000015,0.0000000004,0.0000000000,0.0000000000,0.0000161860,0.0000123494,
		0.0000027263,0.0000000000,0.0001225990,0.0000903703,0.0000297645,0.0000114938,0.0001332627,0.0001123698,0.0000895263,0.0000344179
		},{
		0.0000000295,0.0000000013,0.0000000002,0.0000000000,0.0000000034,0.0000000012,0.0000000000,0.0000000000,0.0000514302,0.0000239961,
		0.0000047765,0.0000000002,0.0002599014,0.0002016429,0.0001417040,0.0001034708,0.0003478659,0.0003149412,0.0003157957,0.0002825135,
		0.0000000452,0.0000000015,0.0000000003,0.0000000000,0.0000000043,0.0000000012,0.0000000000,0.0000000000,0.0000280866,0.0000233669,
		0.0000074276,0.0000000000,0.0002076980,0.0001793560,0.0001377298,0.0000676134,0.0001902075,0.0001876180,0.0001946719,0.0001773396
		},{
		0.0000000193,0.0000000009,0.0000000001,0.0000000000,0.0000000024,0.0000000008,0.0000000000,0.0000000000,0.0001612655,0.0000767754,
		0.0000030992,0.0000000001,0.0007902611,0.0003761498,0.0001701120,0.0000493729,0.0011983353,0.0007016607,0.0003143604,0.0001771009,
		0.0000000364,0.0000000012,0.0000000002,0.0000000000,0.0000000033,0.0000000009,0.0000000000,0.0000000000,0.0001113982,0.0000764033,
		0.0000059924,0.0000000000,0.0006093337,0.0003319527,0.0001464207,0.0000293241,0.0005876531,0.0003952602,0.0002427287,0.0001212675
		},{
		0.0000000279,0.0000000013,0.0000000002,0.0000000000,0.0000000033,0.0000000012,0.0000000000,0.0000000000,0.0001382877,0.0000766047,
		0.0000044924,0.0000000002,0.0006212443,0.0003717088,0.0001932077,0.0000552541,0.0009574685,0.0007094832,0.0003788397,0.0002630184,
		0.0000000478,0.0000000016,0.0000000003,0.0000000000,0.0000000044,0.0000000013,0.0000000000,0.0000000000,0.0000901087,0.0000760437,
		0.0000078665,0.0000000000,0.0005160693,0.0002925099,0.0001693718,0.0000360309,0.0004892221,0.0004083544,0.0002708363,0.0001694871
		},{
		0.0000000477,0.0000000028,0.0000000003,0.0000000000,0.0000000085,0.0000000031,0.0000000000,0.0000000000,0.0001318078,0.0001052571,
		0.0000073214,0.0000000003,0.0006430936,0.0004386124,0.0002651919,0.0001276619,0.0008956386,0.0008226470,0.0006108689,0.0004287192,
		0.0000001832,0.0000000056,0.0000000011,0.0000000000,0.0000000153,0.0000000039,0.0000000000,0.0000000000,0.0001013199,0.0000866109,
		0.0000304139,0.0000000000,0.0005568590,0.0003949820,0.0002403313,0.0000917924,0.0005402578,0.0004998288,0.0004282389,0.0002761413
		},{
		0.0000002844,0.0000000117,0.0000000017,0.0000000000,0.0000000294,0.0000000100,0.0000000000,0.0000000000,0.0001166545,0.0001116556,
		0.0000463854,0.0000000015,0.0005712178,0.0004685537,0.0003843669,0.0002365646,0.0007037026,0.0008110816,0.0007595831,0.0006621719,
		0.0000003250,0.0000000112,0.0000000019,0.0000000000,0.0000000328,0.0000000102,0.0000000000,0.0000000000,0.0001057150,0.0001104076,
		0.0000530275,0.0000000001,0.0004889682,0.0004442254,0.0003229013,0.0002022755,0.0005303296,0.0005445463,0.0005193195,0.0003962233
		},{
		0.0000823041,0.0000274606,0.0000008828,0.0000000000,0.0001163417,0.0000458702,0.0000000046,0.0000000000,0.0000918572,0.0000468299,
		0.0000010483,0.0000005746,0.0000212553,0.0000152099,0.0000107647,0.0000058480,0.0000200254,0.0000201708,0.0000161896,0.0000138231,
		0.0005193816,0.0001133483,0.0000228734,0.0000000000,0.0003002979,0.0000669598,0.0000002388,0.0000000000,0.0001118043,0.0000469821,
		0.0000018419,0.0000007075,0.0000238049,0.0000175070,0.0000120635,0.0000303191,0.0000313061,0.0000700936,0.0000452497,0.0000619370
		},{
		0.0001393132,0.0000712085,0.0000018920,0.0000000000,0.0001875128,0.0000705679,0.0000000098,0.0000000000,0.0001233266,0.0000725648,
		0.0000022831,0.0000011032,0.0000654572,0.0000516221,0.0000412165,0.0000311212,0.0000473590,0.0000459968,0.0000372700,0.0000496619,
		0.0006141384,0.0002543620,0.0000621832,0.0000000000,0.0003251981,0.0001109580,0.0000006493,0.0000000000,0.0001259168,0.0000728432,
		0.0000216489,0.0000193399,0.0000878173,0.0000558096,0.0000611402,0.0000948409,0.0000860558,0.0001032295,0.0000902085,0.0001034445
		},{
		0.0001989258,0.0001164260,0.0000029388,0.0000000000,0.0002611137,0.0001315173,0.0000000153,0.0000000000,0.0001323288,0.0000768088,
		0.0000203338,0.0000016530,0.0000612187,0.0000319356,0.0000405908,0.0000329799,0.0000436160,0.0000428678,0.0000528783,0.0000316020,
		0.0006588294,0.0004167500,0.0001200217,0.0000000000,0.0004042812,0.0001738249,0.0000368297,0.0000000000,0.0001522075,0.0000771803,
		0.0000049236,0.0000204836,0.0000654318,0.0000525745,0.0000250208,0.0000632640,0.0000747593,0.0000613686,0.0000875661,0.0001041343
		},{
		0.0000795959,0.0000715660,0.0000198432,0.0000000000,0.0001473495,0.0000869948,0.0000001031,0.0000000000,0.0000869620,0.0000552850,
		0.0000207643,0.0000008881,0.0000819023,0.0000806184,0.0000666898,0.0000517113,0.0000640243,0.0000608878,0.0000499150,0.0000593688,
		0.0003966162,0.0002539178,0.0000977328,0.0000000000,0.0002244114,0.0000915341,0.0000365970,0.0000000000,0.0000891670,0.0000735617,
		0.0000402813,0.0000372181,0.0001060141,0.0000861982,0.0000873614,0.0001165910,0.0001273746,0.0001218672,0.0001247028,0.0001513897
		},{
		0.0000017098,0.0000005705,0.0000000183,0.0000000000,0.0000024169,0.0000009529,0.0000000001,0.0000000000,0.0000019082,0.0000009728,
		0.0000000218,0.0000000119,0.0000004416,0.0000003160,0.0000002236,0.0000001215,0.0000004160,0.0000004190,0.0000003363,0.0000002872,
		0.0000815732,0.0000023547,0.0000004752,0.0000000000,0.0000062384,0.0000013910,0.0000000050,0.0000000000,0.0000023226,0.0000009760,
		0.0000000383,0.0000000147,0.0000004945,0.0000003637,0.0000002506,0.0000006298,0.0000006504,0.0000014561,0.0000009400,0.0000012867
		},{
		0.0000047640,0.0000019742,0.0000000573,0.0000000000,0.0000065849,0.0000025438,0.0000000003,0.0000000000,0.0000066298,0.0000040493,
		0.0000004018,0.0000000353,0.0000113388,0.0000080407,0.0000056680,0.0000024116,0.0000105276,0.0000102521,0.0000082145,0.0000067155,
		0.0001332029,0.0000606214,0.0000016891,0.0000000000,0.0000676114,0.0000038371,0.0000000176,0.0000000000,0.0000075771,0.0000041719,
		0.0000009160,0.0000003236,0.0000129890,0.0000093080,0.0000065000,0.0000232395,0.0000173715,0.0000352429,0.0000318148,0.0000301335
		},{
		0.0000622469,0.0000452942,0.0000010621,0.0000000000,0.0001126871,0.0000445237,0.0000000055,0.0000000000,0.0000552882,0.0000115498,
		0.0000014756,0.0000005640,0.0000342873,0.0000416131,0.0000340373,0.0000081306,0.0000328522,0.0000313880,0.0000247797,0.0000206067,
		0.0002824818,0.0001643094,0.0000225184,0.0000000000,0.0001460227,0.0000649179,0.0000180234,0.0000000000,0.0000745492,0.0000295992,
		0.0000027347,0.0000009703,0.0000566729,0.0000451505,0.0000360576,0.0000510859,0.0000693523,0.0000845149,0.0000748150,0.0000875487
		},{
		0.0000640152,0.0000464998,0.0000010910,0.0000000000,0.0000977060,0.0000458933,0.0000000057,0.0000000000,0.0000583885,0.0000311544,
		0.0000017753,0.0000005796,0.0000608295,0.0000470188,0.0000196518,0.0000275286,0.0000432208,0.0000400099,0.0000490203,0.0000252920,
		0.0002900761,0.0001687563,0.0000591645,0.0000000000,0.0001679756,0.0000667337,0.0000184060,0.0000000000,0.0000776829,0.0000491126,
		0.0000207219,0.0000188796,0.0000656249,0.0000333694,0.0000572648,0.0000529337,0.0000986472,0.0000930882,0.0000811116,0.0000927938
		},{
		0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,
		0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,
		0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,
		0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000
		},{
		0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,
		0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,
		0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,
		0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000
		},{
		0.0000004291,0.0000001427,0.0000000046,0.0000000000,0.0000006044,0.0000002383,0.0000000000,0.0000000000,0.0000015684,0.0000011210,
		0.0000002750,0.0000000030,0.0000057235,0.0000038938,0.0000027630,0.0000010141,0.0000053539,0.0000052795,0.0000041463,0.0000031956,
		0.0000203953,0.0000005887,0.0000001188,0.0000000000,0.0000015598,0.0000003478,0.0000000012,0.0000000000,0.0000016722,0.0000011342,
		0.0000003337,0.0000000037,0.0000063241,0.0000045869,0.0000031108,0.0000019899,0.0000087411,0.0000258153,0.0000066782,0.0000231499
		},{
		0.0000015840,0.0000006553,0.0000000190,0.0000000000,0.0000021829,0.0000008432,0.0000000001,0.0000000000,0.0000070187,0.0000049550,
		0.0000009322,0.0000000117,0.0000272961,0.0000181992,0.0000119945,0.0000046351,0.0000267267,0.0000250306,0.0000195733,0.0000327400,
		0.0000439842,0.0000201945,0.0000005606,0.0000000000,0.0000048089,0.0000012720,0.0000000059,0.0000000000,0.0000077511,0.0000051789,
		0.0000014614,0.0000001078,0.0000312510,0.0000212425,0.0000308619,0.0000076603,0.0000789921,0.0000718364,0.0000626618,0.0000563999
		},{
		0.0000000023,0.0000000001,0.0000000000,0.0000000000,0.0000000003,0.0000000001,0.0000000000,0.0000000000,0.0000110470,0.0000067235,
		0.0000003695,0.0000000000,0.0000884955,0.0000463602,0.0000144519,0.0000030776,0.0000734857,0.0000586722,0.0000241308,0.0000153202,
		0.0000000046,0.0000000001,0.0000000000,0.0000000000,0.0000000004,0.0000000001,0.0000000000,0.0000000000,0.0000119828,0.0000068604,
		0.0000007570,0.0000000000,0.0001344130,0.0000706363,0.0000159655,0.0000057444,0.0002534417,0.0001317902,0.0000504787,0.0000202502
		},{
		0.0000000034,0.0000000002,0.0000000000,0.0000000000,0.0000000004,0.0000000002,0.0000000000,0.0000000000,0.0000078008,0.0000053042,
		0.0000005508,0.0000000000,0.0000561568,0.0000406875,0.0000129865,0.0000038193,0.0000406025,0.0000511033,0.0000399041,0.0000153564,
		0.0000000066,0.0000000002,0.0000000000,0.0000000000,0.0000000006,0.0000000002,0.0000000000,0.0000000000,0.0000083446,0.0000054342,
		0.0000010953,0.0000000000,0.0000975502,0.0000446625,0.0000140835,0.0000061746,0.0001341038,0.0000993414,0.0000649346,0.0000376723
		},{
		0.0000000114,0.0000000005,0.0000000001,0.0000000000,0.0000000013,0.0000000005,0.0000000000,0.0000000000,0.0000179470,0.0000122319,
		0.0000018394,0.0000000001,0.0001422390,0.0000899949,0.0000319160,0.0000108205,0.0001343744,0.0001167053,0.0000907363,0.0000569251,
		0.0000000185,0.0000000006,0.0000000001,0.0000000000,0.0000000017,0.0000000005,0.0000000000,0.0000000000,0.0000180378,0.0000124908,
		0.0000030408,0.0000000000,0.0001500428,0.0001142097,0.0000694873,0.0000159066,0.0002549624,0.0001892990,0.0001405834,0.0001201633
		},{
		0.0000000272,0.0000000012,0.0000000002,0.0000000000,0.0000000031,0.0000000011,0.0000000000,0.0000000000,0.0000319388,0.0000230167,
		0.0000043926,0.0000000001,0.0002342709,0.0001776474,0.0001374855,0.0000613119,0.0002166819,0.0002116546,0.0001877537,0.0001584907,
		0.0000000422,0.0000000014,0.0000000002,0.0000000000,0.0000000040,0.0000000012,0.0000000000,0.0000000000,0.0000498554,0.0000236109,
		0.0000069370,0.0000000000,0.0002620825,0.0002054063,0.0001405215,0.0001067203,0.0003707786,0.0003307903,0.0003021593,0.0002644011
		},{
		0.0000000229,0.0000000010,0.0000000001,0.0000000000,0.0000000028,0.0000000010,0.0000000000,0.0000000000,0.0001371370,0.0000769235,
		0.0000036818,0.0000000001,0.0005934319,0.0003083149,0.0001502882,0.0000269198,0.0005634495,0.0004006507,0.0002404522,0.0001483780,
		0.0000000405,0.0000000013,0.0000000002,0.0000000000,0.0000000037,0.0000000011,0.0000000000,0.0000000000,0.0001560571,0.0000774075,
		0.0000066633,0.0000000000,0.0007902924,0.0004072055,0.0001742437,0.0000583099,0.0012044879,0.0007068039,0.0003336374,0.0002060795
		},{
		0.0000000292,0.0000000013,0.0000000002,0.0000000000,0.0000000035,0.0000000012,0.0000000000,0.0000000000,0.0000980195,0.0000767029,
		0.0000047019,0.0000000002,0.0005249671,0.0002906173,0.0001738554,0.0000317968,0.0004923339,0.0004181506,0.0002682792,0.0001776526,
		0.0000000508,0.0000000017,0.0000000003,0.0000000000,0.0000000047,0.0000000013,0.0000000000,0.0000000000,0.0001169924,0.0000775754,
		0.0000083610,0.0000000000,0.0006078322,0.0003857026,0.0001976948,0.0000825157,0.0009683879,0.0007180510,0.0004362627,0.0002750194
		},{
		0.0000000509,0.0000000029,0.0000000003,0.0000000000,0.0000000089,0.0000000032,0.0000000000,0.0000000000,0.0001076153,0.0000860450,
		0.0000078596,0.0000000003,0.0005429734,0.0003889726,0.0002437068,0.0000848001,0.0005360390,0.0005057014,0.0004017883,0.0003002863,
		0.0000001863,0.0000000057,0.0000000011,0.0000000000,0.0000000156,0.0000000040,0.0000000000,0.0000000000,0.0001433664,0.0001049322,
		0.0000309184,0.0000000000,0.0006372650,0.0004475756,0.0002675585,0.0001395194,0.0008743298,0.0008199472,0.0006308185,0.0004563910
		},{
		0.0000002894,0.0000000119,0.0000000017,0.0000000000,0.0000000299,0.0000000101,0.0000000000,0.0000000000,0.0001121248,0.0000923140,
		0.0000472022,0.0000000016,0.0005558248,0.0004439129,0.0003473964,0.0001576250,0.0005185278,0.0005782374,0.0005139007,0.0004238402,
		0.0000003313,0.0000000114,0.0000000020,0.0000000000,0.0000000334,0.0000000104,0.0000000000,0.0000000000,0.0000931787,0.0000938227,
		0.0000540488,0.0000000001,0.0005860547,0.0004799935,0.0003892232,0.0002703133,0.0006833981,0.0007752376,0.0008043727,0.0007131948
		}
	};


        for (k = 0; k < (int)isFree[0][0].size(); k++) {
            for (i = (int)isFree.size() - 1; i >= 0; i--) {
                for (j = (int)isFree[0].size() - 1; j >= 0; j--) {
                        if (isFree[i][j][k]) {
                            allocation_index.push_back(0);
                        } else {
                            allocation_index.push_back(1);
                        }
                    }
                }
            }


            for (j = 0; j < (int)allocation_index.size(); j++)
            {
                for (i = 0; i < (int)allocation_index.size(); i++)
                {
                    sum_inlet += DMatrix[j][i] * (Pidle + Putil * allocation_index[i]);
                    //std::cout<<"DMatrix["<<j<<"]["<<i<<"]="<<DMatrix[j][i]<<"; sum_inlet("<<i<<")="<<sum_inlet<<std::endl;
                }
                //std::cout<<"DP("<<j<<")="<<sum_inlet<<std::endl;
                inlet_temperature.push_back(Tsup+sum_inlet);
                sum_inlet = 0;
            }

            for (i = 0; i < (int)allocation_index.size(); i++)
            {
                //std::cout<<"index="<<i<<"; allocation:"<<allocation_index[i]<<"; inlet_temperature is="<<inlet_temperature[i]<<"C."<<std::endl;
                if (max_inlet < inlet_temperature[i]) {
                    max_inlet = inlet_temperature[i];
                }
                if (allocation_index[i]) {
                    busynodes++;	
                }
            }

            // Total power of data center
            Pcompute = busynodes * Putil + allocation_index.size() * Pidle;

            // Supply temperature
            Tsup = Tsup + Tred-max_inlet;

            // Coefficient of performance
            COP = 0.0068 * Tsup * Tsup + 0.0008 * Tsup + 0.458;

            // Cooling power in kW
            Scaling_Factor = 214.649 / 120;
            Pcooling = 0.001 * Pcompute * (1 / COP) / Scaling_Factor;

            //std::cout<<"Maximum Temperature is: "<<max_inlet<<"C."<<std::endl;
            return  Pcooling;

        }

        //Jie: end editing Oct2013	






        /*
           std::string MachineMesh::tostd::string(){
        //returns human readable view of which processors are free
        //presented in layers by z-coordinate (0 first), with the
        //  (0,0) position of each layer in the bottom left
        //uses "X" and "." to denote free and busy processors respectively

        std::string retVal = "";
        for(int k=0; k<isFree[0][0].size(); k++) {
        for(int j=isFree[0].length-1; j>=0; j--) {
        for(int i=0; i<isFree.length; i++)
        if(isFree[i][j][k]) retVal += "X";
        else
        retVal += ".";
        retVal += "\n";
        }

        if(k != isFree[0][0].length-1)  //add blank line between layers
        retVal += "\n";
        }

        return retVal;
        }
        */

