
#ifndef CACHE_H_
#define CACHE_H_

//Object related definitions
//
//Stats related definition
typedef unsigned int THitStatsCache;
typedef unsigned int TMissStatsCache;
typedef unsigned int TMaxOccupancyStatsCache;
typedef unsigned int TCommunityHitStatsCache;
typedef double TByteCommunityHitStatsCache;
typedef double TByteHitStatsCache;
typedef double TByteMissStatsCache;

//Stats related definition
typedef struct statsCache TStatsCache;

TStatsCache *createStatsCache();

typedef THitStatsCache (* TGetHitStatsCache)(TStatsCache *stats);
typedef TMissStatsCache (* TGetMissStatsCache)(TStatsCache *stats);
typedef TMaxOccupancyStatsCache (* TGetMaxOccupancyStatsCache)(TStatsCache *stats);
typedef TCommunityHitStatsCache (* TGetCommunityHitStatsCache)(TStatsCache *stats);
typedef TByteCommunityHitStatsCache (* TGetByteCommunityHitStatsCache)(TStatsCache *stats);
typedef TByteHitStatsCache (* TGetByteHitStatsCache)(TStatsCache *stats);
typedef TByteMissStatsCache (* TGetByteMissStatsCache)(TStatsCache *stats);

typedef void (* TSetHitStatsCache)(TStatsCache *stats, THitStatsCache hit);
typedef void (* TSetMissStatsCache)(TStatsCache *stats, TMissStatsCache miss);
typedef void (* TSetMaxOccupancyStatsCache)(TStatsCache *stats, TMaxOccupancyStatsCache ocuppancy);
typedef void (* TSetCommunityHitStatsCache)(TStatsCache *stats, TCommunityHitStatsCache communityHits);
typedef void (* TSetByteCommunityHitStatsCache)(TStatsCache *stats, TByteCommunityHitStatsCache ByteCommunityHit);
typedef void (* TSetByteHitStatsCache)(TStatsCache *stats, THitStatsCache hit);
typedef void (* TSetByteMissStatsCache)(TStatsCache *stats, TMissStatsCache miss);

typedef void (* TAddHitStatsCache)(TStatsCache *stats, THitStatsCache amount);
typedef void (* TAddMissStatsCache)(TStatsCache *stats, TMissStatsCache amount);
typedef void (* TAddMaxOccupancyStatsCache)(TStatsCache *stats, TMaxOccupancyStatsCache amount);
typedef void (* TAddCommunityHitStatsCache)(TStatsCache *stats, TCommunityHitStatsCache amount);
typedef void (* TAddByteCommunityHitStatsCache)(TStatsCache *stats, TByteCommunityHitStatsCache amount);
typedef void (* TAddByteHitStatsCache)(TStatsCache *stats, TByteHitStatsCache amount);
typedef void (* TAddByteMissStatsCache)(TStatsCache *stats, TByteMissStatsCache amount);


struct statsCache{
	//private data
	void *data;

	//public methods (adds, gets, sets)
	TGetHitStatsCache getHit;
	TGetMissStatsCache getMiss;
	TGetMaxOccupancyStatsCache getMaxOccupancy;
	TGetCommunityHitStatsCache getCommunityHit;
	TGetByteCommunityHitStatsCache getByteCommunityHit;
	TGetByteHitStatsCache getByteHit;
	TGetByteMissStatsCache getByteMiss;

	TSetHitStatsCache setHit;
	TSetMissStatsCache setMiss;
	TSetMaxOccupancyStatsCache setMaxOccupancy;
	TSetCommunityHitStatsCache setCommunityHit;
	TSetByteCommunityHitStatsCache setByteCommunityHit;
	TSetByteHitStatsCache setByteHit;
	TSetByteMissStatsCache setByteMiss;

	TAddHitStatsCache addHit;
	TAddMissStatsCache addMiss;
	TAddMaxOccupancyStatsCache addMaxOccupancy;
	TAddCommunityHitStatsCache addCommunityHit;
	TAddByteCommunityHitStatsCache addByteCommunityHit;
	TAddByteHitStatsCache addByteHit;
	TAddByteMissStatsCache addByteMiss;

};


//Cache related implementation
typedef unsigned int TAvailabilityCache;
typedef unsigned int TSizeCache;
typedef unsigned int TOccupancyCache;
typedef struct cache TCache;


//
//Cache Related functions
//
TCache *createCache(TSizeCache size, void *policy );

