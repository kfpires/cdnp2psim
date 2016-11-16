/*
 * topology.c
 *
 *  Created on: Jan 7, 2013
 *      Author: cesar
 */

#include "connection.h"
#include "internals.h"
#include "randomic.h"
#include "community.h"
#include "peer.h"
#include "topology.h"

#include "stdlib.h"
#include "stdio.h"

enum {BOOTSTRAP=1, MAINTENANCE=3, STOPPED = 4, FINISHED=5};
//int online[]={-1,-2};
//#define MAX 100

typedef struct _data_topology TDataTopology;
struct _data_topology{
	void *data_policy;
	// status evaluation
	short status;
	short cycle;
	TArrayDynamic *neighbors;
	//connection management
	TConnManager *connManager;
};

static short IsInmaintenanceTopology(TTopology *topo){
	TDataTopology  *data = topo->data;

	return (data->status == MAINTENANCE);
}

static void updateHitsStatsTopology(TTopology *topo, unsigned int idPeer, int amount){
	TDataTopology  *data = topo->data;

	TConnManager *connManager = data->connManager;

	connManager->updateHitsStats(connManager, idPeer, amount);
}

static void updateRequestsStatsTopology(TTopology *topo, unsigned int idPeer, int amount){
	TDataTopology  *data = topo->data;

	TConnManager *connManager = data->connManager;

	connManager->updateRequestsStats(connManager, idPeer, amount);
}

static void resumeTopology(TTopology *topo, void *sysInfo){
	TDataTopology  *data = topo->data;

	TConnManager *connManager = data->connManager;

	connManager->evaluateConnections(connManager, sysInfo);

	data->status = BOOTSTRAP;
}


static void stopTopology(TTopology *topo){
	TDataTopology  *data = topo->data;

	if (data->status == MAINTENANCE){
		TConnManager *connManager = data->connManager;

		connManager->teardownConnection(connManager);

		data->status = STOPPED;
	}
}

static void showNeighborhoodTopology(TTopology *topo){
	TDataTopology  *data = topo->data;

	TConnManager *connManager = data->connManager;
	if (data->status == MAINTENANCE)
		connManager->showOngoingConnections(connManager);

}

static void showEstablishedNeighorsTopology(TTopology *topo){
	TDataTopology  *data = topo->data;

	TConnManager *connManager = data->connManager;
	if (data->status == MAINTENANCE)
		connManager->showEstablishedConnections(connManager);

}

// return a array of idpeers
static void *getNeighborsTopology(TTopology *topo){
	int i, occup;
	unsigned int idClient;
	TConnection *conn;
	TDataTopology  *data = topo->data;
	TArrayDynamic *neighbors=data->neighbors;
	TConnManager *connManager = data->connManager;
	unsigned int idOwner = connManager->getIdOwner(connManager);

	neighbors->removeAll(neighbors);

	if (data->status == MAINTENANCE){
		TArrayDynamic *ongoingConnections = connManager->getListEstablishedConnections(connManager);
		occup = ongoingConnections->getOccupancy(ongoingConnections);
		for(i=0;i<occup;i++){
			conn = ongoingConnections->getElement(ongoingConnections,i);
			idClient = conn->getIdClient(conn);
			if (idClient != idOwner)
				neighbors->insert(neighbors, idClient, conn->getClient(conn) );
			else
				neighbors->insert(neighbors, conn->getIdServer(conn), conn->getServer(conn));
		}
	}
	return neighbors;
}


