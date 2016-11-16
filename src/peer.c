#include "stdio.h"
#include "stdlib.h"
#include "replicate.h"
#include "datasource.h"
#include "randomic.h"
#include "cache.h"
#include "hierarchy.h"
#include "object.h"
#include "system.h"
#include "topology.h"
#include "dictionary.h"
#include "peer.h"
#include "channel.h"
//#include "community.h"


typedef struct profile TProfilePolicyPeer;
typedef float (*TRunProfilePeer)(TPeer *peer, void* profilePolicy);
//typedef void * (*TGetProfilePeer)(TPeer *peer);
struct profile{
	void *data;
	TGetProfilePeer get;
	TRunProfilePeer run;
};

float runJaccardContentProfilePeer(TPeer *peer, void* vRemoteCache){
	THCache *localHCache = peer->getHCache(peer);//@ tratar
	THCache *remoteHCache = vRemoteCache;//@ tratar


	int A = 0,B = 0,C = 0;
	float CoeficienteJaccard = 0;

	int levelPrincipal=localHCache->getLevelPrincipal(localHCache);
	TCache *localCache=localHCache->getCache(localHCache,levelPrincipal);
	TCache *remoteCache=remoteHCache->getCache(remoteHCache,levelPrincipal);

	TObject *video=NULL;
	TIteratorListObject *it = createIteratorListObject(remoteCache->getObjects(remoteCache));
	it->reset(it);

	video = it->current(it);
	while(video != NULL){
		if(localCache->has(localCache,video)){//@ tratar
			A++;
		}else{
			B++;
		}

		it->next(it); video = it->current(it);
	}
	C =  abs(localCache->getNumberOfStoredObject(localCache) - A);//@ Pegar o total da hierarquia ou do nivel principal ?
	CoeficienteJaccard = ((float) A / ((float)A+(float)B+(float)C));

	it->ufree(it);

	return CoeficienteJaccard;
}



void * getJaccardContentProfilePeer(TPeer *peer){

	return  peer->getHCache(peer);
}

void *createJaccardContentProfilePeer(void *entry){
	TProfilePolicyPeer *sf = malloc(sizeof(TProfilePolicyPeer));

	sf->data = entry;
	sf->get = getJaccardContentProfilePeer;
	sf->run = runJaccardContentProfilePeer;

	return sf;
}

//@ Functions Session Topology
float runStartSessionProfilePeer(TPeer *peer, void* vneighbor){
	TPeer *neighbor=vneighbor;

	//Retorna o absoluto da diferença entre os tempos de Inicio de Sessão
	return abs(peer->getStartSession(peer)-neighbor->getStartSession(neighbor));

}

void * getStartSessionProfilePeer(TPeer *peer){

	return  peer;
}

//createSessionContentProfilePeer
void *createStartSessionProfilePeer(void *entry){
	TProfilePolicyPeer *sf = malloc(sizeof(TProfilePolicyPeer));

	sf->data = entry;
	sf->get = getStartSessionProfilePeer;
	sf->run = runStartSessionProfilePeer;

	return sf;
}

float runAnderbergContentProfilePeer(TPeer *peer, void *vRemoteCache){
	THCache *localHCache = peer->getHCache(peer); //@ tratar
	THCache *remoteHCache = vRemoteCache;//@ tratar
	int A = 0,B = 0,C = 0;
	float coeficient = 0;

	int levelPrincipal=localHCache->getLevelPrincipal(localHCache);
	TCache *localCache=localHCache->getCache(localHCache,levelPrincipal);
	TCache *remoteCache=remoteHCache->getCache(remoteHCache,levelPrincipal);

	TObject *video=NULL;
	TIteratorListObject *it = createIteratorListObject(remoteCache->getObjects(remoteCache));
	it->reset(it);

	video = it->current(it);
	while(video != NULL){
		if(localCache->has(localCache,video)){//@ tratar
			A++;
		}else{
			B++;
		}

		it->next(it); video = it->current(it);
	}
	C =  abs(localCache->getNumberOfStoredObject(localCache) - A);//@ tratar
	coeficient = (float)A / (A + (2 * (B+C)));

	it->ufree(it);

	return coeficient;
}


