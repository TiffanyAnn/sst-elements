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
//

#include <sst_config.h>
#include <sst/core/sharedRegion.h>
#include "sst/core/rng/xorshift.h"
#include "sst/elements/merlin/merlin.h"

#include "dragonfly2.h"

#include <stdlib.h>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

#define ROUTE 4 /* 0 is 1st direct route,
				   1 is 2nd direct route,
				   2 is 1st valiant route,
				   3 is 2nd valiant route,
				   set to any other number to disable */

#define RUNTYPE 2 /* 0 - generates routing table
					 1 - loads the routing table from file and
				         does adaptive routing for failed links */
				  // 2 - normal run

using namespace SST::Merlin;
int enter = 0;
static int downLinkEncountered = 0;
/*int downLinkCount;
int minBlockedCount;
int valBlockedCount;
int dirPacketCount;
int valPacketCount;
int allPackets; */

static int minBlocked =0;
static int valBlocked = 0;
static int minPackets = 0;
static int valPackets = 0;
static int packets = 0;
bool phase0 = true;
int getKey(unsigned int val){
    return (val ^ 5300);
}

// grp | rtr | port
static unsigned int downLink0 = (1111 << 16) | (1110 << 8) | 17;
static unsigned int downLink1 = (1111 << 16) | (1119 << 8) | 6;
static unsigned int downLink2 = (1111 << 16) | (1110 << 8) | 3;
static unsigned int downLink3 = (1112 << 16) | (1110 << 8) | 4;
std::unordered_set<unsigned int>DL1= { downLink0, downLink1,
								 	  downLink2, downLink3 };
/*
std::unordered_multimap<unsigned int,unsigned int>umap;
std::unordered_set<unsigned int>downRoutes;
*/

// Check routing decisions against the downRoute set
// increment downLink encountered counter each time a route is avoided

bool isRouteDown2(unsigned int route){
	std::unordered_set<unsigned int>::const_iterator i = downRoutes.find (route);
	if( i == downRoutes.end()) //route exists in the set
		return false;
	else {
		return true;
	}
}

bool isPortDown(uint32_t src_group_id, uint32_t group_id, uint32_t router_id, int port);

void
RouteToGroup2::init(SharedRegion* sr, size_t g, size_t r)
{
    region = sr;
    data = sr->getPtr<const RouterPortPair2*>();
    groups = g;
    routes = r;

}

const RouterPortPair2&
RouteToGroup2::getRouterPortPair(int group, int route_number)
{
    return data[group*routes + route_number];
}

void
RouteToGroup2::setRouterPortPair(int group, int route_number, const RouterPortPair2& pair) {
    region->modifyArray(group*routes+route_number,pair);
}


/*
 * Port Layout:
 * [0, params.p)                    // Hosts 0 -> params.p
 * [params.p, params.p+params.a-1)  // Routers within this group
 * [params.p+params.a-1, params.k)  // Other groups
 */