typedef void* (* TFirstKCache)(TCache *cache, int k);
typedef short (* TUpdateCache)(TCache *cache, void *object, void *SystemData);
typedef short (* TInsertCache)(TCache *cache, void *object, void *SystemData);
typedef short (* TIsCacheableCache)(TCache *cache, void *object, void *SystemData);
typedef short (* THasCache)(TCache *cache, void *object);
typedef short (* TDisposeCache)(TCache* cache);
typedef void (* TShowCache)(TCache* cache);
typedef void (* TRemoveRepCache)(TCache* cache);

//static void removeRepCache(TCache *cache);

typedef TAvailabilityCache (* TGetAvailabilityCache)(TCache* cache);
typedef TSizeCache (* TGetSizeCache)(TCache* cache);
typedef TStatsCache* (* TGetStatsCache)(TCache* cache);
typedef void (* TShowStatsCache)(TCache* cache);
typedef void* (* TGetDisposedObjectsCache)(TCache* cache);
typedef void* (* TGetObjectsCache)(TCache* cache);
typedef TOccupancyCache(* TGetOccupancyCache)(TCache *cache);
typedef void (* TAddAvailabilityCache)(TCache* cache, TAvailabilityCache amount);

typedef unsigned int (* TGetNumberOfStoredObjectCache)(TCache* cache);

struct cache{
	//private data
	void *data;

	//public methods
	TFirstKCache firstK;
	TUpdateCache update;
	TInsertCache insert;
	TIsCacheableCache isCacheable;
	THasCache has;
	TDisposeCache dispose;
	TShowCache show;
	//TRemoveRepCache removeRep;

	//gets and sets
	TGetAvailabilityCache getAvailability;
	TGetSizeCache getSize;
	TGetStatsCache getStats;
	TShowStatsCache showStats;
	TGetDisposedObjectsCache getEvictedObjects;
	TGetObjectsCache getObjects;
	TGetOccupancyCache getOccupancy;
	TAddAvailabilityCache addAvailability;

	TGetNumberOfStoredObjectCache getNumberOfStoredObject;


};


// Policy related declaration
//
typedef void TOMPolicy;

// General Policy stuff declaration
typedef short (* TOMReplaceGeneralPolicy)(void* systemData, TCache* cache, void* object);
typedef short (* TOMUpdateGeneralPolicy)(void* systemData, TCache* cache, void* object);
typedef short (* TOMCacheableGeneralPolicy)(void* systemData, TCache* cache, void* object);
typedef void* (* TOMFirstKGeneralPolicy)(TCache* cache, int K);

//
typedef struct GeneralPolicy TGeneralPolicy;

// Policy related data/function definition
//
typedef struct OMGeneralPolicy TOMGeneralPolicy;

struct OMGeneralPolicy{
	TOMReplaceGeneralPolicy Replace; // Object Management Policy Replacement(LRU/Popularity)
	TOMUpdateGeneralPolicy Update; // Object Management Policy Update cache(LRU/Popularity)
	TOMCacheableGeneralPolicy Cacheable; // Eligibility criteria
	TOMFirstKGeneralPolicy FirstK; // First K object for policy

};

typedef void TDATAGeneralPolicy;

struct GeneralPolicy{
	TOMGeneralPolicy *OM;
	TDATAGeneralPolicy *data;
};


//GDSP
typedef struct GDSPPolicy TGDSPPolicy;
typedef struct OMGDSPPolicy TOMGDSPPolicy;
typedef struct _data_GDSPPolicy TDATAGDSPPolicy;

typedef unsigned long int TTimeScaleOfInterestGDSPPolicy;
typedef float TAgingGDSPPolicy;

TAgingGDSPPolicy getAgingDataGDSPPolicy(TDATAGDSPPolicy *data);
void setAgingDataGDSPPolicy(TDATAGDSPPolicy *data, TAgingGDSPPolicy L);

void *createGDSPPolicy(void *entry);
short replaceGDSPPolicy(void* systemData, TCache* cache, void* object);
short updateGDSPPolicy(void* systemData, TCache* cache, void* object);

// Replicate related OM procedure for GDSP
void* firstKGDSPPolicy(TCache* cache, int K);

//Replicate
//short removeReplicate(TCache* cache);

//GDSF
typedef struct GDSFPolicy TGDSFPolicy;
typedef struct OMGDSFPolicy TOMGDSFPolicy;
typedef struct _data_GDSFPolicy TDATAGDSFPolicy;