void * getAnderbergContentProfilePeer(TPeer *peer){
	return  peer->getHCache(peer);
}

void *createAnderbergContentProfilePeer(void *entry){
	TProfilePolicyPeer *sf = malloc(sizeof(TProfilePolicyPeer));

	sf->data = entry;
	sf->get = getAnderbergContentProfilePeer;
	sf->run = runAnderbergContentProfilePeer;

	return sf;
}


typedef struct sessionLasting TSessionLasting;
typedef unsigned int (*TPickSessionLasting)(TSessionLasting *sl, void*);
struct sessionLasting{
	void *data;
	TPickSessionLasting pick;
};

unsigned int randomPickSessionLasting(TSessionLasting *sl, void* vPeer){
	TRandomic *rand = sl->data;

	return rand->pick(rand);
}

void *createRandomSessionLasting(void *entry){
	TSessionLasting *sl = malloc(sizeof(TSessionLasting));

	sl->data = entry;
	sl->pick = randomPickSessionLasting;

	return sl;
}



unsigned int partialPlaylistPickSessionLasting(TSessionLasting *sl, void* vPeer){
	TRandomic *randomic = sl->data;
	TPeer *peer = vPeer;
    TDataSource *dataSource = peer->getDataSource(peer);
    char xsize[20];
    int size = dataSource->size(dataSource);

    sprintf(xsize, "%d;", size -1);
    randomic->reset(randomic, xsize);
    int boundedSize = randomic->pick(randomic) + 1;
    int duration = dataSource->firstkduration(dataSource, boundedSize);

	return duration;
}

void *createPartialPlaylistSessionLasting(void *entry){
	TSessionLasting *sl = malloc(sizeof(TSessionLasting));

	sl->data = entry;
	sl->pick = partialPlaylistPickSessionLasting;

	return sl;
}



unsigned int playlistPickSessionLasting(TSessionLasting *sl, void* vPeer){
	//TRandomic *rand = sl->data;
	TPeer *peer = vPeer;
    TDataSource *dataSource = peer->getDataSource(peer);

    int size = dataSource->size(dataSource);
    int duration = dataSource->firstkduration(dataSource, size);

	return duration;
}

void *createPlaylistSessionLasting(void *entry){
	TSessionLasting *sl = malloc(sizeof(TSessionLasting));

	sl->data = entry;
	sl->pick = playlistPickSessionLasting;

	return sl;
}

typedef struct {
	unsigned int idPeer;
	unsigned int requests;
	unsigned int hopRequests;
	unsigned int hits;
	unsigned int hopHits;
} TDataMapQuery;

typedef struct {
	void *data;

} TMapQuery;

TDataMapQuery *initDataMapQuery(unsigned int idPeer){
	TDataMapQuery *data = malloc(sizeof(TDataMapQuery));

	data->idPeer = idPeer;
	data->hits = 0;
	data->hopHits =0;
	data->requests=0;
	data->hopRequests=0;

	return data;
}

static void updateHitsMapQuery(TMapQuery *map, int hops){
	TDataMapQuery *data = map->data;
	data->hopHits += hops;
	data->hits++;
}

static void updateRequestsMapQuery(TMapQuery *map, int hops){
	TDataMapQuery *data = map->data;

	data->hopRequests += hops;
	data->requests++;
}

static void showMapQuery(TMapQuery *map){
	if (map){
		TDataMapQuery *data = map->data;
		printf("%u\t%d\t%d\t%d\t%d\n", data->idPeer, data->requests, data->hopRequests, data->hits, data->hopHits);
	}
}


static TMapQuery *createMapQuery(unsigned int idPeer){
	TMapQuery *map = malloc(sizeof(TMapQuery));

	map->data = initDataMapQuery(idPeer);

	return map;
}

//peer related logic and data
//
typedef struct _data_peer TDataPeer;
struct _data_peer{
	unsigned int id;
	unsigned long int startSession;
	short tier;
    short status;
    TDictionary *Queries;
    //TClassPeer class;
    //TCache *cache;
    THCache *hc;
    TStatsPeer *stats;
    // Controls Dynamic join time
    void *dynamicJoin;
    // controls Dynamic leave time
    void *dynamicLeave;
    // controls requests from peer
    TRandomic *dynamicRequest;
    // controls peer data source
    TDataSource *dataSource;
    // Replicate policy
    TReplicate *replicate;
    //topology manager
    TTopology *topology;
    //! policies for profile estimation
    TProfilePolicyPeer *profilePolicy;