TTopology * createTopology(int maxconnections, int maxAttempts, unsigned int IdPeer, void *data_policy){
	TTopology *topo = malloc(sizeof(TTopology));
	TDataTopology *data = malloc(sizeof(TDataTopology));

	data->connManager = createConnManager(maxconnections,maxAttempts,IdPeer, NULL);
	data->status = BOOTSTRAP;
	data->cycle = 0;
	data->data_policy = data_policy; // to receive a topology police data
	data->neighbors = createArrayDynamic(0);

	topo->data = data;
	topo->stop = stopTopology;
	topo->show = showNeighborhoodTopology;
	topo->showEstablished = showEstablishedNeighorsTopology;
	topo->getNeighbors = getNeighborsTopology;
	topo->updateHitsStats = updateHitsStatsTopology;
	topo->updateRequestsStats = updateRequestsStatsTopology;
	topo->IsInmaintenance = IsInmaintenanceTopology;

	return topo;
}




//!functions Related to random topology maintenance
typedef struct _data_randomic_topology TDataRandomicTopology;
struct _data_randomic_topology{
	TRandomic *onlinePeer;
	TRandomic *connected;
};

void bootstrapTryRandomicTopology( TTopology *topo, void *vCommunity ){
	int i;
	TCommunity *community = vCommunity;
	TDataTopology  *data = topo->data;
	TConnManager *connManager = data->connManager;
	TArrayDynamic *online = community->getAlivePeer(community);
	int numOnline = community->getNumberOfAlivePeer(community);

	unsigned int idOwner = connManager->getIdOwner(connManager);

	for (i=0;i<numOnline;i++){
		TPeer *peer = online->getElement(online, i);
		unsigned int idPeer = peer->getId(peer);
		if (idPeer != idOwner){
			connManager->openConnection(connManager,idPeer,peer);
		}
	}
}

void bootstrapRandomicTopology(TTopology *topo, void *sysInfo, void *vCommunity){
	TDataTopology  *data = topo->data;
	TConnManager *connManager = data->connManager;
	TCommunity *community = vCommunity;
	unsigned int idOwner = connManager->getIdOwner(connManager);

	// update owner
	void *owner = community->getPeer(community,idOwner);
	connManager->setOwner(connManager,owner);

	if (!connManager->isConnected(connManager)){ // still waiting for a connection request!!!??
		if (!connManager->hasPending(connManager)){
			bootstrapTryRandomicTopology(topo, community);
		}else{
			connManager->evaluateConnections(connManager, sysInfo);
			if (connManager->isConnected(connManager))
				data->status = MAINTENANCE;
		}
	}else{
		connManager->evaluateConnections(connManager, sysInfo);
		data->status = MAINTENANCE;
	}
}


//! Random topology management policy
//
static int maintenanceFindNewNeighborRandomicTopology(TTopology *topo, void *vCommunity){
	TCommunity *community = vCommunity;
	TDataTopology  *data = topo->data;
	TConnManager *connManager = data->connManager;
	TDataRandomicTopology *data_randomic = data->data_policy;

	unsigned int idPeer, owner = connManager->getIdOwner(connManager);
	TArrayDynamic *online = community->getAlivePeer(community);
	int numOnLine = community->getNumberOfAlivePeer(community);

	int selected=-1, i, k=0;
	int *candidate = malloc(sizeof(int)*numOnLine);

	for (i=0;i<numOnLine;i++){
		TPeer *peer = online->getElement(online,i);
		idPeer = peer->getId(peer);
		if ( idPeer != owner ){
			if( !connManager->hasEstablishedConnection(connManager, idPeer ) ){
				candidate[k] = idPeer;
				k++;
			}
		}
	}

	if (k>0){
		TRandomic *onlinePeer = data_randomic->onlinePeer;

		char entry[30];sprintf(entry,"%d;",k-1);
		onlinePeer->reset(onlinePeer, entry);
		selected = onlinePeer->pick(onlinePeer) - 1; // uniform [0, k-1]
		if ((selected < 0) && (selected >= k)){
			fprintf(stderr,"FATAL:topology.c: picked peer out of range!!");
			exit(0);
		}
		selected  =  candidate[selected];
	}
	free(candidate);
	return selected;
}

