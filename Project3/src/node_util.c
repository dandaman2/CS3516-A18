#include "node_util.h"

extern float clocktime;
extern int TraceLevel;

//Initializes all node table values to infinity
void init_infinity(struct distance_table *table, int numNodes){
	int r, c;
	for(r = 0; r < numNodes; r++){
		for(c =0; c < numNodes; c++){
			table->costs[r][c] = INFINITY;
		}
	}
}

//Initializes the intial adjacent node values into the table
void init_adjacent(struct distance_table *table, struct NeighborCosts *ncosts){
	int d;
	for(d = 0; d < ncosts->NodesInNetwork; d++){
		table->costs[d][d] = ncosts->NodeCosts[d];
	}
}
//send the costs data to its neighbors
void send_costs(struct distance_table *table, struct NeighborCosts *ncosts, int node_id){

	int i, j, k, temp_min;
	int numNodes = ncosts->NodesInNetwork;
	int min_dists[numNodes];

	//calculate the cost minimums
	for(i =0; i<numNodes; i++){
		temp_min = INFINITY;
		for(j = 0; j<numNodes; j++){
			if(table->costs[i][j] < temp_min){
				temp_min = table->costs[i][j];
			}
		}
		min_dists[i] = temp_min;
	}

	for(i = 0; i < numNodes; i++){
		if((i != node_id) && ncosts->NodeCosts[i] != INFINITY){
			//create the packet to send out
			struct RoutePacket* packet = malloc(sizeof(struct RoutePacket));
			packet->sourceid = node_id;
			packet->destid = i;
			if(TraceLevel >= 1){
				printf("Copying mins into packet\n");
			}
			memcpy(packet->mincost, min_dists, sizeof(packet->mincost));
			if(TraceLevel >= 1){
				printf("At time t= %f, node %d sent packet to node %d with data:",
						clocktime, packet->sourceid, packet->destid);
				for(k = 0; k < numNodes; k++){
					printf("%d ", min_dists[k]);
				}
				printf("\n");
			}
			//send the packet to layer 2
			toLayer2(*packet);
		}
	}
}

//determines whether an update to the distance table is needed
int should_update(struct distance_table *table, struct RoutePacket *rcvdpkt, int numNodes, int node_id){
	int rcvd_sourceid = rcvdpkt->sourceid;
	int update = 0;
	int i;
	if(TraceLevel >= 1){
		printf("At time t=%f, rtupdate() called. node %d receives a packet from node %d\n",
		    		clocktime, node_id, rcvd_sourceid);
	}
	//Bellman-Ford for determining new lowest cost
	for(i =0; i <numNodes; i++){
		int cur_cost = table->costs[rcvd_sourceid][rcvd_sourceid] + rcvdpkt->mincost[i];
		int prev_cost = table ->costs[i][rcvd_sourceid];
		if(cur_cost < prev_cost){
			table->costs[i][rcvd_sourceid] = cur_cost;
			update++;
		}
	}
	return update;
}
