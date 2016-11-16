/*
 * topology.h
 *
 *  Created on: Jan 10, 2013
 *      Author: cesar
 */

#ifndef TOPOLOGY_H_
#define TOPOLOGY_H_

typedef struct topology TTopology;
typedef void (*TRunTopology)(TTopology *topo, void* sysInfo, void *community);
typedef void (*TStopTopology)(TTopology *topo);
typedef void (*TShowTopology)(TTopology *topo);
typedef void* (*TGetNeighborsTopology)(TTopology *topo);
typedef void (*TUpdateHitsStatsTopology)(TTopology *topo, unsigned int idPeer, int amount);
typedef void (*TUpdateRequestsStatsTopology)(TTopology *topo, unsigned int idPeer, int amount);
typedef short (*TIsInmaintananceTopology)(TTopology *topo);

struct topology{
	void *data;
	TRunTopology run;
	TStopTopology stop;
	TShowTopology show;
	TShowTopology showEstablished;
	TGetNeighborsTopology getNeighbors;
	TUpdateHitsStatsTopology updateHitsStats;
	TUpdateRequestsStatsTopology updateRequestsStats;
	TIsInmaintananceTopology IsInmaintenance;
};

void *createRandomTopology();
void *createProactiveTopology();
void* createSessionTopology(void *entry);//@



#endif /* TOPOLOGY_H_ */
