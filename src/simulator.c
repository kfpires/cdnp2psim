#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "peer.h"
#include "event.h"
#include "hash.h"
#include "system.h"
#include "object.h"
#include "datasource.h"
#include "randomic.h"
#include "replicate.h"
#include "cache.h"
#include "symtable.h"
#include "internals.h"
#include "topology.h"
#include "community.h"
#include "search.h"

#define LENGTH_VIDEO_CATALOG 413524


//extreme replication control policy
//
void poisonCacheReplicationPolicy(unsigned int idPeer, THashTable* hashTable, TCommunity* community){
	TListObject *listObject;
	TObject *object, *walker;
	TItemHashTable *item;
	TIdObject idVideo;
	TCache *cache;
	TReplicate *replica;
	int maxReplicas;
	//TDataCommunity *data = community->data;

	TPeer *peer = community->getPeer(community, idPeer);

	// lookup the Object into the peer's Cache
	cache = peer->getCache(peer);

	// init a item that goes on Hash Table
	item = createItemHashTable();

	listObject = cache->getObjects(cache);

	replica = (TReplicate *)peer->getReplicate(peer);
	maxReplicas = getMaxReplicasReplicateRandomic(replica);


	object = listObject->getNext(listObject,NULL);
	while(object!=NULL){

		if ( community->howManyReplicate(community, object, hashTable) >= maxReplicas ){
			getIdObject(object, idVideo);

			cache->addAvailability(cache, getStoredObject(object) );

			item->set(item, idPeer, NULL, idVideo, object);
			hashTable->removeItem(hashTable, item);

			walker = listObject->getNext(listObject, object);

			listObject->remove(listObject, object);

			object = walker;
		}
		else{
			object = listObject->getNext(listObject, object);
		}
	}

	item->dispose(item);

}

// setting up all related
//
void setupJoiningSimulator(unsigned int idPeer, THashTable* hashTable, TCommunity* community){
	TPeer *peer;

	peer = community->getPeer(community, idPeer);

	peer->setupJoining(peer);

}

void initSimulator(int simTime, TCommunity** pCommunity, TPriorityQueue** pQueue, THashTable** hashTable, TSystemInfo** systemData, char *scenarios){
	int i;
	TPeer *peer;
	TEvent *event;
	TCommunity *community;
	TPriorityQueue *queue;

	*pCommunity = createCommunity(simTime, scenarios);
	community = *pCommunity;

	*systemData = createSystemInfo(LENGTH_VIDEO_CATALOG);

	*pQueue = createMinimumPriorityQueue(32);
	queue = *pQueue;

	int sizeComm = community->getSize(community);
	for(i=0;i< sizeComm;i++){

		peer = community->getPeer(community,i);

		event = createEvent( peer->getDownSessionDuration(peer), JOIN, (TOwnerEvent) i);

		queue->enqueue(queue, event->getTime(event), event);

	}

	event = createEvent( 1, SHOW_TOPOLOGY, 0);
	queue->enqueue(queue, event->getTime(event), event);

	event = createEvent( 1, SHOW_MAPQUERY, 0);
	queue->enqueue(queue, event->getTime(event), event);

	*hashTable = createHashTable((int)LENGTH_VIDEO_CATALOG*LOAD_FACTOR_HASH_TABLE + 7);


}

// process Replicate on peers
short processReplicateSimulator(unsigned int idPeer, THashTable* hashTable,
		TCommunity* community, TSystemInfo* systemData) {
	//TCache *cache;
	//TReplicate *replicate;

	//return 1;


	// peers cache
	//cache = getCachePeer(community->peers[idPeer]);

	//replicate = getReplicatePeer(community->peers[idPeer]);

	//RunReplicate(replicate, cache, systemData, hashTable, community->peers);

	return (short) 1;
}


void processStoreSimulator(THashTable* hashTable, TCommunity* community, TSystemInfo* systemData){

	TPeer *peer = community->getPeer(community, 0);

	TDataSource *ds = peer->getDataSource(peer);

	TListObject *topK = pickTopKFromCatalogDataSource(ds,0.0001);

	topK->show(topK);

	topK->destroy(topK);

}

// Process Request from peers
void processTopologySimulator(unsigned int idPeer, THashTable* hashTable, TCommunity* community, TSystemInfo* sysInfo){
	TPeer *peer = community->getPeer(community,idPeer);

	TTopology *topo = peer->getTopologyManager(peer);

	short status = peer->getStatus(peer);
	if (status == DOWN_PEER){
		topo->stop(topo);
	}else{
		topo->run(topo, sysInfo, community);
		if (topo->IsInmaintenance(topo) && status == STALL_PEER){
			peer->setStatus(peer,UP_PEER);
		}
	}
}

