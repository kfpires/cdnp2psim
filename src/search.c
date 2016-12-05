/*
 * search.c
 *
 *  Created on: Jan 15, 2013
 *      Author: cesar
 */
#include "internals.h"
#include "community.h"
#include "peer.h"
#include "hierarchy.h"
#include "topology.h"
#include "randomic.h"
#include "search.h"

#include "stdio.h"
#include "stdlib.h"

typedef struct _data_search TDataSearch;
struct _data_search{
	void *data_policy;
};

typedef struct _data_randomwalk_search TDataRandomWalkSearch;
struct _data_randomwalk_search{
	TRandomic *walker;
	short maxTries;
};

// random walk
static void* runRandomWalkSearch(TSearch *search, void *vPeer, void *object, unsigned int clientId, float prefetchFraction ){
	TPeer *peer = vPeer;
	TDataSearch *data = search->data;
	TDataRandomWalkSearch *data_rwalk = data->data_policy;
	TRandomic *walker = data_rwalk->walker;
	unsigned int idRequester=-1, idSource = peer->getId(peer);
	short found = 0, tries=-1;
	do{
		TTopology *topo = peer->getTopologyManager(peer);
		found = 1;

		if (!peer->canStream(peer,object, clientId, prefetchFraction)){
			TArrayDynamic *neighbors = topo->getNeighbors(topo);
			if (idRequester >= 0){
				topo->updateRequestsStats(topo, idRequester, 1);
				peer->updateRequestsMapQuery(peer,idSource,tries+1);
			}
			idRequester = peer->getId(peer);

			int numNeighbors = neighbors->getOccupancy(neighbors);
			if (numNeighbors){
				char entry[30];sprintf(entry,"%d;",numNeighbors);
				walker->reset(walker, entry);
				int i = walker->pick(walker)-1;
				peer = neighbors->getElement(neighbors,i);
			}else{
				tries = data_rwalk->maxTries+1;
			}
			found = 0;
		}else{
			if (idRequester >= 0){
				topo->updateHitsStats(topo, idRequester, 1);
				peer->updateHitsMapQuery(peer,idSource,tries+1);
			}

		}
		tries++;

	}while( (tries <= data_rwalk->maxTries) && (!found) );

	if (tries > data_rwalk->maxTries){
		peer = NULL;
	}
	return peer;
}

void uFreeRandomWalkSearch(TSearch *search){
	TDataSearch *data = search->data;
	TDataRandomWalkSearch *data_rw = data->data_policy;

	free(data_rw);
	free(data);
	free(search);
}

TSearch *createRandomWalkSearch(char *entry){
	TSearch *search = malloc(sizeof(TSearch));
	TDataSearch *data = malloc(sizeof(TDataSearch));
	TDataRandomWalkSearch *data_rw = malloc(sizeof(TDataRandomWalkSearch));

    // extract parameters from user's entry limited by semi-color
    TParameters *lp = createParameters(entry, PARAMETERS_SEPARATOR);

    lp->iterator(lp);


    // walk through the parameters as setting up
    data_rw->maxTries = atoi(lp->next(lp));

    data_rw->walker = createUniformRandomic("10;");

	data->data_policy = data_rw;

	search->data = data;
	search->run = runRandomWalkSearch;
	search->ufree = uFreeRandomWalkSearch;

	lp->dispose(lp);

	return search;
}

//
typedef struct _data_floading_search TDataFloadingSearch;
struct _data_floading_search{
	int maxlevel;
};

typedef struct item_floading_search TItemFloadingSearch;
struct item_floading_search{
	int level;
	int idRequester;
	void *peer;
};

static TItemFloadingSearch *createItemFloadingSearch(int level, int idRequester, void* peer){
	TItemFloadingSearch *item = malloc(sizeof(TItemFloadingSearch));

	item->level  = level;
	item->peer = peer;
	item->idRequester = idRequester;

	return item;
}

static void* runFloadingSearch(TSearch *search, void* vPeer, void* object, unsigned int clientId, float prefetchFraction){
	TPeer *peer = vPeer;
	TDataSearch *data = search->data;
	TDataFloadingSearch *data_floading = data->data_policy;
	TArrayDynamic *visited = createArrayDynamic(10);
	TPriorityQueue *pq = createMaximumPriorityQueue(10);

	short found = 0, deadend=0;
	int i, occup;
	TItemFloadingSearch *item;
	unsigned int level=0, 	idPeer = peer->getId(peer), idSource = idPeer;
	int idRequester = -1;

	visited->insert(visited, idPeer, peer);
	do{
		TTopology *topo = peer->getTopologyManager(peer);

		if (!peer->canStream(peer,object,clientId, prefetchFraction)){
			TArrayDynamic *neighbors = topo->getNeighbors(topo);
			if (idRequester>=0){
				topo->updateRequestsStats(topo,idRequester,1);
				peer->updateRequestsMapQuery(peer,idSource, level);
			}
			idRequester = peer->getId(peer);
			occup = neighbors->getOccupancy(neighbors);
			for(i=0;i<occup;i++){  // BFS on neighborhood
				peer = neighbors->getElement(neighbors,i);
				idPeer = peer->getId(peer);
				if (!visited->retrieval(visited, idPeer)){
					visited->insert(visited, idPeer, peer);
					pq->enqueue(pq,level+1,createItemFloadingSearch(level+1, idRequester, peer));
				}
			}
			item =pq->dequeue(pq);
			if (item){
				level = item->level;
				peer = item->peer;
				idRequester = item->idRequester;
				free(item);
			}else{
				deadend = 1;
			}

		}else{
			found = 1;
			if (idRequester>=0){
				topo->updateHitsStats(topo,idRequester,1);
				peer->updateHitsMapQuery(peer,idSource, level);
			}
		}

	}while( (!found) && (!deadend) && (level<=data_floading->maxlevel) );

	item =pq->dequeue(pq);
	while(item){
		free(item);
		item =pq->dequeue(pq);
	}
	pq->ufree(pq);

	visited->ufree(visited);

	if (!found)
		peer = NULL;

	return peer;
}