    //Canal
    TChannel *channel;
    TFluctuation *fluctuation;

    TObject *currentlyViewing;

    TDictionary *channelsReceiving;
    TDictionary *videosReceiving;
    TDictionary *videosSending;
};



//@ modificado para hierarquia de cache
TDataPeer *initDataPeer(unsigned int id, short tier, void *dynamicJoin, void *dynamicLeave, void *dynamicRequest, void *dataSource, void *replicate, void *hcache, void *topology, void *channel){
	TDataPeer *data = malloc(sizeof(TDataPeer));

	data->id = id;
	data->status = DOWN_PEER;
	data->tier = tier;
	data->startSession = NULL; //@ startSession

	data->dynamicJoin = dynamicJoin;
	data->dynamicLeave = dynamicLeave;
	data->dynamicRequest = dynamicRequest;

	data->dataSource = dataSource;
	data->hc = hcache; //@ Iniciando com a hierarquia
	data->stats = initStatsPeer();

	data->replicate = replicate;

	data->topology = topology;

	data->Queries = createDictionary();

	//Canal
	data->channel = channel;

	data->currentlyViewing = NULL;

	data->channelsReceiving = createDictionary();
	data->videosReceiving = createDictionary();
	data->videosSending = createDictionary();

	return data;
}

//@ modificado para hierarquia de cache
static void* getEvictedFromCachePeer(TPeer *peer, int levels){
	TDataPeer *data = peer->data;

	THCache *hc = data->hc;//@ modificado para hierarquia de cache
	levels=hc->getLevelPrincipal(hc); //@

	return hc->getEvictedObjects(hc, levels);
}

//@ modificado para hierarquia de cache
static short insertCachePeer(TPeer *peer, void *vObject, void *vSystemData, int levels){
	TObject *video = vObject;
	TSystemInfo *systemData = vSystemData;
	TDataPeer *datapeer = peer->data;
	THCache *hc = datapeer->hc;
	TCache *cache=hc->getCache(hc,levels);
/*
	int levelPrincipal;
	levelPrincipal=hc->getLevelPrincipal(hc); //@
*/

	//Record a miss
	//TStatsCache *statsCache = hc->getStats(hc,levels);
	TStatsCache *statsCache = cache->getStats(cache);

	statsCache->addMiss(statsCache, 1);
	statsCache->addByteMiss(statsCache, getLengthObject(video) );

	//try to insert missed video
	return hc->insert( hc,levels, video, systemData ) ;


}

//@ modificado para hierarquia de cache
static void updateCacheAsServerPeer(TPeer *serverPeer, void *vObject, void *vSystemData){
	TObject *storedVideo, *video = vObject;
	TSystemInfo *systemData = vSystemData;
	TDataPeer *data = serverPeer->data;
	TListObject *listObject;
	THCache *hcacheServerPeer;

	hcacheServerPeer = data->hc;
	int lPrincipal=hcacheServerPeer->getLevelPrincipal(hcacheServerPeer);

	// get stored copy
	listObject = hcacheServerPeer->getObjects(hcacheServerPeer,lPrincipal);
	storedVideo = listObject->getObject(listObject, video);

	//updating peer's stats
	TStatsCache *statsCacheServer = hcacheServerPeer->getStats(hcacheServerPeer,lPrincipal);
	statsCacheServer->addCommunityHit(statsCacheServer, 1);
	statsCacheServer->addByteCommunityHit( statsCacheServer, getStoredObject(storedVideo) );

	statsCacheServer->addByteMiss(statsCacheServer, getLengthObject(storedVideo) - getStoredObject(storedVideo) );

	// update cache
	hcacheServerPeer->update(hcacheServerPeer,lPrincipal, video, systemData);

}