// Process Request from peers
int processRequestSimulator(unsigned int idPeer, THashTable* hashTable, TCommunity* community, TSystemInfo* systemData){

	TObject *video;
	TListObject *listEvicted;
	TPeer *serverPeer;
	TDataSource *dataSource;
	TItemHashTable *item;
	unsigned int idServerPeer;
	TIdObject idVideo;
	int videoLength;

	TPeer *peer = community->getPeer(community, idPeer);

	dataSource = peer->getDataSource(peer);
	video = dataSource->pick(dataSource);

	serverPeer = community->searching(community,peer,video);

	videoLength = getLengthObject(video);

	// for on-going request, update dispatch requests
	addRequestStatsPeer( peer->getOnStats(peer), 1);

	if (serverPeer == peer ){

		peer->updateCache(peer, video, systemData);
		listEvicted = peer->getEvictedCache(peer);
		hashTable->removeEvictedItens(hashTable, idPeer, listEvicted);

		free(video);

		//Looking UP peers into keepers
	}else if ( serverPeer != NULL ){

		idServerPeer = serverPeer->getId(serverPeer);
		serverPeer->updateCacheAsServer(serverPeer,video,systemData);

		// updating hash table due to possible eviction that made room for the cached video
		listEvicted = serverPeer->getEvictedCache(serverPeer);
		hashTable->removeEvictedItens(hashTable, idServerPeer, listEvicted);


		free(video);
	}else{

		//try to insert missed video
		if ( peer->insertCache( peer, video, systemData ) ){

			getIdObject(video, idVideo);

			item = createItemHashTable();
			item->set(item, idPeer, peer, idVideo, video);
			hashTable->insert(hashTable, item);
			item->dispose(item);

			// updating hash table due to evicting that made room for the cached video
			listEvicted = peer->getEvictedCache(peer);
			hashTable->removeEvictedItens(hashTable, idPeer, listEvicted);

		}else{ // failed (cache full or unwanted video)
			free(video);
		}
	}

	return videoLength;
}


