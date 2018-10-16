#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "project3.h"

struct distance_table {
  int costs[MAX_NODES][MAX_NODES];
};

void init_infinity(struct distance_table *table, int numNodes);
void init_adjacent(struct distance_table *table, struct NeighborCosts *ncosts);
void send_costs(struct distance_table *table, struct NeighborCosts *ncosts, int node_id);
int should_update(struct distance_table *table, struct RoutePacket *rcvdpkt, int numNodes, int node_id);