static void updateCachePeer(TPeer *peer, void *vObject, void *vSystemData){
	TObject *storedVideo, *video = vObject;
	TSystemInfo *systemData = vSystemData;
	TDataPeer *data = peer->data;

	// update cache
	THCache *hc = data->hc;
	int lPrincipal=hc->getLevelPrincipal(hc);

	hc->update(hc,lPrincipal, video, systemData);

	// record a hit
	TStatsCache *statsCache = hc->getStats(hc,lPrincipal);
	statsCache->addHit( statsCache , 1);

	// get stored copy
	TListObject *listObject = hc->getObjects(hc,lPrincipal);
	storedVideo = listObject->getObject(listObject, video);
	if (storedVideo == NULL)
		printf("Nào achou o video\n");

	statsCache->addByteHit( statsCache, getStoredObject(storedVideo) );
	statsCache->addByteMiss( statsCache, getLengthObject(storedVideo) - getStoredObject(storedVideo) );

}

static void setupJoiningPeer(TPeer *peer){
	TDataPeer *data = peer->data;
	TDataSource *dataSource = data->dataSource;

	dataSource->reset(dataSource);

}

//@ Set Time Join Peer
static void setStartSessionPeer(TPeer *peer,float clock){
	TDataPeer *data = peer->data;

	data->startSession=clock;

}




static void setTierPeer(TPeer *peer, short tier){
	TDataPeer *data = peer->data;
	data->tier = tier;
}


static short getTierPeer(TPeer *peer){
	TDataPeer *data = peer->data;
	return data->tier;
}

static void setTopologyManagerPeer(TPeer *peer, void *topologyManager){
	TDataPeer *data = peer->data;

	data->topology = topologyManager;
}

static void* getTopologyManagerPeer(TPeer *peer){
	TDataPeer *data = peer->data;

	return data->topology;
}

//Canal
static void setChannelPeer(TPeer *peer, void *channel){
	TDataPeer *data = peer->data;

	data->channel = channel;
}

//Fluctuation
static void updateTimeForFluctuationChannelPeer(TPeer *peer){

	TChannel *channel=peer->getChannel(peer);
	channel->updateRates(channel);
}

//Canal
static void setCurrentlyViewingPeer(TPeer *peer, void *currentlyViewing){
	TDataPeer *data = peer->data;

	data->currentlyViewing = currentlyViewing;
}

static void* getChannelPeer(TPeer *peer){
	TDataPeer *data = peer->data;

	return data->channel;
}

static void* getCurrentlyViewingPeer(TPeer *peer){
	TDataPeer *data = peer->data;

	return data->currentlyViewing;
}

static TStatsPeer* getOnStatsPeer(TPeer *peer){
	TDataPeer *data = peer->data;
	return data->stats;
}

static unsigned int getIdPeer(TPeer *peer){
	TDataPeer *data = peer->data;
    return data->id;
}

static short getStatusPeer(TPeer *peer){
	TDataPeer *data = peer->data;
    return data->status;
}

static void setStatusPeer(TPeer *peer, short status){
	TDataPeer *data = peer->data;
    data->status = status;
}


static void setDynamicJoinPeer(TPeer *peer, void *dynamicJoin){
	TDataPeer *data = peer->data;
	data->dynamicJoin = dynamicJoin;
}

static void *getDynamicJoinPeer(TPeer *peer){
	TDataPeer *data = peer->data;
	return data->dynamicJoin;
}

static void setDynamicLeavePeer(TPeer *peer, void *dynamicLeave){
	TDataPeer *data = peer->data;
	data->dynamicLeave = dynamicLeave;
}

static void *getDynamicLeavePeer(TPeer *peer){
	TDataPeer *data = peer->data;
	return data->dynamicLeave;
}

static void setDynamicRequestPeer(TPeer *peer, void *dynamicRequest){
	TDataPeer *data = peer->data;
	data->dynamicRequest = dynamicRequest;
}

static void *getDynamicRequestPeer(TPeer *peer){
	TDataPeer *data = peer->data;
	return data->dynamicRequest;
}


static unsigned int getRequestTimePeer(TPeer* peer){
	TDataPeer *data = peer->data;
	TRandomic *randomic = data->dynamicRequest;

	return randomic->pick(randomic);

}

static unsigned int getReplicateTimePeer(TPeer* peer){
	TDataPeer *data = peer->data;

	TRandomic *cycle = getCycleReplicate(data->replicate);

	return cycle->pick(cycle);

}