typedef float TAgingGDSFPolicy;

TAgingGDSFPolicy getAgingDataGDSFPolicy(TDATAGDSFPolicy *data);
void setAgingDataGDSFPolicy(TDATAGDSFPolicy *data, TAgingGDSFPolicy L);

void *initGDSFPolicy(void *entry);
short replaceGDSFPolicy(void* systemData, TCache* cache, void* object);
short updateGDSFPolicy(void* systemData, TCache* cache, void* object);


//GDS
typedef struct GDSPolicy TGDSPolicy;
typedef struct OMGDSPolicy TOMGDSPolicy;
typedef struct _data_GDSPolicy TDATAGDSPolicy;

typedef float TAgingGDSPolicy;

TAgingGDSPolicy getAgingDataGDSPolicy(TDATAGDSPolicy *data);
void setAgingDataGDSPolicy(TDATAGDSPolicy *data, TAgingGDSPolicy L);

void *initGDSPolicy(void *entry);
short replaceGDSPolicy(void* systemData, TCache* cache, void* object);
short updateGDSPolicy(void* systemData, TCache* cache, void* object);


//LRU
typedef struct LRUPolicy TLRUPolicy;
void *createLRUPolicy(void *entry);
short replaceLRUPolicy(void* systemData, TCache* cache, void* object);
short updateLRUPolicy(void* systemData, TCache* cache, void* object);
short cacheableLRUPolicy(void* systemData, TCache* cache, void* object);

void* firstKLRUPolicy(TCache* cache, int k);

//@FIFO for Replicated
//FIFO
typedef struct FIFOPolicy TFIFOPolicy;
void *createFIFOPolicy(void *entry);
short replaceFIFOPolicy(void* systemData, TCache* cache, void* object);
short updateFIFOPolicy(void* systemData, TCache* cache, void* object);
short cacheableFIFOPolicy(void* systemData, TCache* cache, void* object);

void* firstKFIFOPolicy(TCache* cache, int k);

//@ LRU for Replicated
typedef struct LRURepPolicy TLRURepPolicy;
void *createLRURepPolicy(void *entry);
short replaceLRURepPolicy(void* systemData, TCache* cache, void* object);
short updateLRURepPolicy(void* systemData, TCache* cache, void* object);
short cacheableLRURepPolicy(void* systemData, TCache* cache, void* object);

void* firstKLRURepPolicy(TCache* cache, int k);

//LFU
typedef struct LFUPolicy TLFUPolicy;
void *createLFUPolicy(void *entry);
short replaceLFUPolicy(void* systemData, TCache* cache, void* object);
short updateLFUPolicy(void* systemData, TCache* cache, void* object);
short cacheableLFUPolicy(void* systemData, TCache* cache, void* object);

void* firstKLFUPolicy(TCache* cache, int k);

//POPULARITY
typedef struct POPULARITYPolicy TPOPULARITYPolicy;
typedef unsigned long int TThresholdPOPULARITYPolicy;
void *initPOPULARITYPolicy(void *entry);
short replacePOPULARITYPolicy(void* systemData, TCache* cache, void* object);
short updatePOPULARITYPolicy(void* systemData, TCache* cache, void* object);
short cacheablePOPULARITYPolicy(void* systemData, TCache* cache, void* object);

void* firstKPOPULARITYPolicy(TCache* cache, int k);


//PARTIAL
typedef struct PARTIALPolicy TPARTIALPolicy;
void *initPARTIALPolicy( void *entry );
short replacePARTIALPolicy(void* systemData, TCache* cache, void* object);
short updatePARTIALPolicy(void* systemData, TCache* cache, void* object);


//GDSA
typedef struct GDSAPolicy TGDSAPolicy;
typedef struct OMGDSAPolicy TOMGDSAPolicy;
typedef struct _data_GDSAPolicy TDATAGDSAPolicy;

typedef float TAgingGDSAPolicy;

TAgingGDSAPolicy getAgingDataGDSAPolicy(TDATAGDSAPolicy *data);
void setAgingDataGDSAPolicy(TDATAGDSAPolicy *data, TAgingGDSAPolicy L);

void *createGDSAPolicy(void *entry);
short replaceGDSAPolicy(void* systemData, TCache* cache, void* object);
short updateGDSAPolicy(void* systemData, TCache* cache, void* object);



#endif /* CACHE_H_ */