static void ufreeFloadingSearch(TSearch *search){
	TDataSearch *data = search->data;
	TDataFloadingSearch *data_floading = data->data_policy;

	free(data_floading);
	free(data);
	free(search);
}

TSearch *createFloadingSearch(char *entry){
	TSearch *search = malloc(sizeof(TSearch));
	TDataSearch *data = malloc(sizeof(TDataSearch));
	TDataFloadingSearch *data_floading = malloc(sizeof(TDataFloadingSearch));

    TParameters *lp = createParameters(entry, PARAMETERS_SEPARATOR);

    lp->iterator(lp);

	data_floading->maxlevel = atoi(lp->next(lp));

	data->data_policy = data_floading;

	search->data = data;
	search->run = runFloadingSearch;
	search->ufree = ufreeFloadingSearch;

	lp->dispose(lp);

	return search;
}

// Busca na vizinhança


//
typedef struct _data_neighborhood_search TDataNeighborhoodSearch;
struct _data_neighborhood_search{
	int maxlevel;
};

typedef struct item_neighborhood_search TItemNeighborhoodSearch;
struct item_neighborhood_search{
	int level;
	int idRequester;
	void *peer;
};

/*static TItemNeighborhoodSearch *createItemNeighborhoodSearch(int level, int idRequester, void* peer){
	TItemNeighborhoodSearch *item = malloc(sizeof(TItemNeighborhoodSearch));

	item->level  = level;
	item->peer = peer;
	item->idRequester = idRequester;

	return item;
}*/

static void* runNeighborhoodSearch(TSearch *search, void* vPeer, void* object, unsigned int clientId, float prefetchFraction){
	TPeer *peer = vPeer;
	TDataSearch *data = search->data;
	TDataNeighborhoodSearch *data_neighborhood = data->data_policy;
	TArrayDynamic *listPeers = createArrayDynamic(1);
	//TPriorityQueue *pq = createMaximumPriorityQueue(10);
	//TPeer **listPeers;
	THCache *hcache;

	short found = 0;
	int i, occup;
	int levelInit, levelEnd;
	//TItemNeighborhoodSearch *item;
	unsigned int level=0, 	idPeer = peer->getId(peer), idSource = idPeer;
	int idRequester = -1;

	//visited->insert(visited, idPeer, peer);
	//do{
	TTopology *topo = peer->getTopologyManager(peer);

	//if (!peer->canStream(peer,object,clientId, prefetchFraction)){
	TArrayDynamic *neighbors = topo->getNeighbors(topo);
	if (idRequester>=0){
		topo->updateRequestsStats(topo,idRequester,1);
		peer->updateRequestsMapQuery(peer,idSource, level);
	}
	idRequester = peer->getId(peer);
	occup = neighbors->getOccupancy(neighbors);

	//listPeers = malloc(1 * sizeof(TPeer*));//Cria vetor com lista de pares a retornar

	for(i=0;i<occup;i++){  // BFS on neighborhood

		peer = neighbors->getElement(neighbors,i);
		idPeer = peer->getId(peer);
		//if (peer->canStream(peer,object,clientId, prefetchFraction)){
			//idPeer = peer->getId(peer);
		/*	if (!visited->retrieval(visited, idPeer)){
				visited->insert(visited, idPeer, peer);*/

				hcache=peer->getHCache(peer);
				levelInit=0;
				levelEnd=hcache->getLevels(hcache);
				if(hcache->search(hcache,object,levelInit, levelEnd)!=NULL){
					listPeers->insert(listPeers,idPeer,peer);
					found++;
				}


				//pq->enqueue(pq,level+1,createItemNeighborhoodSearch(level+1, idRequester, peer));
			//}//end if !visited
		//}
	}
	//}

	/*
			item =pq->dequeue(pq);
			if (item){
				level = item->level;
				peer = item->peer;
				idRequester = item->idRequester;
				free(item);
			}else{
				deadend = 1;
			}

		}else{
			found = 1;
			if (idRequester>=0){
				topo->updateHitsStats(topo,idRequester,1);
				peer->updateHitsMapQuery(peer,idSource, level);
			}
		}*/

	//}while( (!found) && (!deadend) && (level<=data_neighborhood->maxlevel) );

	/*	item =pq->dequeue(pq);
	while(item){
		free(item);
		item =pq->dequeue(pq);
	}
	pq->ufree(pq);*/

	//visited->ufree(visited);
	/*
	if (!found)
		peer = NULL;*/

	return listPeers;
}

static void ufreeNeighborhoodSearch(TSearch *search){
	TDataSearch *data = search->data;
	TDataNeighborhoodSearch *data_neighborhood = data->data_policy;

	free(data_neighborhood);
	free(data);
	free(search);
}

TSearch *createNeighborhoodSearch(char *entry){
	TSearch *search = malloc(sizeof(TSearch));
	TDataSearch *data = malloc(sizeof(TDataSearch));
	TDataNeighborhoodSearch *data_neighborhood = malloc(sizeof(TDataNeighborhoodSearch));

    TParameters *lp = createParameters(entry, PARAMETERS_SEPARATOR);

    lp->iterator(lp);

	data_neighborhood->maxlevel = atoi(lp->next(lp));

	data->data_policy = data_neighborhood;

	search->data = data;
	search->run = runNeighborhoodSearch;
	search->ufree = ufreeNeighborhoodSearch;

	lp->dispose(lp);

	return search;
}