void runSimulator(unsigned int SimTime, unsigned int warmupTime, unsigned int scale, TPriorityQueue* queue, TCommunity* community, THashTable* hashTable, TSystemInfo *sysInfo){
	TEvent *event;
	TTimeEvent timeEvent;
	TTypeEvent typeEvent;
	unsigned int idPeer;
	TPeer *peer;

	int videoLength;
	float hitRate=0, missRate=0, byteHitRate, byteMissRate, hitRateCom;
	unsigned long int totalRequests=0;
	unsigned long int clock = 0;
	int peersUp;
	int k=1;
	short int first_time = 1;

	while ( clock < SimTime ){
		event = queue->dequeue(queue);
		if (!event){
			fprintf(stderr,"ERROR: simulator.c: run out of events\n");
			exit(0);
		}

		idPeer = (unsigned int)event->getOwner(event);
		clock = (TTimeEvent)event->getTime(event);
		typeEvent = event->getType(event);

		event->ufree(event);

		peer = community->getPeer(community, idPeer);

		sysInfo->setTime(sysInfo, clock);

		if (clock > k*scale){
			if (clock > warmupTime){
				if (first_time){
					community->resetStatistics(community);
					first_time = 0; // NOT TRUE
				} else {

					community->collectStatistics(community, &hitRate, &missRate, &byteHitRate, &byteMissRate, &totalRequests, &peersUp, &hitRateCom);

					printf("%lu ", clock-warmupTime);
					printf("%f %f ", hitRate, missRate);
					//printf("%f %f ", byteHitRate, byteMissRate);

					//printf("%ld ", community->onCache(community));
					//printf("%ld ", hashTable->redundancyDemand(hashTable));
					printf("%ld ",totalRequests);
					printf("%d\n",peersUp);

					//community->collectStatsOnTiers(community, clock-warmupTime);


					fflush(NULL);

				}
			}
			k = k+1;
		}

		if (typeEvent == JOIN){

			//extreme replicated data control by setting Num. Max Of Replica
			//poisonCacheReplicationPolicy(idPeer, hashTable, community);

			setupJoiningSimulator(idPeer, hashTable, community);

			// change the peer status to UP
			peer->setStatus(peer, STALL_PEER);

			// inform for community that peer is all alive
			community->setAlivePeer(community, idPeer);

			//Queuing a LEAVE event
			timeEvent = clock + peer->getUpSessionDuration(peer);
			event  = createEvent((TTimeEvent) timeEvent, LEAVE, (TOwnerEvent)idPeer);
			queue->enqueue(queue, timeEvent, event);

			//replicate content
			//timeEvent = clock + getReplicateTimePeer(community->peers[idPeer]);
			//event  = initEvent((TTimeEvent) timeEvent-1, REPLICATE, (TOwnerEvent)idPeer);
			//enQueue(queue, event);

			// Update UP time Stats
			addUpTimeStatsPeer( peer->getOnStats(peer), (timeEvent>SimTime?(SimTime-clock):(timeEvent-clock)));

//			//Queuing A Request event
//			timeEvent = clock + peer->getRequestTime(peer);
//			event  = createEvent((TTimeEvent) timeEvent, REQUEST, (TOwnerEvent)idPeer);
//			queue->enqueue(queue, timeEvent, event);

			//Queuing A Topology manager event
			timeEvent = clock + 10;
			event  = createEvent((TTimeEvent) timeEvent, TOPOLOGY, (TOwnerEvent)idPeer);
			queue->enqueue(queue, timeEvent, event);

		}else if(typeEvent == TOPOLOGY){

			short status = peer->getStatus(peer);
			processTopologySimulator(idPeer, hashTable, community, sysInfo);

			if (peer->isUp(peer) && (status == STALL_PEER)){
				//Queuing A Request event
				timeEvent = clock + peer->getRequestTime(peer);
				event  = createEvent((TTimeEvent) timeEvent, REQUEST, (TOwnerEvent)idPeer);
				queue->enqueue(queue, timeEvent, event);
			}

			//Queuing A Topology manager event
			timeEvent = clock + (status==STALL_PEER?10:130);
			event  = createEvent((TTimeEvent) timeEvent, TOPOLOGY, (TOwnerEvent)idPeer);
			queue->enqueue(queue, timeEvent, event);

		}else if(typeEvent == SHOW_TOPOLOGY){

			printf("Topology(Begin):%lu-----------------------------------------\n",clock);
			TArrayDynamic *alives = community->getAlivePeer(community);
			int xi;
			for(xi=0;xi<community->getNumberOfAlivePeer(community);xi++){
				TPeer *p = alives->getElement(alives,xi);
				TTopology *topox = p->getTopologyManager(p);
				topox->showEstablished(topox);
			}

			printf("Topology(End):%lu---------------------------------------------------\n",clock);

			timeEvent = clock + SimTime*0.05;
			event  = createEvent((TTimeEvent) timeEvent, SHOW_TOPOLOGY, (TOwnerEvent)idPeer);
			queue->enqueue(queue, timeEvent, event);

		}else if(typeEvent == SHOW_MAPQUERY){

			printf("Map Query(Begin):%lu-----------------------------------------\n",clock);
			TArrayDynamic *alives = community->getAlivePeer(community);
			int xi;
			for(xi=0;xi<community->getNumberOfAlivePeer(community);xi++){
				TPeer *p = alives->getElement(alives,xi);
				p->showMapQuery(p);
			}

			printf("Map Query(End):%lu---------------------------------------------------\n",clock);

			timeEvent = clock + SimTime*0.05;
			event  = createEvent((TTimeEvent) timeEvent, SHOW_MAPQUERY, (TOwnerEvent)idPeer);
			queue->enqueue(queue, timeEvent, event);


		}else if (typeEvent == LEAVE){ // change it with U need a model without churn
			// change the peer status to DOWN
			peer->setStatus(peer, DOWN_PEER);

			processTopologySimulator(idPeer, hashTable, community, sysInfo);

			// inform to the community that peer has left
			community->unsetAlivePeer(community, idPeer);


			//Queuing a JOIN event
			timeEvent = clock + peer->getDownSessionDuration(peer);
			event  = createEvent((TTimeEvent) timeEvent, JOIN, (TOwnerEvent)idPeer);
			queue->enqueue(queue, timeEvent, event);

			// Update DOWN time Stats
			addDownTimeStatsPeer( peer->getOnStats(peer), (timeEvent>SimTime?(SimTime-clock):(timeEvent-clock)));

		}else if( (typeEvent == REQUEST) && (peer->isUp(peer)) ){
			//Processing Request event
			videoLength = processRequestSimulator(idPeer, hashTable, community, sysInfo);

			//Queuing A Request event based on video length and the user thinking time
			timeEvent = clock + videoLength + peer->getRequestTime(peer);
			event  = createEvent((TTimeEvent) timeEvent, REQUEST, (TOwnerEvent)idPeer);
			queue->enqueue(queue, timeEvent, event);


		}else if ( (typeEvent == STORE) ){
			processStoreSimulator(hashTable,community,sysInfo);

			//Queuing A Request event based on video length and the user thinking time
			timeEvent = clock + 7*24*60*60;
			event  = createEvent((TTimeEvent) timeEvent, STORE, (TOwnerEvent)idPeer);
			queue->enqueue(queue, timeEvent, event);


		}else if ((typeEvent == REPLICATE) && (peer->isUp(peer)) ){

			//Processing Replicate event
			//processReplicateSimulator(idPeer, hashTable, community, systemData);

			//Queuing A Request event based on video length and the user thinking time
			//timeEvent = clock + getReplicateTimePeer(community->peers[idPeer]);
			//event  = initEvent((TTimeEvent) timeEvent, REPLICATE, (TOwnerEvent)idPeer);
			//enQueue(queue, event);
		}

		//dequeue processed event
	}
}

int main(int argc, char **argv){
	TCommunity* community;
	TPriorityQueue* queue;
	THashTable* hashTable;
	TSystemInfo* systemData;
	unsigned int simTime, warmupTime, scale;


	simTime = 1000*604800;
	warmupTime = 6048;
	scale = 24*60*60;

	char *entry = (char*)"/data/research/p2p/traces/music/scenario1.xml";

	initSimulator(simTime, &community, &queue, &hashTable, &systemData, entry);

	runSimulator(simTime, warmupTime, scale, queue, community, hashTable, systemData);

	//printStatCommunity(community);
	//printHashTable(hashTable);

	queue->ufree(queue);
	hashTable->dispose(hashTable);
	community->dispose(community);

	return 0;
}