#include "node_util.h"
#define NODE_ID 0

struct distance_table dt0;
struct NeighborCosts *neighbor0;

extern int TraceLevel;

void printdt0( int MyNodeNumber, struct NeighborCosts *neighbor,
		struct distance_table *dtptr );
/* students to write the following two routines, and maybe some others */

void rtinit0() {
	if(TraceLevel >=1) printf("rinit0 start\n");
	//gets the neighboring direct costs
	neighbor0 = getNeighborCosts(NODE_ID);
	//set all table values to infinity
	init_infinity(&dt0, neighbor0->NodesInNetwork);
	// sets the values for the direct node connections
	init_adjacent(&dt0, neighbor0);
	//sends the cost data to its neighbors
	send_costs(&dt0, neighbor0, NODE_ID);
	if(TraceLevel >= 1){
		printdt0(NODE_ID, neighbor0, &dt0);
	}

}


void rtupdate0( struct RoutePacket *rcvdpkt ) {
	if(TraceLevel >=1) printf("rtupdate0\n");

	int updated;
	//Updates the table with new min costs
	updated = should_update(&dt0, rcvdpkt, neighbor0->NodesInNetwork, NODE_ID);
	if(updated){
		if(TraceLevel >=1) printf("Table updated, sending out update to nodes\n");
		send_costs(&dt0, neighbor0, NODE_ID);
	}
	if(TraceLevel >=1){
		printdt0(NODE_ID, neighbor0, &dt0);
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
void printdt0( int MyNodeNumber, struct NeighborCosts *neighbor, 
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
}    // End of printdt0