topo_dragonfly2::topo_dragonfly2(Component* comp, Params &p) :
    Topology(comp)
{
    params.p = (uint32_t)p.find<int>("dragonfly:hosts_per_router");
    params.a = (uint32_t)p.find<int>("dragonfly:routers_per_group");
    params.k = (uint32_t)p.find<int>("num_ports");
    params.h = (uint32_t)p.find<int>("dragonfly:intergroup_per_router");
    params.g = (uint32_t)p.find<int>("dragonfly:num_groups");
    params.n = (uint32_t)p.find<int>("dragonfly:intergroup_links");

    std::string global_route_mode_s = p.find<std::string>("dragonfly:global_route_mode","absolute");
    if ( global_route_mode_s == "absolute" ) global_route_mode = ABSOLUTE;
    else if ( global_route_mode_s == "relative" ) global_route_mode = RELATIVE;
    else {
        output.fatal(CALL_INFO, -1, "Invalid dragonfly:global_route_mode specified: %s.\n",global_route_mode_s.c_str());
    }

    std::string route_algo = p.find<std::string>("dragonfly:algorithm", "minimal");

    adaptive_threshold = p.find<double>("dragonfly:adaptive_threshold",2.0);

    // Get the global link map
    std::vector<int64_t> global_link_map;

    // For now, parse array ourselves so as not to create a dependency
    // on new core features.  Once we want to use new core features,
    // delete code below and uncomment line above (can also get rid of
    // #include <sstream> above).
    std::string array = p.find<std::string>("dragonfly:global_link_map");
    if ( array != "" ) {
        array = array.substr(1,array.size()-2);

        std::stringstream ss(array);

        while( ss.good() ) {
            std::string substr;
            getline( ss, substr, ',' );
            global_link_map.push_back(strtol(substr.c_str(), NULL, 0));
        }
    }
    // End parse array on our own

    // Get a shared region
    SharedRegion* sr = Simulation::getSharedRegionManager()->getGlobalSharedRegion("dragonfly:group_to_global_port",
                                                                                  ((params.g-1) * params.n) * sizeof(RouterPortPair2),
                                                                                   new SharedRegionMerger());
    // Set up the RouteToGroup object
    group_to_global_port.init(sr, params.g, params.n);

    // Fill in the shared region using the RouteToGroupObject (if
    // vector for param dragonfly:global_link_map is empty, then
    // nothing will be intialized.
    for ( int i = 0; i < global_link_map.size(); i++ ) {
        // Figure out all the mappings
        int64_t value = global_link_map[i];
        if ( value == -1 ) continue;

        int group = value % (params.g - 1);
        int route_num = value / (params.g - 1);
        int router = i / params.h;
        int port = (i % params.h) + params.p + params.a - 1;

        RouterPortPair2 rpp;
        rpp.router = router;
        rpp.port = port;
        group_to_global_port.setRouterPortPair(group, route_num, rpp);
    }


    // Publish the shared region to make sure everyone has the data.
    sr->publish();

    if ( !route_algo.compare("valiant") ) {
        if ( params.g <= 2 ) {
            /* 2 or less groups... no point in valiant */
            algorithm = MINIMAL;
        } else {
            algorithm = VALIANT;
        }
    }
    else if ( !route_algo.compare("adaptive-local") ) {
        algorithm = ADAPTIVE_LOCAL;
    }
    else {
        algorithm = MINIMAL;
    }

    uint32_t id = p.find<int>("id");
    group_id = id / params.a;
    router_id = id % params.a;

    rng = new RNG::XORShiftRNG(id+1);


    output.verbose(CALL_INFO, 1, 1, "%u:%u:  ID: %u   Params:  p = %u  a = %u  k = %u  h = %u  g = %u\n",
            group_id, router_id, id, params.p, params.a, params.k, params.h, params.g);
}


topo_dragonfly2::~topo_dragonfly2()
{
}