static void maintenanceEvaluateSortConnectedRandomicTopology(TArrayDynamic *connected){
	int i, j, ocup;
	TConnection *conn, *actual;
	ocup = connected->getOccupancy(connected);
	for(i=1;i<ocup;i++){
		conn = connected->getElement(connected, i);
		actual = connected->getElement(connected, i-1);
		j=i-1;

		while( (j>=0) && (actual->getEstablishedSince(actual) < conn->getEstablishedSince(conn)) ){
			connected->swapElement(connected,j,j+1);
			j--;
			conn=actual;
			actual = (j>=0?connected->getElement(connected, j):NULL);
		}
	}

}

static void maintenanceEvaluateNeighborhoodRandomicTopology(TTopology *topo){
	int occupancy;
	TArrayDynamic *connected;
	TDataTopology  *data = topo->data;
	TConnManager *connManager = data->connManager;
	TDataRandomicTopology *data_randomic = data->data_policy;

	if (connManager->isFullConnected(connManager)){
		TRandomic *connectedPeer = data_randomic->connected;

		connected = connManager->getListEstablishedConnections(connManager);
		occupancy = connected->getOccupancy(connected);

		if (occupancy > 0){
			maintenanceEvaluateSortConnectedRandomicTopology(connected);

			char entry[30];sprintf(entry,"%d;",occupancy-1);
			connectedPeer->reset(connectedPeer, entry);
			unsigned int selected = connectedPeer->pick(connectedPeer);

			TConnection *conn = connected->getElement(connected,selected);
			if (conn->getIdClient(conn) != connManager->getIdOwner(connManager))
				connManager->closeConnection(connManager,conn->getIdClient(conn));
			else
				connManager->closeConnection(connManager,conn->getIdServer(conn));

		}
	}
}


static void maintenanceRandomicTopology(TTopology *topo, void *sysInfo, void *vCommunity){
	int idPeer;
	TDataTopology  *data = topo->data;
	TConnManager *connManager = data->connManager;
	TCommunity *community = vCommunity;

	connManager->evaluateConnections(connManager, sysInfo);

	if (data->cycle == 3){ // neighborhood evaluations
		idPeer = maintenanceFindNewNeighborRandomicTopology(topo, community);
		if (idPeer>=0){
			maintenanceEvaluateNeighborhoodRandomicTopology(topo);

			connManager->openConnection(connManager,(unsigned int)idPeer, community->getPeer(community, (unsigned int)idPeer));
		}
		data->cycle = 0;
	}else{
		data->cycle++;
	}
}

static void runRandomicTopology(TTopology *topo, void* sysInfo, void *community){
	TDataTopology  *data = topo->data;
	short int status = data->status;
	if(status == BOOTSTRAP){
		bootstrapRandomicTopology(topo, sysInfo, community);
	}else if (status == MAINTENANCE){
		maintenanceRandomicTopology(topo, sysInfo, community);
	}else if (status == STOPPED){
		resumeTopology(topo, sysInfo);
		bootstrapRandomicTopology(topo, sysInfo, community);
	}
}

void* createRandomTopology(void *entry){
	TDataRandomicTopology *data = malloc(sizeof(TDataRandomicTopology));
	int maxconnections, maxAttempts;
	unsigned int IdPeer;

	TParameters *p = createParameters(entry,PARAMETERS_SEPARATOR);
	p->iterator(p);

	maxconnections = atoi(p->next(p));
	maxAttempts = atoi(p->next(p));
	IdPeer = (unsigned int)atoi(p->next(p));


	char pars[20];sprintf(pars,"10;");
	data->onlinePeer = createTruncatedGeometricRandomic(pars);

	sprintf(pars,"10;");
	data->connected = createTruncatedGeometricRandomic(pars);

	TTopology *topo = createTopology(maxconnections, maxAttempts, IdPeer, data);

	topo->run = runRandomicTopology;

	p->dispose(p);

	return topo;
}



