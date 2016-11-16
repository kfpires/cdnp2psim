/*
 * community.h
 *
 *  Created on: Jan 10, 2013
 *      Author: cesar
 */

#ifndef COMMUNITY_H_
#define COMMUNITY_H_


typedef struct _data_community TDataCommunity;

typedef struct community TCommunity;

typedef unsigned long int (* TOnCacheCommunity)(TCommunity* community);
typedef void (* TResetStatCommunity)(TCommunity* community);
typedef void (* TCollectStatTiersCommunity)(TCommunity* community, unsigned long int timestamp);
typedef void (* TCollectStatCommunity)(TCommunity* community, float *hitRate, float *missRate, float *byteHitRate, float *byteMissRate, unsigned long int *totalRequests, int *peersUp, float *hitRateCom);
typedef void (* TPrintStatCommunity)(TCommunity* community);
typedef int (* THowManyReplicateIntoCommunity)(TCommunity* community, void *object, void *hashTable);
typedef short (* THasIntoCommunity)(TCommunity* community, void *object, void *hashTable, unsigned int *idPeer);
typedef void* (* TGetPeersCommunity)(TCommunity* community, unsigned int idPeer);
typedef int (* TGetSizeCommunity)(TCommunity* community);
typedef short (* TIsAlivePeerCommunity)(TCommunity* community, unsigned int idpeer);
typedef void (* TSetAlivePeerCommunity)(TCommunity *community, unsigned int idPeer);
typedef void (* TUnsetAlivePeerCommunity)(TCommunity *community, unsigned int idPeer);
typedef void* (* TGetAlivePeerCommunity)(TCommunity *community);
typedef int (* TGetNumberOfAlivePeerCommunity)(TCommunity *community);
typedef void (* TDisposeCommunity)(TCommunity* community);
typedef void* (* TSearchingCommunity)(TCommunity* community, void *peer, void *object, unsigned int clientId, int prefetch);
typedef int (* TFluctuationCommunity)(TCommunity* community, void *timeForFluctuation, void *hashTable);

struct community{
	void *data;

	TOnCacheCommunity onCache;
	TResetStatCommunity resetStatistics;
	TCollectStatTiersCommunity collectStatsOnTiers;
	TCollectStatCommunity collectStatistics;
	TPrintStatCommunity printStatistics;
	THowManyReplicateIntoCommunity howManyReplicate;
	THasIntoCommunity has;
	TGetPeersCommunity getPeer;
	TGetSizeCommunity getSize;
	TSetAlivePeerCommunity setAlivePeer;
	TUnsetAlivePeerCommunity unsetAlivePeer;
	TIsAlivePeerCommunity isAlivePeer;
	TGetAlivePeerCommunity getAlivePeer;
	TGetNumberOfAlivePeerCommunity getNumberOfAlivePeer;
	TSearchingCommunity searching;
	TFluctuationCommunity fluctuation;

	TDisposeCommunity dispose;
};

TCommunity* createCommunity(int simTime, char *scenarios);

#endif /* COMMUNITY_H_ */