void topo_dragonfly2::route(int port, int vc, internal_router_event* ev)
{

    topo_dragonfly2_event *td_ev = static_cast<topo_dragonfly2_event*>(ev);

    // Break this up by port type
	if(ev->getTrack() == true ){
		std::cout << "\n========== route() ==========\n";
		printf("src_grp: %u curr_grp: %u curr_rtr: %u curr_port: %u\n", td_ev->src_group, group_id, router_id, port);
	}

    uint32_t next_port = 0;
    if ( (uint32_t)port < params.p ) {
        // Host ports
        if ( td_ev->dest.group == td_ev->src_group ) {
            // Packets stays within the group
        	if ( td_ev->dest.router == router_id ) {
             	// Stays within the router
            	next_port = td_ev->dest.host;
        	}
            else {
                // Route to the router specified by mid_group.  If
                // this is a direct route then mid_group will equal
                // router and packet will go direct.
                next_port = port_for_router(td_ev->dest.mid_group);
				}

            }
        else {
            // Packet is leaving group.  Simply route to group
            // specified by mid_group.  If this is a direct route then
            // mid_group will be set to group.
            next_port = port_for_group(td_ev->dest.mid_group, td_ev->global_slice);
        }
    }
    else if ( (uint32_t)port < ( params.p + params.a - 1) ) {
        // Intragroup links
        if ( td_ev->dest.group == group_id ) {
            // In final group
            if ( td_ev->dest.router == router_id ) {
                // In final router, route to host port
                next_port = td_ev->dest.host;
			}
            else {
                // This is a valiantly routed packet within a group.
                // Need to increment the VC and route to correct
                // router.
                td_ev->setVC(vc+1);
                next_port = port_for_router(td_ev->dest.router);
          }
        }
        else {
            // Not in correct group, should route out one of the
            // global links
            if ( (td_ev->dest.mid_group != group_id) && (td_ev->dest.mid_group != td_ev->dest.group) ) {
                next_port = port_for_group(td_ev->dest.mid_group, td_ev->global_slice);
            }
			else {
                next_port = port_for_group(td_ev->dest.group, td_ev->global_slice);
            }
		}
	}
    else { // global
        /* Came in from another group.  Increment VC */
        td_ev->setVC(vc+1);
        if ( td_ev->dest.group == group_id ) {
            if ( td_ev->dest.router == router_id ) {
                // In final router, route to host port
                next_port = td_ev->dest.host;
            }
            else {
                // Go to final router
                next_port = port_for_router(td_ev->dest.router);
            }
        }
        else {
            // Just passing through on a valiant route.  Route
            // directly to final group
            next_port = port_for_group(td_ev->dest.group, td_ev->global_slice);
        }
    }

    if(ev->getTrack() == true){
    	printf("ev->dest: %u dest_grp: %u dest_rtr: %u next_port: %u\n",
                ev->getDest(), td_ev->dest.group, td_ev->dest.router, next_port);
		unsigned int src_to_dest0 = (0 << 16) | (ev->getSrc() << 8) | ev->getDest();
		unsigned int src_to_dest1 = (1 << 16) | (ev->getSrc() << 8) | ev->getDest();
		unsigned int src_to_dest2 = (2 << 16) | (ev->getSrc() << 8) | ev->getDest();
		unsigned int src_to_dest3 = (3 << 16) | (ev->getSrc() << 8) | ev->getDest();
		printf("r0: %d r1: %d r2: %d r3: %d\n",  src_to_dest0,  src_to_dest1, src_to_dest2, src_to_dest3);
	}

    output.verbose(CALL_INFO, 1, 1, "%u:%u, Recv: %d/%d  Setting Next Port/VC:  %u/%u\n", group_id, router_id, port, vc, next_port, td_ev->getVC());
    td_ev->setNextPort(next_port);

#if RUNTYPE == 0
		unsigned int src_to_dest = (ROUTE << 16) | (ev->getSrc() << 8) | ev->getDest();
		unsigned int link = (group_id << 16) | (router_id << 8) | port;
		umap.insert(std::make_pair(src_to_dest,link));
#endif
}