//! Similarity topology management policy
//
//!functions Related to random topology maintenance
typedef struct _data_proactive_topology TDataProactiveTopology;
struct _data_proactive_topology{
	TRandomic *pickConnected;
};

void *findNeighborSimilarToMeProactiveTopology(TPeer *peer, void *profile){
	int i;

	static TPriorityQueue *mostSimilar=NULL;
	TTopology *topo = peer->getTopologyManager(peer);
	TDataTopology  *data = topo->data;

	TArrayDynamic *neighbors = data->neighbors;
	int numNeighbors = neighbors->getOccupancy(neighbors);

	 if (mostSimilar){
		 mostSimilar->cleanup(mostSimilar);
	 }else
		 mostSimilar = createMaximumPriorityQueue(numNeighbors);

	for(i=0;i<numNeighbors;i++){
		TPeer *neighbbor = neighbors->getElement(neighbors,i);
		int degreeSimilarity = 1000*(neighbbor->runProfilePolicy(neighbbor,profile));
		mostSimilar->enqueue(mostSimilar,degreeSimilarity,neighbbor);
	}

	return mostSimilar;
}

int maintenanceFindNewNeighborProactiveTopology(TTopology *topo, void *vCommunity){
	TDataTopology  *data = topo->data;
	TArrayDynamic *neighbors = data->neighbors;
	int numNeighbors = neighbors->getOccupancy(neighbors);

	if (numNeighbors==0)
		return -1;

	TConnManager *connManager = data->connManager;
	TPeer *peer = connManager->getOwner(connManager);
	unsigned int idPeer = peer->getId(peer);


	TDataProactiveTopology *data_policy = data->data_policy;
	TRandomic *pickConnected = data_policy->pickConnected;

	char entry[30];sprintf(entry,"%d;",numNeighbors);
	pickConnected->reset(pickConnected, entry);
	unsigned int i = pickConnected->pick(pickConnected)-1; // uniform [1, k]

	TPeer *pickedNeighbbor = neighbors->getElement(neighbors,i);

	void *profile = peer->getProfile(peer);
	TPriorityQueue *pq = findNeighborSimilarToMeProactiveTopology(pickedNeighbbor, profile);

	while( !(pq->isEmpty(pq)) ){
		TPeer *similarPeer = pq->dequeue(pq);
		unsigned int idSimilar = similarPeer->getId(similarPeer);

		if (idSimilar != idPeer ){
			if( !connManager->hasEstablishedConnection(connManager, idSimilar ) ){
				return idSimilar;
			}
		}
	}

	return -1;
}

int maintenanceEvaluateSortConnectedProactiveTopology(TTopology *topo, TArrayDynamic *connected){
	int i, occup;
	TConnection *conn;
	TDataTopology *data = topo->data;

	TConnManager *cm = data->connManager;
	unsigned int idOwner = cm->getIdOwner(cm);
	float minIndexSatisfaction, indexSatisfaction;
	int k = 0;
	occup = connected->getOccupancy(connected);

	conn = connected->getElement(connected, 0);
	minIndexSatisfaction = conn->getIndexSatisfactionStats(conn, idOwner);

	for(i=1;i<occup;i++){
		conn = connected->getElement(connected, i);
		indexSatisfaction = conn->getIndexSatisfactionStats(conn, idOwner);

		if (indexSatisfaction < minIndexSatisfaction){
			minIndexSatisfaction = indexSatisfaction;
			k=i;
		}
	}

	return k;

}