static unsigned int getUpSessionDurationPeer(TPeer* peer){
	TDataPeer *data = peer->data;

	TSessionLasting *sl = data->dynamicJoin;


	return sl->pick(sl, peer);
}

static unsigned int getDownSessionDurationPeer(TPeer* peer){
	TDataPeer *data = peer->data;
	TSessionLasting *sl = data->dynamicLeave;


	return sl->pick(sl, peer);
}


static void *getStartSessionPeer(TPeer* peer){
	TDataPeer *data = peer->data;
	return data->startSession;
}


static void *getHCachePeer(TPeer* peer){//@
	TDataPeer *data = peer->data;
	return data->hc;
}

static void setHCachePeer(TPeer* peer, void *hcache){ //@
	TDataPeer *data = peer->data;
	data->hc = hcache;
}


static void *getReplicatePeer(TPeer* peer){
	TDataPeer *data = peer->data;
	return data->replicate;
}

static void setDataSourcePeer(TPeer *peer, void *dataSource){
	TDataPeer *data = peer->data;
	data->dataSource = dataSource;
}

static void *getDataSourcePeer(TPeer* peer){
	TDataPeer *data = peer->data;
	return data->dataSource;
}

static short isUpPeer(TPeer* peer){
	TDataPeer *data = peer->data;
	return (data->status == UP_PEER);
}

static short isDownPeer(TPeer* peer){
	TDataPeer *data = peer->data;
	return (data->status == DOWN_PEER);
}

static void openULVideoChannelPeer(TPeer * peer, unsigned int destId, TObject *video, float prefetchFraction){
	TDataPeer *data = peer->data;
	TChannel *channel = data->channel;
	TDictionary *d = data->videosSending;
	TKeyDictionary key;

	float bitRate = getBitRateObject(video) * (1.f + prefetchFraction);
	unsigned int *content = malloc(sizeof(unsigned int));;

	*content = destId;
	d->insert(d, destId, content);

	channel->openUL(channel, data->id, destId, bitRate);
}

static void openDLVideoChannelPeer(TPeer *peer, unsigned int destId, TObject *video, float prefetchFraction){
	TDataPeer *data = peer->data;
	TChannel *channel = data->channel;
	TDictionary *dc = data->channelsReceiving;
	TDictionary *dv = data->videosReceiving;
	TIdObject idVideo;
	TKeyDictionary key;

	float bitRate;
	unsigned int *content = malloc(sizeof(unsigned int));;

	bitRate = getBitRateObject(video) * (1.f + prefetchFraction);

	getIdObject(video, idVideo);
	key = dv->keyGenesis(idVideo);
	dv->insert(dv, key, video);

	key = dc->keyGenesis(idVideo);
	*content = destId;

	dc->insert(dc, key, content );

	channel->openDL(channel, data->id, destId, bitRate);
}

static int closeDLVideoChannelPeer(TPeer * peer, TObject *video){
	TDataPeer *data = peer->data;
	TChannel *channel = data->channel;
	TDictionary *dc = data->channelsReceiving;
	TDictionary *dv = data->videosReceiving;
	TIdObject idVideo;
	TKeyDictionary key;
	unsigned int *content;
	unsigned int destId;

	if (data->currentlyViewing == video)
		data->currentlyViewing = NULL;

	getIdObject(video, idVideo);
	key = dv->keyGenesis(idVideo);
	dv->remove(dv,key);

	key = dc->keyGenesis(idVideo);
	content = (unsigned int*)dc->remove(dc,key);

	if (content == NULL)
		return -1;

	destId = *content;

	channel->closeDL(channel, destId);

	free(content);

	return destId;
}

static void closeULVideoChannelPeer(TPeer * peer, unsigned int destId){
	TDataPeer *data = peer->data;
	TChannel *channel = data->channel;
	TDictionary *d = data->videosSending;
	TIdObject idVideo;
	unsigned int *content;

	content = (unsigned int*)d->remove(d,destId);
	free(content);

	channel->closeUL(channel, destId);
}

static TDictionary *getOpenULVideoChannelsPeer(TPeer *peer){
	TDataPeer *data = peer->data;

	return data->videosSending;
}

