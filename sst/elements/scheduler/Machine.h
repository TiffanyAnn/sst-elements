// Copyright 2009-2014 Sandia Corporation. Under the terms
// of Contract DE-AC04-94AL85000 with Sandia Corporation, the U.S.
// Government retains certain rights in this software.
// 
// Copyright (c) 2009-2014, Sandia Corporation
// All rights reserved.
// 
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

/*
 * Abstract base class for machines
 */

#ifndef SST_SCHEDULER_MACHINE_H__
#define SST_SCHEDULER_MACHINE_H__

#include <string>
#include <vector>

namespace SST {
    namespace Scheduler {
        class AllocInfo;

        class Machine{
            public:

                Machine(int numNodes, int numCoresPerNode, double** D_matrix);
                ~Machine();

                int getNumFreeNodes() const { return numAvail; }
                int getNumNodes() const { return numNodes; }
                int getNumCoresPerNode() const { return coresPerNode; }
                
                std::vector<int>* getFreeNodes() const;
                
                virtual std::string getSetupInfo(bool comment) = 0;
                virtual void reset() = 0;
                virtual void allocate(AllocInfo* allocInfo) = 0;
                virtual void deallocate(AllocInfo* allocInfo) = 0;
                virtual long getNodeDistance(int node1, int node2) const = 0;
                
                double** D_matrix;

            protected:
                int numNodes;          //total number of nodes
                int numAvail;          //number of available nodes
                int coresPerNode;
                
                //TODO: put private:
                std::vector<bool> isFree;  //whether each node is free
                
        };

    }
}
#endif
