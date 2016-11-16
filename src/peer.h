#include "object.h"
#include "dictionary.h"




typedef unsigned int TUpTimeStatsPeer;
typedef unsigned int TDownTimeStatsPeer;
typedef unsigned int TRequestStatsPeer;

typedef struct statsPeer TStatsPeer;

TStatsPeer *initStatsPeer();
TRequestStatsPeer getRequestStatsPeer(TStatsPeer *stats);
TUpTimeStatsPeer getUpTimeStatsPeer(TStatsPeer *stats);
TDownTimeStatsPeer getDownTimeStatsPeer(TStatsPeer *stats);

void setRequestStatPeer(TStatsPeer *stats, TRequestStatsPeer request);
void setUpTimeStatsPeer(TStatsPeer *stats, TUpTimeStatsPeer upTime);
void setDownTimeStatsPeer(TStatsPeer *stats, TDownTimeStatsPeer downTime);

void addRequestStatsPeer(TStatsPeer *stats, TRequestStatsPeer amount);
void addUpTimeStatsPeer(TStatsPeer *stats, TUpTimeStatsPeer amount);
void addDownTimeStatsPeer(TStatsPeer *stats, TDownTimeStatsPeer amount);

// Peer related definition
//
//
typedef struct peer TPeer;
typedef enum{DOWN_PEER=1, UP_PEER=2, STALL_PEER=4} TStatusPeer;

typedef TStatsPeer* (* TGetOnStatsPeer)(TPeer *peer);
typedef unsigned int (* TGetIdPeer)(TPeer *peer);
typedef short(* TGetStatusPeer)(TPeer *peer);
typedef void *(* TGetDynamicJoinPeer)(TPeer *peer);
typedef void *(* TGetDynamicLeavePeer)(TPeer *peer);
typedef void *(* TGetDynamicRequestPeer)(TPeer *peer);
typedef unsigned int (* TGetRequestTimePeer)(TPeer* peer);
typedef unsigned int (* TGetReplicateTimePeer)(TPeer* peer);
typedef unsigned int (* TGetUpSessionDurationPeer)(TPeer* peer);
typedef unsigned int (* TGetDownSessionDurationPeer)(TPeer* peer);
typedef unsigned int (* TGetStartSessionPeer)(TPeer* peer);


typedef void *(* TGetHCachePeer)(TPeer* peer);
typedef void *(* TGetReplicatePeer)(TPeer* peer);
typedef void *(* TGetDataSourcePeer)(TPeer* peer);
typedef void (* TSetStatusPeer)(TPeer *peer, short status);
typedef void (* TSetDynamicJoinPeer)(TPeer *peer, void *dynamicJoin);
typedef void (* TSetDynamicLeavePeer)(TPeer *peer, void *dynamicLeave);
typedef void (* TSetDynamicRequestPeer)(TPeer *peer, void *dynamicRequest);
typedef void (* TSetHCachePeer)(TPeer* peer, void *hcache);
typedef void (* TSetDataSourcePeer)(TPeer *peer, void *dataSource);
typedef short (* TIsUpPeer)(TPeer* peer);
typedef short (* TIsDownPeer)(TPeer* peer);
typedef short (* THasDownlinkPeer)(TPeer *peer, TObject *video, float prefetchFraction);

typedef void (* TOpenULVideoChannelPeer)(TPeer * peer, unsigned int destId, TObject *video, float prefetchFraction);
typedef void (* TOpenDLVideoChannelPeer)(TPeer * peer, unsigned int destId, TObject *video, float prefetchFraction);
typedef void (* TCloseULVideoChannelPeer)(TPeer * peer, unsigned int destId);
typedef int (* TCloseDLVideoChannelPeer)(TPeer * peer, TObject *video);
typedef TDictionary *(* TGetOpenULVideoChannelsPeer)(TPeer * peer);
typedef TDictionary *(* TGetOpenDLVideoChannelsPeer)(TPeer * peer);
typedef TDictionary *(* TGetOpenDLVideosPeer)(TPeer * peer);
typedef TObject *(* TGetVideoReceivingFromPeer)(TPeer * peer, int serverId);

typedef void (* TSetupJoiningPeer)(TPeer *peer);
typedef void (* TSetStartSessionPeer)(TPeer *peer, float clock);//@ Tipo SetStartSessionPeer


typedef void *(* TGetTopologyManagerPeer)(TPeer *peer);
typedef void (* TSetTopologyManagerPeer)(TPeer *peer, void* tm);
typedef short (* TCanStreamPeer)(TPeer *peer, void *video, unsigned int clientId, float prefetchFraction);

//Canal
typedef void *(* TGetChannelPeer)(TPeer *peer);
typedef void (* TSetChannelPeer)(TPeer *peer, void* tm);
typedef void (* TUpdateTimeForFluctuationChannelPeer)(TPeer *peer);
typedef void *(* TGetCurrentlyViewingPeer)(TPeer *peer);
typedef void (* TSetCurrentlyViewingPeer)(TPeer *peer, void* video);