static TDictionary *getOpenDLVideoChannelsPeer(TPeer *peer){
	TDataPeer *data = peer->data;

	return data->channelsReceiving;
}

static TDictionary *getOpenDLVideosPeer(TPeer *peer){
	TDataPeer *data = peer->data;

	return data->videosReceiving;
}

static TObject* getVideoReceivingFromPeer(TPeer *peer, int serverId){
	TDataPeer *data = peer->data;
	TDictionary *dc = data->channelsReceiving;
	TDictionary *dv = data->videosReceiving;
	TKeyDictionary key;
	TIterator *i = createKeyIteratorDictionary(dc);
	unsigned int *serverCandidate;

	i->reset(i);

	while (i->has(i)) {
		key = (TKeyDictionary)i->current(i);

		serverCandidate = dc->retrieval(dc, key);

		if (*serverCandidate == serverId) {
			return dv->retrieval(dv, key);
		}

		i->next(i);
	}

	return NULL;
}

static short hasDownlinkPeer(TPeer* peer, TObject *video,  float prefetchFraction){
	TDataPeer *data = peer->data;
	TChannel *channel = data->channel;
	float bitRate = getBitRateObject(video) * (1.f + prefetchFraction);

	return channel->hasDownlink(channel, bitRate);
}

static short hasCachedPeer(TPeer* peer, void *object){
	TDataPeer *data = peer->data;
	THCache *hc = data->hc;

	return hc->has(hc, object);
	//return (data->status == DOWN_PEER);
}

static short canStreamPeer(TPeer* peer, void *object, unsigned int clientId, float prefetchFraction){ //@
	TDataPeer *data = peer->data;
	//THCache *hcache = data->hcache;
	TChannel *channel = data->channel;
	TDictionary *videosSending = data->videosSending;
	float bitRate;
	unsigned int *content;

	// Verificar se tem no cache
	if (!peer->hasCached(peer, object))
		return 0;

	//Verificar se cliente já está conectado
	//a esse servidor
	content = videosSending->retrieval(videosSending, clientId);

	if(content)
		return 0;

	fprintf(stderr, "Bit rate1: %f\n", getBitRateObject(object));
	fprintf(stderr, "Prefetch fraction: %f\n", prefetchFraction);
	// Obter a taxa necessária do vídeo
	bitRate = getBitRateObject(object) * (1.f + prefetchFraction);
	fprintf(stderr, "Bit rate2: %f\n", bitRate);
	fprintf(stderr, "Can stream: %d\n", channel->canStream(channel, bitRate));
		
	// 3o) Chamar channel->canStream com essa taxa
	// para verificar se tem taxa de uplink disponívels
	return channel->canStream(channel, bitRate);
}

static void* getProfilePeer(TPeer* peer){
	TDataPeer *data = peer->data;
	TProfilePolicyPeer *pp = data->profilePolicy;

	return pp->get(peer);
}

static float runProfilePolicyPeer(TPeer* peer, void *rProfile){
	TDataPeer *data = peer->data;
	TProfilePolicyPeer *pp = data->profilePolicy;

	return pp->run(peer,rProfile);

}

static void setProfilePolicyPeer(TPeer* peer, void *profile){
	TDataPeer *data = peer->data;

	data->profilePolicy = profile;
}

static void updateRequestsMapQueryPeer(TPeer* peer, unsigned int idRequester, short hops){
	TDataPeer *data = peer->data;

	TDictionary *queries = data->Queries;

	TMapQuery *map = queries->retrieval(queries, idRequester);

	if (!map){
		map = createMapQuery(idRequester);
		queries->insert(queries,idRequester,map);
	}
	updateRequestsMapQuery(map,hops);
}

static void updateHitsMapQueryPeer(TPeer* peer, unsigned int idRequester, short hops){
	TDataPeer *data = peer->data;

	TDictionary *queries = data->Queries;

	TMapQuery *map = queries->retrieval(queries, idRequester);

	if (!map){
		map = createMapQuery(idRequester);
		queries->insert(queries,idRequester,map);
	}
	updateHitsMapQuery(map,hops);
}