static void maintenanceEvaluateNeighborhoodProactiveTopology(TTopology *topo){
	int occupancy;
	TArrayDynamic *connected;
	TDataTopology  *data = topo->data;
	TConnManager *connManager = data->connManager;
	//TDataProactiveTopology *data_proactive = data->data_policy;

	if (connManager->isFullConnected(connManager)){
		//TRandomic *pickConnected = data_proactive->pickConnected;

		connected = connManager->getListEstablishedConnections(connManager);
		occupancy = connected->getOccupancy(connected);

		if (occupancy > 0){
			int selected = maintenanceEvaluateSortConnectedProactiveTopology(topo, connected);

			TConnection *conn = connected->getElement(connected,selected);
			if (conn->getIdClient(conn) != connManager->getIdOwner(connManager))
				connManager->closeConnection(connManager,conn->getIdClient(conn));
			else
				connManager->closeConnection(connManager,conn->getIdServer(conn));

		}
	}
}


static void maintenanceProactiveTopology(TTopology *topo, void *sysInfo, void *vCommunity){
	int idPeer;
	TDataTopology  *data = topo->data;
	TConnManager *connManager = data->connManager;
	TCommunity *community = vCommunity;

	connManager->evaluateConnections(connManager, sysInfo);
	if (data->cycle == 3){ // neighborhood evaluations
		idPeer = maintenanceFindNewNeighborProactiveTopology(topo, community);
		if (idPeer>=0){
			maintenanceEvaluateNeighborhoodProactiveTopology(topo);

			connManager->openConnection(connManager,(unsigned int)idPeer, community->getPeer(community, (unsigned int)idPeer));
		}
		data->cycle = 0;
	}else{
//		maintenanceEvaluateNeighborhoodProactiveTopology(topo);
		data->cycle++;
	}

}

void bootstrapTryProactiveTopology( TTopology *topo, void *vCommunity ){
	int i;
	TCommunity *community = vCommunity;
	TDataTopology  *data = topo->data;
	TConnManager *connManager = data->connManager;
	TArrayDynamic *online = community->getAlivePeer(community);
	int numOnline = community->getNumberOfAlivePeer(community);

	unsigned int idOwner = connManager->getIdOwner(connManager);
	TPeer *owner = connManager->getOwner(connManager);
	void *profile = owner->getProfile(owner);

	 TPriorityQueue *pq = createMaximumPriorityQueue(numOnline);

	 for(i=0;i<numOnline;i++){
		 TPeer *onlinePeer = online->getElement(online,i);
		 int degreeSimilarity = 1000*(onlinePeer->runProfilePolicy(onlinePeer,profile));
		 pq->enqueue(pq,degreeSimilarity,onlinePeer);
	 }

	 while( !(pq->isEmpty(pq)) ){
		 TPeer *similarPeer = pq->dequeue(pq);
		 unsigned int idSimilar = similarPeer->getId(similarPeer);

		 if (idSimilar != idOwner ){
			 connManager->openConnection(connManager,idSimilar,similarPeer);
		 }
		 pq->dequeue(pq);
	 }

	 pq->ufree(pq);
}



void bootstrapProactiveTopology(TTopology *topo, void *sysInfo, void *vCommunity){
	TDataTopology  *data = topo->data;
	TConnManager *connManager = data->connManager;
	TCommunity *community = vCommunity;
	unsigned int idOwner = connManager->getIdOwner(connManager);

	// update owner
	void *owner = community->getPeer(community,idOwner);
	connManager->setOwner(connManager,owner);

	if (!connManager->isConnected(connManager)){ // still waiting for a connection request!!!??
		if (!connManager->hasPending(connManager)){
			bootstrapTryProactiveTopology(topo, community);
		}else{
			connManager->evaluateConnections(connManager, sysInfo);
			if (connManager->isConnected(connManager))
				data->status = MAINTENANCE;
		}
	}else{
		connManager->evaluateConnections(connManager, sysInfo);
		data->status = MAINTENANCE;
	}
}

static void runProactiveTopology(TTopology *topo, void* sysInfo, void *community){
	TDataTopology  *data = topo->data;
	short int status = data->status;
	if(status == BOOTSTRAP){
		bootstrapProactiveTopology(topo, sysInfo, community);
	}else if (status == MAINTENANCE){
		maintenanceProactiveTopology(topo, sysInfo, community);
	}else if (status == STOPPED){
		resumeTopology(topo, sysInfo);
		bootstrapProactiveTopology(topo, sysInfo, community);
	}
}