void topo_dragonfly2::reroute(int port, int vc, internal_router_event* ev)
{

packets++;
allPackets = packets;

#if RUNTYPE == 1
	bool r0 = false; bool r1 = false;
	bool r2 = false; bool r3 = false;
	if(isRouteDown2((0 << 16) | (ev->getSrc() << 8) | ev->getDest()) == true)
		r0 = true;
	if(isRouteDown2((1 << 16) | (ev->getSrc() << 8) | ev->getDest()) == true)
		r1 = true;
    if(isRouteDown2((2 << 16) | (ev->getSrc() << 8) | ev->getDest()) == true)
        r2 = true;
    if(isRouteDown2((3 << 16) | (ev->getSrc() << 8) | ev->getDest()) == true)
        r3 = true;

	// if all routes are marked as unavailable then routing will fail
	if((r0 == true) && (r1 == true) && (r2==true) && (r3==true)){
		printf("ERROR_1: No available routes from %d -> %d\n", ev->getSrc(), ev->getDest());
        exit( EXIT_FAILURE);
	}
#endif

    if ( algorithm != ADAPTIVE_LOCAL ) return;

    topo_dragonfly2_event *td_ev = static_cast<topo_dragonfly2_event*>(ev);

    if(ev->getTrack() == true){
		enter++;
        std::cout << "========== reroute() ==========\n";
		std::cout << enter << "\n";
	    printf("src_grp: %u curr_grp: %u curr_rtr: %u curr_port: %u\n", td_ev->src_group, group_id, router_id, port);
	}

    // For now, we make the adaptive routing decision only at the
    // input to the network and at the input to a group for adaptively
    // routed packets
    if ( port >= params.p && port < (params.p + params.a-1)){
		minPackets++;
	//	dirPacketCount = minPackets;
		td_ev->setRouting(0); //set as direct route taken
		return;
	}

    // Adaptive routing when packet stays in group
    if ( (port < params.p) && td_ev->dest.group == group_id ) {
        // If we're at the correct router, no adaptive needed
        if ( td_ev->dest.router == router_id){
			minPackets++;
		//	dirPacketCount = minPackets;
			td_ev->setRouting(0); //set as direct route taken
			return;
		}

        int direct_route_port = port_for_router(td_ev->dest.router);
        int direct_route_credits = output_credits[direct_route_port * num_vcs + vc];

        int valiant_route_port = port_for_router(td_ev->dest.mid_group);
        int valiant_route_credits = output_credits[valiant_route_port * num_vcs + vc];

		int drc = 0;
		int vrc = 0;
#if RUNTYPE == 1
    //if(phase0 == true){
		if(r0 == true || r1 == true) { direct_route_credits = -1; drc = -1;}
		if(r2 == true || r3 == true) { valiant_route_credits = -1; vrc = -1;}
//	}
    	//if( isPortDown(group_id, td_ev->dest.group, td_ev->dest.router, direct_route_port) == true ) direct_route_credits = -1;
    	//if( isPortDown(group_id, td_ev->dest.mid_group, td_ev->dest.router, valiant_route_port) == true ) valiant_route_credits = -1;
#endif
        if (((ROUTE!=0) || (ROUTE!=1)) || (valiant_route_credits > (int)((double)direct_route_credits * adaptive_threshold)) ) {
            td_ev->setNextPort(valiant_route_port);
			valPackets++;
			td_ev->setRouting(1); //set as direct route not taken
			if(drc == -1) { downLinkEncountered++; minBlocked++; }
			if(ev->getTrack() == true){
				std::cout << "taking valiant route\n";
				printf("dest_grp: %u dest_rtr: %u next_port: %u\n",
				td_ev->dest.mid_group, td_ev->dest.router, valiant_route_port);
			}
        }
        else {
            td_ev->setNextPort(direct_route_port);
			minPackets++;
			td_ev->setRouting(0); //set as direct route taken
			if(vrc == -1){ downLinkEncountered++; valBlocked++; }
			if(ev->getTrack() == true){
                std::cout << "taking direct route\n";
                printf("dest_grp: %u dest_rtr: %u next_port: %u\n",
				td_ev->dest.group, td_ev->dest.router, direct_route_port);
            }
        }
				downLinkCount = downLinkEncountered;
				minBlockedCount = minBlocked;
				valBlockedCount = valBlocked;
			//	dirPacketCount = minPackets;
			//	valPacketCount = valPackets;
        return;
    }

    // If the dest is in the same group, no need to adaptively route
    if ( td_ev->dest.group == group_id ){
		 minPackets++;
		 //dirPacketCount = minPackets;
		 td_ev->setRouting(0); //set as direct route taken
		 return;
	}


    // Based on the output queue depths, choose minimal or valiant
    // route.  We'll chose the direct route unless the remaining
    // output credits for the direct route is half of the valiant
    // route.  We'll look at two slice for each direct and indirect,
    // giving us a total of 4 routes we are looking at.  For packets
    // which came in adaptively on global links, look at two direct
    // routes and chose between the.
    int direct_slice1 = td_ev->global_slice_shadow;
   // int direct_slice2 = td_ev->global_slice;
    int direct_slice2 = (td_ev->global_slice_shadow + 1) % params.n;
    int direct_route_port1 = port_for_group(td_ev->dest.group, direct_slice1, 0 );
    int direct_route_port2 = port_for_group(td_ev->dest.group, direct_slice2, 1 );
    int direct_route_credits1 = output_credits[direct_route_port1 * num_vcs + vc];
    int direct_route_credits2 = output_credits[direct_route_port2 * num_vcs + vc];
    int direct_slice;
    int direct_route_port;
    int direct_route_credits;

	int temp_drc1 = direct_route_credits1;
	int temp_drc2 = direct_route_credits2;
	int temp_drc;
	int temp_vrc1;
	int temp_vrc2;
	int temp_vrc = 0;
	int temp_downlink = 0;

#if RUNTYPE == 0
	if(ROUTE == 0) direct_route_credits1 = -1;
	if(ROUTE == 1) direct_route_credits2 = -1;
#endif

#if RUNTYPE == 1
  //  if(phase0 == true){
//	int direct_rtr = router_to_group(td_ev->dest.group);
//    if( isPortDown(ev,group_id, td_ev->dest.group, direct_rtr, direct_route_port1) == true )
		if(r0 == true) direct_route_credits1 = -1;
//    if( isPortDown(ev,group_id, td_ev->dest.group, direct_rtr, direct_route_port2) == true )
		if(r1 == true) direct_route_credits2 = -1;
//	}
#endif
    if ( direct_route_credits1 > direct_route_credits2 ) {
	//	if(temp_drc1 < temp_drc2) temp_downlink=1;
        direct_slice = direct_slice1;
        direct_route_port = direct_route_port1;
        direct_route_credits = direct_route_credits1;
		temp_drc = temp_drc1;
    }
    else {
	//	if(temp_drc1 > temp_drc2) temp_downlink=1;
        direct_slice = direct_slice2;
        direct_route_port = direct_route_port2;
        direct_route_credits = direct_route_credits2;
		temp_drc = temp_drc2;
    }

    int valiant_slice = 0;
    int valiant_route_port = 0;
    int valiant_route_credits = 0;

    if ( port >= (params.p + params.a-1) ) {
        // Global port, no indirect routes.  Set credits negative so
        // it will never get chosen
		temp_vrc = -1;
        valiant_route_credits = -1;
    }
    else {
        int valiant_slice1 = td_ev->global_slice;
        int valiant_slice2 = (td_ev->global_slice + 1) % params.n;
        int valiant_route_port1 = port_for_group(td_ev->dest.mid_group_shadow, valiant_slice1, 2 );
        int valiant_route_port2 = port_for_group(td_ev->dest.mid_group_shadow, valiant_slice2, 3 );
        int valiant_route_credits1 = output_credits[valiant_route_port1 * num_vcs + vc];
        int valiant_route_credits2 = output_credits[valiant_route_port2 * num_vcs + vc];

		temp_vrc1 = valiant_route_credits1;
    	temp_vrc2 = valiant_route_credits2;

#if RUNTYPE == 0
		if(ROUTE == 2) { valiant_route_credits1 = -1; direct_route_credits=-1; }
		if(ROUTE == 3) { valiant_route_credits2 = -1; direct_route_credits=-1; }
#endif

#if RUNTYPE == 1
//	if(phase0 == true){
	//	if( isPortDown(ev,group_id, td_ev->dest.mid_group_shadow, td_ev->dest.router, valiant_route_port1) == true )
		if(r2 == true) valiant_route_credits1 = -1;
	//	if( isPortDown(ev,group_id, td_ev->dest.mid_group_shadow, valiant_rtr, valiant_route_port2) == true )
		if(r3 == true) valiant_route_credits2 = -1;
//	}
#endif
        if ( valiant_route_credits1 > valiant_route_credits2) {
	//		if(temp_vrc1 < temp_vrc2) temp_downlink=1;
            valiant_slice = valiant_slice1;
            valiant_route_port = valiant_route_port1;
            valiant_route_credits = valiant_route_credits1;
			temp_vrc = temp_vrc1;
        }
        else {
	//		if(temp_vrc1 > temp_vrc2) temp_downlink=1;
            valiant_slice = valiant_slice2;
            valiant_route_port = valiant_route_port2;
            valiant_route_credits = valiant_route_credits2;
			temp_vrc = temp_vrc2;
        }
    }

	//printf("vrc: %d drc: %d temp_vrc: %d temp_drc: %d\n", valiant_route_credits, direct_route_credits, temp_vrc, temp_drc);

	if((valiant_route_credits!=direct_route_credits) && valiant_route_credits > (int)((double)direct_route_credits*adaptive_threshold))
	{ // Use valiant route
#if RUNTYPE == 1
		if((r0==true && r1 == true) && temp_vrc < (int)((double)temp_drc*adaptive_threshold)){ downLinkEncountered++; minBlocked++; }
#endif
		valPackets++;
		td_ev->setRouting(1); //set as valiant route taken
        td_ev->dest.mid_group = td_ev->dest.mid_group_shadow;
        td_ev->setNextPort(valiant_route_port);
        td_ev->global_slice = valiant_slice;
		if(ev->getTrack() == true){
			std::cout << "taking valiant route\n";
            printf("grp: %u rtr: %u next_port: %u\n",
			group_id, router_id, valiant_route_port);
        }
    }
    else { // Use direct route
#if RUNTYPE == 1
		if((r3 == true || r2 == true) && temp_vrc > (int)((double)temp_drc*adaptive_threshold)){ downLinkEncountered++; valBlocked++; }
#endif
		minPackets++;
		td_ev->setRouting(0); //set as direct route taken
		td_ev->dest.mid_group = td_ev->dest.group;
        td_ev->setNextPort(direct_route_port);
        td_ev->global_slice = direct_slice;
		if(ev->getTrack() == true){
            std::cout << "taking direct route\n";
            printf("src: %u grp: %u rtr: %u next_port: %u\n",
            td_ev->src_group,group_id, router_id, direct_route_port);
        }
    }
	downLinkCount = downLinkEncountered;
	minBlockedCount = minBlocked;
    valBlockedCount = valBlocked;
	//dirPacketCount = minPackets;
	//valPacketCount = valPackets;

}