static void showMapQueryPeer(TPeer* peer){
	TDataPeer *data = peer->data;

	TDictionary *queries = data->Queries;

	TIterator *it = createSoftIteratorDictionary(queries);

	it->reset(it);
	while(it->has(it)){
		TMapQuery *map = it->current(it);
		printf("p:\t%d\t",data->id);showMapQuery(map);
		it->next(it);
	}

	it->ufree(it);
}

static void showChannelsInfoPeer(TPeer* peer){
	unsigned int peerId;
	TDataPeer *data = peer->data;
	TObject * currentlyViewing = peer->getCurrentlyViewing(peer);
	TIdObject idVideo;

	TKeyDictionary key;
	TDictionary* channelsReceiving = peer->getOpenDLVideoChannels(peer);
	unsigned int* clientId;
	unsigned int* serverId;
	TDictionary* videosSending = peer->getOpenULVideoChannels(peer);
	TIterator *iterator;

	float bitRate;

	peerId = peer->getId(peer);

	if(currentlyViewing != NULL) {
		getIdObject(currentlyViewing, idVideo);
		bitRate = getBitRateObject(currentlyViewing);

		key = channelsReceiving->keyGenesis(idVideo);
		serverId = channelsReceiving->retrieval(channelsReceiving, key);

		if (serverId != NULL)
			printf("Peer %d is viewing %s (%f Kbps) from %d\n", peerId, idVideo, bitRate, *serverId);
		else
			printf("Peer %d is viewing %s (%f Kbps) from cache or CDN\n", peerId, idVideo, bitRate);
	}

	iterator = createIteratorDictionary(videosSending);
	iterator->reset(iterator);

	if (iterator->has(iterator)) {
		printf("Peer %d is sending (total of %f Kbps) to\n", peerId, data->channel->getULRate(data->channel));

		for (; iterator->has(iterator); iterator->next(iterator)) {
			clientId = (unsigned int*)iterator->current(iterator);
			if (clientId != NULL)
				printf("\t%d\n", *clientId);
		}
	}

	iterator = createIteratorDictionary(channelsReceiving);
	iterator->reset(iterator);

	if (iterator->has(iterator)) {
		printf("Peer %d is receiving (total of %f Kbps) from\n", peerId, data->channel->getDLRate(data->channel));

		for (; iterator->has(iterator); iterator->next(iterator)) {
			serverId = (unsigned int*)iterator->current(iterator);
			if (serverId != NULL)
				printf("\t%d\n", *serverId);
		}
	}
}

TPeer* createPeer(unsigned int id,  short tier, void *dynamicJoin, void *dynamicLeave, void *dynamicRequest, void *dataSource, void *replicate, void *hcache, void *topo, void *channel){
    TPeer *p = (TPeer*)malloc(sizeof(TPeer));

    //Canal
    p->data = initDataPeer(id, tier, dynamicJoin, dynamicLeave, dynamicRequest, dataSource, replicate, hcache, topo, channel);

    p->getOnStats = getOnStatsPeer;
    p->getId = getIdPeer;
    p->getStatus = getStatusPeer;
    p->getDynamicJoin = getDynamicJoinPeer;
    p->getDynamicLeave = getDynamicLeavePeer;
    p->getDynamicRequest = getDynamicRequestPeer;
    p->getRequestTime = getRequestTimePeer;
    p->getReplicateTime = getReplicateTimePeer;
    p->getUpSessionDuration = getUpSessionDurationPeer;
    p->getDownSessionDuration = getDownSessionDurationPeer;
    p->getStartSession = getStartSessionPeer;
    p->getHCache = getHCachePeer; //@ Hierarquia
    p->getReplicate = getReplicatePeer;
    p->getDataSource = getDataSourcePeer;
    p->getCurrentlyViewing = getCurrentlyViewingPeer;
    p->setStatus = setStatusPeer;
    p->setDynamicJoin = setDynamicJoinPeer;
    p->setDynamicLeave = setDynamicLeavePeer;
    p->setDynamicRequest = setDynamicRequestPeer;
    p->setHCache = setHCachePeer;
    p->setDataSource = setDataSourcePeer;
    p->setCurrentlyViewing = setCurrentlyViewingPeer;
    p->isUp = isUpPeer;
    p->isDown = isDownPeer;
    p->setupJoining = setupJoiningPeer;
    p->setStartSession = setStartSessionPeer;
    p->hasCached = hasCachedPeer;
    p->hasDownlink = hasDownlinkPeer;

    p->openULVideoChannel = openULVideoChannelPeer;
    p->openDLVideoChannel = openDLVideoChannelPeer;
    p->closeULVideoChannel = closeULVideoChannelPeer;
    p->closeDLVideoChannel = closeDLVideoChannelPeer;
    p->getOpenULVideoChannels = getOpenULVideoChannelsPeer;
    p->getOpenDLVideoChannels = getOpenDLVideoChannelsPeer;
    p->getOpenDLVideos = getOpenDLVideosPeer;
    p->getVideoReceivingFrom = getVideoReceivingFromPeer;

    p->insertCache = insertCachePeer;
    p->updateCache = updateCachePeer;
    p->updateCacheAsServer = updateCacheAsServerPeer;
    p->getEvictedCache = getEvictedFromCachePeer;

    p->getTopologyManager = getTopologyManagerPeer;
    p->setTopologyManager = setTopologyManagerPeer;

    //Canal
    p->getChannel = getChannelPeer;
    p->setChannel = setChannelPeer;
    p->updateTimeForFluctuation = updateTimeForFluctuationChannelPeer;
    p->canStream = canStreamPeer;

    p->setTier = setTierPeer;
    p->getTier = getTierPeer;

    p->runProfilePolicy = runProfilePolicyPeer;
    p->getProfile = getProfilePeer;
    p->setProfilePolicy = setProfilePolicyPeer;

    // deal with MapQuery
    p->updateHitsMapQuery = updateHitsMapQueryPeer;
    p->updateRequestsMapQuery = updateRequestsMapQueryPeer;
    p->showMapQuery = showMapQueryPeer;
    p->showChannelsInfo = showChannelsInfoPeer;

    return p;
}