void* createProactiveTopology(void *entry){
	TDataProactiveTopology *data = malloc(sizeof(TDataProactiveTopology));
	int maxconnections, maxAttempts;
	unsigned int IdPeer;

	TParameters *p = createParameters(entry,PARAMETERS_SEPARATOR);
	p->iterator(p);

	maxconnections = atoi(p->next(p));
	maxAttempts = atoi(p->next(p));
	IdPeer = (unsigned int)atoi(p->next(p));

	char pars[20];sprintf(pars,"10;");
	data->pickConnected = createTruncatedGeometricRandomic(pars);

	TTopology *topo = createTopology(maxconnections, maxAttempts, IdPeer, data);

	topo->run = runProactiveTopology;

	p->dispose(p);

	return topo;
}


//@ Functions related to Session Topology
//! Session topology management policy
//

typedef struct _data_session_topology TDataSessionTopology;
struct _data_session_topology{
	TRandomic *pickConnected;
};

void *findNeighborSimilarToMeSessionTopology(TPeer *peer, void *profile){
	int i;

	static TPriorityQueue *mostSimilar=NULL;
	TTopology *topo = peer->getTopologyManager(peer);
	TDataTopology  *data = topo->data;

	TArrayDynamic *neighbors = data->neighbors;
	int numNeighbors = neighbors->getOccupancy(neighbors);

	 if (mostSimilar){
		 mostSimilar->cleanup(mostSimilar);
	 }else
		 mostSimilar = createMaximumPriorityQueue(numNeighbors);

	for(i=0;i<numNeighbors;i++){
		TPeer *neighbbor = neighbors->getElement(neighbors,i);
		//@ grau de similaridade sera o tempo de entrada mais curto entre os pares
		int degreeSimilarity = (neighbbor->runProfilePolicy(neighbbor,profile));
		mostSimilar->enqueue(mostSimilar,degreeSimilarity,neighbbor);
	}

	return mostSimilar;
}

int maintenanceFindNewNeighborSessionTopology(TTopology *topo, void *vCommunity){
	TDataTopology  *data = topo->data;
	TArrayDynamic *neighbors = data->neighbors;
	int numNeighbors = neighbors->getOccupancy(neighbors);

	if (numNeighbors==0)
		return -1;

	TConnManager *connManager = data->connManager;
	TPeer *peer = connManager->getOwner(connManager);
	unsigned int idPeer = peer->getId(peer);

	TDataSessionTopology *data_policy = data->data_policy;
	TRandomic *pickConnected = data_policy->pickConnected;

	char entry[30];sprintf(entry,"%d;",numNeighbors);
	pickConnected->reset(pickConnected, entry);
	unsigned int i = pickConnected->pick(pickConnected)-1; // uniform [1, k]
	TPeer *pickedNeighbbor = neighbors->getElement(neighbors,i);

	void *profile = peer->getProfile(peer);
	TPriorityQueue *pq = findNeighborSimilarToMeSessionTopology(pickedNeighbbor, profile);

	while( !(pq->isEmpty(pq)) ){
		TPeer *similarPeer = pq->dequeue(pq);
		unsigned int idSimilar = similarPeer->getId(similarPeer);

		if (idSimilar != idPeer ){
			if( !connManager->hasEstablishedConnection(connManager, idSimilar ) ){
				return idSimilar;
			}
		}
	}

	return -1;
}