internal_router_event* topo_dragonfly2::process_input(RtrEvent* ev)
{
    dgnfly2Addr dstAddr = {0, 0, 0, 0};
    idToLocation(ev->request->dest, &dstAddr);

    switch (algorithm) {
    case MINIMAL:
        if ( dstAddr.group == group_id ) {
            dstAddr.mid_group = dstAddr.router;
        }
        else {
            dstAddr.mid_group = dstAddr.group;
        }
        break;
    case VALIANT:
    case ADAPTIVE_LOCAL:
        if ( dstAddr.group == group_id ) {
            // staying within group, set mid_group to be an intermediate router within group
            do {
                dstAddr.mid_group = rng->generateNextUInt32() % params.a;
                // dstAddr.mid_group = router_id;
            }
            while ( dstAddr.mid_group == router_id );
            // dstAddr.mid_group = dstAddr.group;
        } else {
            do {
                dstAddr.mid_group = rng->generateNextUInt32() % params.g;
                // dstAddr.mid_group = rand() % params.g;
            } while ( dstAddr.mid_group == group_id || dstAddr.mid_group == dstAddr.group );
        }
        break;
    }
    dstAddr.mid_group_shadow = dstAddr.mid_group;

    topo_dragonfly2_event *td_ev = new topo_dragonfly2_event(dstAddr);
    td_ev->src_group = group_id;
    td_ev->setEncapsulatedEvent(ev);
    td_ev->setVC(ev->request->vn * 3);
    td_ev->global_slice = ev->request->src % params.n;
    td_ev->global_slice_shadow = ev->request->src % params.n;

    if ( td_ev->getTraceType() != SST::Interfaces::SimpleNetwork::Request::NONE ) {
        output.output("TRACE(%d): process_input():"
                      " mid_group_shadow = %u\n",
                      td_ev->getTraceID(),
                      td_ev->dest.mid_group_shadow);
    }
    return td_ev;
}