//Stats related implementation
struct statsPeer{
	TUpTimeStatsPeer upTime;
	TDownTimeStatsPeer downTime;
	TRequestStatsPeer request;
};



TStatsPeer *initStatsPeer(){
	TStatsPeer *stats;

	stats = (TStatsPeer*)malloc(sizeof(TStatsPeer));

	stats->upTime = 0; // how long it stay up
	stats->downTime = 0; // how long it stay down
	stats->request = 0; // number of dispatched request

	return stats;
}

TRequestStatsPeer getRequestStatsPeer(TStatsPeer *stats){
	return stats->request;
}

TUpTimeStatsPeer getUpTimeStatsPeer(TStatsPeer *stats){
	return stats->upTime;
}

TDownTimeStatsPeer getDownTimeStatsPeer(TStatsPeer *stats){
	return stats->downTime;
}


void setRequestStatPeer(TStatsPeer *stats, TRequestStatsPeer request){
	stats->request = request;
}

void setUpTimeStatsPeer(TStatsPeer *stats, TUpTimeStatsPeer upTime){
	stats->upTime = upTime;
}

void setDownTimeStatsPeer(TStatsPeer *stats, TDownTimeStatsPeer downTime){
	stats->downTime = downTime;
}

void addRequestStatsPeer(TStatsPeer *stats, TRequestStatsPeer amount){
	stats->request += amount;
}

void addUpTimeStatsPeer(TStatsPeer *stats, TUpTimeStatsPeer amount){
	stats->upTime += amount;
}

void addDownTimeStatsPeer(TStatsPeer *stats, TDownTimeStatsPeer amount){
	 stats->downTime += amount;
}



/*
int main(){
    int i;
    unsigned short seed[] = {10,11,12};
    unsigned short seed2[] = {120,130,140};
        
    TPickRandomic *pickDataUpDynamic = initPickedUniform(seed,10);
    TPickRandomic *pickDataDownDynamic = initPickedPoisson(seed,15);
    TPickRandomic *pickDataRequest = initPickedUniform(seed2,12);
    TCache *cache = initCache(200000, LRUCache); 
    TPeer *p = initPeer(10, 1, 0, pickDataUpDynamic, pickDataDownDynamic, pickDataRequest, cache );

    for(i=1;i<=100;i++)
        printf("%d %d %d\n", getOffSessionDurationPeer(p), getOnSessionDurationPeer(p), getRequestTimePeer(p));

    return 0;
}

*/