typedef short (* THasCachedPeer)(TPeer *peer, void* object);
typedef void (* TSetTierPeer)(TPeer *peer, short tier);
typedef short (*TGetTierPeer)(TPeer *peer);
typedef void (*TUpdateCachePeer)(TPeer *peer, void *vObject, void *vSystemData);
typedef short (*TInsertCachePeer)(TPeer *peer, void *vObject, void *vSystemData, int levels);
typedef void *(*TGetEvictedCachePeer)(TPeer *peer);

typedef void *(*TGetProfilePeer)(TPeer *peer);
typedef float (*TRunProfilePolicyPeer)(TPeer *peer, void *profile);
typedef void (*TSetProfilePolicyPeer)(TPeer *peer, void* profilePolicy);

typedef void (*TUpdateRequestsMapQueryPeer)(TPeer *peer, unsigned int idSource, short hops);
typedef void (*TUpdateHitsMapQueryPeer)(TPeer *peer, unsigned int idSource, short hops);
typedef void (*TShowMapQueryPeer)(TPeer *peer);
typedef void (*TShowChannelsInfoPeer)(TPeer* peer);

TPeer* createPeer(unsigned int id, short tier, void *pickDataOnSessionDynamic, void *pickDataOffSessionDynamic, void *pickDataRequest, void *dataSource, void *replicate, void *cache, void *topologyManager, void *channel);

struct peer {
	void *data;

	//public methods
		TGetOnStatsPeer getOnStats;
		TGetIdPeer getId;
		TGetStatusPeer getStatus;
		TGetDynamicJoinPeer getDynamicJoin;
		TGetDynamicLeavePeer getDynamicLeave;
		TGetDynamicRequestPeer getDynamicRequest;
		TGetRequestTimePeer getRequestTime;
		TGetReplicateTimePeer getReplicateTime;
		TGetUpSessionDurationPeer getUpSessionDuration;
		TGetDownSessionDurationPeer getDownSessionDuration;
		TGetStartSessionPeer getStartSession; //@
		TGetHCachePeer getHCache;
		TGetReplicatePeer getReplicate;
		TGetDataSourcePeer getDataSource;
		TGetTopologyManagerPeer getTopologyManager;
		TSetTopologyManagerPeer setTopologyManager;
		//Canal		
		TGetChannelPeer getChannel;
		TSetChannelPeer setChannel;
		TUpdateTimeForFluctuationChannelPeer updateTimeForFluctuation;

		TGetCurrentlyViewingPeer getCurrentlyViewing;
		TSetCurrentlyViewingPeer setCurrentlyViewing;

		TCanStreamPeer canStream;

		TSetStatusPeer setStatus;
		TSetDynamicJoinPeer setDynamicJoin;
		TSetDynamicLeavePeer setDynamicLeave;
		TSetDynamicRequestPeer setDynamicRequest;
		TSetHCachePeer setHCache;//@ modificado para hierarquia
		TSetDataSourcePeer setDataSource;
		TSetupJoiningPeer setupJoining;
		TSetStartSessionPeer setStartSession; //@ set Start Session
		TIsUpPeer isUp;
		TIsDownPeer isDown;
		THasDownlinkPeer hasDownlink;

		TOpenULVideoChannelPeer openULVideoChannel;
		TOpenDLVideoChannelPeer openDLVideoChannel;
		TCloseULVideoChannelPeer closeULVideoChannel;
		TCloseDLVideoChannelPeer closeDLVideoChannel;
		TGetOpenULVideoChannelsPeer getOpenULVideoChannels;
		TGetOpenDLVideoChannelsPeer getOpenDLVideoChannels;
		TGetOpenDLVideosPeer getOpenDLVideos;
		TGetVideoReceivingFromPeer getVideoReceivingFrom;

		THasCachedPeer hasCached;
		TSetTierPeer setTier;
		TGetTierPeer getTier;
		TInsertCachePeer insertCache;
		TUpdateCachePeer updateCache;
		TUpdateCachePeer updateCacheAsServer;
		TGetEvictedCachePeer getEvictedCache;
		TGetProfilePeer getProfile;
		TRunProfilePolicyPeer runProfilePolicy;
		TSetProfilePolicyPeer setProfilePolicy;

		TUpdateHitsMapQueryPeer updateHitsMapQuery;
		TUpdateRequestsMapQueryPeer updateRequestsMapQuery;
		TShowMapQueryPeer showMapQuery;
		TShowChannelsInfoPeer showChannelsInfo;
};

void *createAnderbergContentProfilePeer(void *entry);
void *createJaccardContentProfilePeer(void *entry);
//@
void *createStartSessionProfilePeer(void *entry);


void *createRandomSessionLasting(void *entry);
void *createPlaylistSessionLasting(void *entry);
void *createPartialPlaylistSessionLasting(void *entry);