int maintenanceEvaluateSortConnectedSessionTopology(TTopology *topo, TArrayDynamic *connected){
	int i, occup;
	TConnection *conn;
	TDataTopology *data = topo->data;

	TConnManager *cm = data->connManager;
	unsigned int idOwner = cm->getIdOwner(cm);
	float minIndexSatisfaction, indexSatisfaction;
	int k = 0;
	occup = connected->getOccupancy(connected);

	conn = connected->getElement(connected, 0);
	minIndexSatisfaction = conn->getIndexSatisfactionStats(conn, idOwner);

	for(i=1;i<occup;i++){
		conn = connected->getElement(connected, i);
		indexSatisfaction = conn->getIndexSatisfactionStats(conn, idOwner);

		if (indexSatisfaction < minIndexSatisfaction){
			minIndexSatisfaction = indexSatisfaction;
			k=i;
		}
	}

	return k;

}


static void maintenanceEvaluateNeighborhoodSessionTopology(TTopology *topo){
	int occupancy;
	TArrayDynamic *connected;
	TDataTopology  *data = topo->data;
	TConnManager *connManager = data->connManager;
	//TDataSessionTopology *data_session = data->data_policy;

	if (connManager->isFullConnected(connManager)){
		//TRandomic *pickConnected = data_session->pickConnected;

		connected = connManager->getListEstablishedConnections(connManager);
		occupancy = connected->getOccupancy(connected);

		if (occupancy > 0){
			int selected = maintenanceEvaluateSortConnectedSessionTopology(topo, connected);

			TConnection *conn = connected->getElement(connected,selected);
			if (conn->getIdClient(conn) != connManager->getIdOwner(connManager))
				connManager->closeConnection(connManager,conn->getIdClient(conn));
			else
				connManager->closeConnection(connManager,conn->getIdServer(conn));

		}
	}
}


static void maintenanceSessionTopology(TTopology *topo, void *sysInfo, void *vCommunity){
	int idPeer;
	TDataTopology  *data = topo->data;
	TConnManager *connManager = data->connManager;
	TCommunity *community = vCommunity;

	connManager->evaluateConnections(connManager, sysInfo);
	if (data->cycle == 3){ // neighborhood evaluations
		idPeer = maintenanceFindNewNeighborSessionTopology(topo, community);
		if (idPeer>=0){
			maintenanceEvaluateNeighborhoodSessionTopology(topo);

			connManager->openConnection(connManager,(unsigned int)idPeer, community->getPeer(community, (unsigned int)idPeer));
		}
		data->cycle = 0;
	}else{
//		maintenanceEvaluateNeighborhoodSessionTopology(topo);
		data->cycle++;
	}

}

void bootstrapTrySessionTopology( TTopology *topo, void *vCommunity ){
	int i;
	TCommunity *community = vCommunity;
	TDataTopology  *data = topo->data;
	TConnManager *connManager = data->connManager;
	TArrayDynamic *online = community->getAlivePeer(community);
	int numOnline = community->getNumberOfAlivePeer(community);

	unsigned int idOwner = connManager->getIdOwner(connManager);
	TPeer *owner = connManager->getOwner(connManager);
	void *profile = owner->getProfile(owner);

	// Falta preencher *pq
	 TPriorityQueue *pq = createMaximumPriorityQueue(numOnline);

	 for(i=0;i<numOnline;i++){
		 TPeer *onlinePeer = online->getElement(online,i);

		 //Aqui criterio da topologia Session
		 int degreeSimilarity = (onlinePeer->runProfilePolicy(onlinePeer,profile));//@
		 pq->enqueue(pq,degreeSimilarity,onlinePeer);
	 }

	 while( !(pq->isEmpty(pq)) ){
		 TPeer *similarPeer = pq->dequeue(pq);
		 unsigned int idSimilar = similarPeer->getId(similarPeer);

		 if (idSimilar != idOwner ){
			 connManager->openConnection(connManager,idSimilar,similarPeer);
		 }
		 pq->dequeue(pq);
	 }

	 pq->ufree(pq);
}