void topo_dragonfly2::routeInitData(int port, internal_router_event* ev, std::vector<int> &outPorts)
{

    bool broadcast_to_groups = false;
    topo_dragonfly2_event *td_ev = static_cast<topo_dragonfly2_event*>(ev);
    if ( td_ev->dest.host == (uint32_t)INIT_BROADCAST_ADDR ) {
        if ( (uint32_t)port >= (params.p + params.a-1) ) {
            /* Came in from another group.
             * Send to locals, and other routers in group
             */
            for ( uint32_t p  = 0 ; p < (params.p + params.a-1) ; p++ ) {
                outPorts.push_back((int)p);
            }
        } else if ( (uint32_t)port >= params.p ) {
            /* Came in from another router in group.
             * send to hosts
             * if this is the source group, send to other groups
             */
            for ( uint32_t p = 0 ; p < params.p ; p++ ) {
                outPorts.push_back((int)p);
            }
            if ( td_ev->src_group == group_id ) {
                broadcast_to_groups = true;
                // for ( uint32_t p = (params.p+params.a-1) ; p < params.k ; p++ ) {
                //     outPorts.push_back((int)p);
                // }
            }
        } else {
            /* Came in from a host
             * Send to all other hosts and routers in group, and all groups
             */
            // for ( int p = 0 ; p < (int)params.k ; p++ ) {
            for ( int p = 0 ; p < (int)(params.p + params.a - 1) ; p++ ) {
                if ( p != port )
                    outPorts.push_back((int)p);
            }
            broadcast_to_groups = true;
        }

        if ( broadcast_to_groups ) {
            for ( int p = 0; p < (int)(params.g - 1); p++ ) {
                const RouterPortPair2& pair = group_to_global_port.getRouterPortPair(p,0);
                if ( pair.router == router_id ) outPorts.push_back((int)(pair.port));
            }
        }
    } else {
        //Not all data structures used for routing during run are
        //initialized yet, so we need to just do a quick minimal
        //routing scheme for init.
        // route(port, 0, ev);

        // TraceFunction(CALL_INFO);
        // Minimal Route
        int next_port;
        if ( td_ev->dest.group != group_id ) {
            next_port = port_for_group(td_ev->dest.group, td_ev->global_slice);
        }
        else if ( td_ev->dest.router != router_id ) {
            next_port = port_for_router(td_ev->dest.router);
        }
        else {
            next_port = td_ev->dest.host;
        }
        outPorts.push_back(next_port);
    }

}


