// -*- mode: c++ -*-

// Copyright 2009-2018 NTESS. Under the terms
// of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2018, NTESS
// All rights reserved.
//
// Portions are copyright of other developers:
// See the file CONTRIBUTORS.TXT in the top level directory
// the distribution for more information.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.


#ifndef COMPONENTS_MERLIN_MERLIN_H
#define COMPONENTS_MERLIN_MERLIN_H

#include <cctype>
#include <string>
#include <sst/core/simulation.h>
#include <sst/core/timeConverter.h>
#include <sst/core/output.h>
#include <unordered_map>
#include <unordered_set>

extern int downLinkCount; //how many times a downlink was encountered
extern int minBlockedCount;
extern int valBlockedCount;
extern int dirPacketCount;
extern int valPacketCount;
extern int allPackets;
extern int valiantRoute;
extern int directRoute;
extern int totalPackets;

extern int RUNTYPE; //0: create routing table
				        //1: read routing table from file
                    //2: normal run

extern int ROUTE;  /* 0 is 1st direct route,
				          1 is 2nd direct route,
				          2 is 1st valiant route,
				          3 is 2nd valiant route,
				          set to any other number to disable */
extern std::string RT_FILENAME;
extern std::string DOWNPORT_FNAME;

extern std::unordered_multimap<uint64_t,uint64_t>umap; //the routing table
extern std::unordered_set<uint64_t>downRoutes; //routes unavailable due to link failures
extern std::unordered_set<uint64_t>downPorts; //ports marked as disabled

using namespace SST;

namespace SST {
namespace Merlin {

    // Library wide Output object.  Mostly used for fatal() calls
    static Output merlin_abort("Merlin: ", 5, -1, Output::STDERR);
    static Output merlin_abort_full("Merlin: @f, line @l: ", 5, -1, Output::STDOUT);

}
}

#endif // COMPONENTS_MERLIN_MERLIN_H
