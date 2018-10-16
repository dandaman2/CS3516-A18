#include "node_util.h"

#define NODE_ID 2

extern int TraceLevel;

struct distance_table dt2;
struct NeighborCosts   *neighbor2;

void printdt2( int MyNodeNumber, struct NeighborCosts *neighbor,
		struct distance_table *dtptr );

/* students to write the following two routines, and maybe some others */

void rtinit2() {
	if(TraceLevel >=1)printf("rinit2 start\n");
	//gets the neighboring direct costs
	neighbor2 = getNeighborCosts(NODE_ID);
	//set all table values to infinity
	init_infinity(&dt2, neighbor2->NodesInNetwork);
	// sets the values for the direct node connections
	init_adjacent(&dt2, neighbor2);
	//sends the cost data to its neighbors
	send_costs(&dt2, neighbor2, NODE_ID);
	if(TraceLevel >= 1){
		printdt2(NODE_ID, neighbor2, &dt2);
	}

}


void rtupdate2( struct RoutePacket *rcvdpkt ) {
	if(TraceLevel >=1)printf("rtupdate2\n");

	int updated;
	//Updates the table with new min costs
	updated = should_update(&dt2, rcvdpkt, neighbor2->NodesInNetwork, NODE_ID);
	if(updated){
		if(TraceLevel >=1) printf("Table updated, sending out update to nodes\n");
		send_costs(&dt2, neighbor2, NODE_ID);
	}
	if(TraceLevel >=1){
		printdt2(NODE_ID, neighbor2, &dt2);
	}
}


/////////////////////////////////////////////////////////////////////
//  printdt
//  This routine is being supplied to you.  It is the same code in
//  each node and is tailored based on the input arguments.
//  Required arguments:
//  MyNodeNumber:  This routine assumes that you know your node
//                 number and supply it when making this call.
//  struct NeighborCosts *neighbor:  A pointer to the structure 
//                 that's supplied via a call to getNeighborCosts().
//                 It tells this print routine the configuration
//                 of nodes surrounding the node we're working on.
//  struct distance_table *dtptr: This is the running record of the
//                 current costs as seen by this node.  It is 
//                 constantly updated as the node gets new
//                 messages from other nodes.
/////////////////////////////////////////////////////////////////////
void printdt2( int MyNodeNumber, struct NeighborCosts *neighbor, 
		struct distance_table *dtptr ) {
    int       i, j;
    int       TotalNodes = neighbor->NodesInNetwork;     // Total nodes in network
    int       NumberOfNeighbors = 0;                     // How many neighbors
    int       Neighbors[MAX_NODES];                      // Who are the neighbors

    // Determine our neighbors 
    for ( i = 0; i < TotalNodes; i++ )  {
        if (( neighbor->NodeCosts[i] != INFINITY ) && i != MyNodeNumber )  {
            Neighbors[NumberOfNeighbors] = i;
            NumberOfNeighbors++;
        }
    }
    // Print the header
    printf("                via     \n");
    printf("   D%d |", MyNodeNumber );
    for ( i = 0; i < NumberOfNeighbors; i++ )
        printf("     %d", Neighbors[i]);
    printf("\n");
    printf("  ----|-------------------------------\n");

    // For each node, print the cost by travelling thru each of our neighbors
    for ( i = 0; i < TotalNodes; i++ )   {
        if ( i != MyNodeNumber )  {
            printf("dest %d|", i );
            for ( j = 0; j < NumberOfNeighbors; j++ )  {
                    printf( "  %4d", dtptr->costs[i][Neighbors[j]] );
            }
            printf("\n");
        }
    }
    printf("\n");
}    // End of printdt2