internal_router_event* topo_dragonfly2::process_InitData_input(RtrEvent* ev)
{
    dgnfly2Addr dstAddr;
    idToLocation(ev->request->dest, &dstAddr);
    topo_dragonfly2_event *td_ev = new topo_dragonfly2_event(dstAddr);
    td_ev->src_group = group_id;
    td_ev->setEncapsulatedEvent(ev);

    return td_ev;
}





Topology::PortState topo_dragonfly2::getPortState(int port) const
{
    if ( (uint32_t)port < params.p ) return R2N;
    else return R2R;
}

std::string topo_dragonfly2::getPortLogicalGroup(int port) const
{
    if ( (uint32_t)port < params.p ) return "host";
    if ( (uint32_t)port >= params.p && (uint32_t)port < (params.p + params.a - 1) ) return "group";
    else return "global";
}

int
topo_dragonfly2::getEndpointID(int port)
{
    return (group_id * (params.a /*rtr_per_group*/ * params.p /*hosts_per_rtr*/)) +
        (router_id * params.p /*hosts_per_rtr*/) + port;
}

void
topo_dragonfly2::setOutputBufferCreditArray(int const* array, int vcs)
{
    output_credits = array;
    num_vcs = vcs;
}


void topo_dragonfly2::idToLocation(int id, dgnfly2Addr *location)
{
    if ( id == INIT_BROADCAST_ADDR) {
        location->group = (uint32_t)INIT_BROADCAST_ADDR;
        location->mid_group = (uint32_t)INIT_BROADCAST_ADDR;
        location->router = (uint32_t)INIT_BROADCAST_ADDR;
        location->host = (uint32_t)INIT_BROADCAST_ADDR;
    } else {
        uint32_t hosts_per_group = params.p * params.a;
        location->group = id / hosts_per_group;
        location->router = (id % hosts_per_group) / params.p;
        location->host = id % params.p;
    }
}


uint32_t topo_dragonfly2::router_to_group(uint32_t group)
{

    if ( group < group_id ) {
        return group / params.h;
    } else if ( group > group_id ) {
        return (group-1) / params.h;
    } else {
        output.fatal(CALL_INFO, -1, "Trying to find router to own group.\n");
        return 0;
    }
}


/* returns local router port if group can't be reached from this router */
uint32_t topo_dragonfly2::port_for_group(uint32_t group, uint32_t slice, int id)
{
    // Look up global port to use
    switch ( global_route_mode ) {
    case ABSOLUTE:
        if ( group >= group_id ) group--;
        break;
    case RELATIVE:
        if ( group > group_id ) {
            group = group - group_id - 1;
        }
        else {
            group = params.g - group_id + group - 1;
        }
        break;
    default:
        break;
    }

    const RouterPortPair2& pair = group_to_global_port.getRouterPortPair(group,slice);

    if ( pair.router == router_id ) {
        return pair.port;
    } else {
        return port_for_router(pair.router);
    }

}


uint32_t topo_dragonfly2::port_for_router(uint32_t router)
{
    uint32_t tgt = params.p + router;
    if ( router > router_id ) tgt--;
    return tgt;
}


bool isPortDown(uint32_t src_grp_id, uint32_t group_id, uint32_t router_id, int port)
{
	unsigned int packedAddress  = (src_grp_id << 24 ) | (group_id << 16) | (router_id << 8) | port;
	auto search = DL1.find((getKey(packedAddress)));
	if(search != DL1.end()){
	/*	printf("link down: %d,%d,%d,%d\n",
				((packedAddress & 0xFF000000) >> 24),
				((packedAddress & 0x00FF0000) >> 16),
				((packedAddress & 0x0000FF00) >> 8),
				(packedAddress & 0x000000FF));*/

		downLinkEncountered++;
		downLinkCount = downLinkEncountered;
		return true;
	}
	else
		return false;
}