void bootstrapSessionTopology(TTopology *topo, void *sysInfo, void *vCommunity){
	TDataTopology  *data = topo->data;
	TConnManager *connManager = data->connManager;
	TCommunity *community = vCommunity;
	unsigned int idOwner = connManager->getIdOwner(connManager);

	// update owner
	void *owner = community->getPeer(community,idOwner);
	connManager->setOwner(connManager,owner);

	if (!connManager->isConnected(connManager)){ // still waiting for a connection request!!!??
		if (!connManager->hasPending(connManager)){
			//tentativa de conexao com as politicas da topologia session
			bootstrapTrySessionTopology(topo, community);
		}else{
			connManager->evaluateConnections(connManager, sysInfo);
			if (connManager->isConnected(connManager))
				data->status = MAINTENANCE;
		}
	}else{
		connManager->evaluateConnections(connManager, sysInfo);
		data->status = MAINTENANCE;
	}
}

static void runSessionTopology(TTopology *topo, void* sysInfo, void *community){
	TDataTopology  *data = topo->data;
	short int status = data->status;
	if(status == BOOTSTRAP){
		bootstrapSessionTopology(topo, sysInfo, community);
	}else if (status == MAINTENANCE){
		maintenanceSessionTopology(topo, sysInfo, community);
	}else if (status == STOPPED){
		resumeTopology(topo, sysInfo);
		bootstrapSessionTopology(topo, sysInfo, community);
	}
}


void* createSessionTopology(void *entry){
	TDataSessionTopology *data = malloc(sizeof(TDataSessionTopology));
	int maxconnections, maxAttempts;
	unsigned int IdPeer;

	TParameters *p = createParameters(entry,PARAMETERS_SEPARATOR);
	p->iterator(p);

	maxconnections = atoi(p->next(p));
	maxAttempts = atoi(p->next(p));
	IdPeer = (unsigned int)atoi(p->next(p));

	char pars[20];sprintf(pars,"10;");

	// Como preencher tipo data ?
	data->pickConnected = createUniformRandomic(pars);

	//Create topology
	TTopology *topo = createTopology(maxconnections, maxAttempts, IdPeer, data);

	topo->run = runSessionTopology;

	p->dispose(p);

	return topo;
}































































//#include "randomic.h"
//#include "system.h"
//
////#define MAX 6
//struct _peer{
//	int id;
//	TTopology *topo;
//};
//
//
//int main(){
//	TSystemInfo* sysInfo = createSystemInfo(1000);
//	struct _peer p[MAX];
//	TTopology *topo;
//	unsigned int i, tm=0, op;
//
//	for (i=0;i<MAX;i++){
//		p[i].id = i;
//		p[i].topo = createRandomTopology(10, 20, i);
//	}
//	unsigned int p1=0;
//	int status=0;
//	char entry[30];sprintf(entry,"%d;",MAX);
//	TRandomic *rp = createUniformRandomic(entry);
//	TRandomic *ro = createUniformRandomic("3;");
//	do{
//		sysInfo->setTime(sysInfo, tm);
//		// sorteia pares
//		p1=(p1+1==MAX?0:p1+1);// = rp->pick(rp)-1;
//
//		op = ro->pick(ro);
//
//		if (op == op){ // conexao
//			topo = p[p1].topo;
//			online[p1] = p1;
//			topo->run(topo, sysInfo);
//			//printf("----tm:%d p:%d\n", tm, p1);
//			//topo->showEstablished(topo);
//			status++;
//		}
//
//		if ( (tm%(100+p1))==0 ){ // desconexao a partir do cliente
//
//
//			printf("STOPPING: %d\n",p1);
//			topo = p[p1].topo;
//			topo->stop(topo);
//			online[p1] = -1;
//
//			for(i=0;i<MAX;i++){
//
//				TTopology *topox = p[i].topo;
//
//				topox->showEstablished(topox);
//			}
//			printf("---------------\n");
//
//		}
//
//		// avança o tempo (tm)
//
//		tm++;
//	}while(tm<4000);
//
//
//
//	return 0;
//}
//
