/*
 * hierarchy.h
 *
 *  Created on: Sep 7, 2016
 *      Author: kratos
 */

#ifndef HIERARCHY_H_
#define HIERARCHY_H_
//#include "cache.h"


/*

//Object related definitions
//
//Stats related definition
typedef unsigned int THitStatsHCache;
typedef unsigned int TMissStatsHCache;
typedef unsigned int TMaxOccupancyStatsHCache;
typedef unsigned int TCommunityHitStatsHCache;
typedef double TByteCommunityHitStatsHCache;
typedef double TByteHitStatsHCache;
typedef double TByteMissStatsHCache;

//Stats related definition
typedef struct statsHCache TStatsHCache;

TStatsHCache *createStatsHCache();

typedef THitStatsHCache (* TGetHitStatsHCache)(TStatsHCache *stats);
typedef TMissStatsHCache (* TGetMissStatsHCache)(TStatsHCache *stats);
typedef TMaxOccupancyStatsHCache (* TGetMaxOccupancyStatsHCache)(TStatsHCache *stats);
typedef TCommunityHitStatsHCache (* TGetCommunityHitStatsHCache)(TStatsHCache *stats);
typedef TByteCommunityHitStatsHCache (* TGetByteCommunityHitStatsHCache)(TStatsHCache *stats);
typedef TByteHitStatsHCache (* TGetByteHitStatsHCache)(TStatsHCache *stats);
typedef TByteMissStatsHCache (* TGetByteMissStatsHCache)(TStatsHCache *stats);

typedef void (* TSetHitStatsHCache)(TStatsHCache *stats, THitStatsHCache hit);
typedef void (* TSetMissStatsHCache)(TStatsHCache *stats, TMissStatsHCache miss);
typedef void (* TSetMaxOccupancyStatsHCache)(TStatsHCache *stats, TMaxOccupancyStatsHCache ocuppancy);
typedef void (* TSetCommunityHitStatsHCache)(TStatsHCache *stats, TCommunityHitStatsHCache communityHits);
typedef void (* TSetByteCommunityHitStatsHCache)(TStatsHCache *stats, TByteCommunityHitStatsHCache ByteCommunityHit);
typedef void (* TSetByteHitStatsHCache)(TStatsHCache *stats, THitStatsHCache hit);
typedef void (* TSetByteMissStatsHCache)(TStatsHCache *stats, TMissStatsHCache miss);

typedef void (* TAddHitStatsHCache)(TStatsHCache *stats, THitStatsHCache amount);
typedef void (* TAddMissStatsHCache)(TStatsHCache *stats, TMissStatsHCache amount);
typedef void (* TAddMaxOccupancyStatsHCache)(TStatsHCache *stats, TMaxOccupancyStatsHCache amount);
typedef void (* TAddCommunityHitStatsHCache)(TStatsHCache *stats, TCommunityHitStatsHCache amount);
typedef void (* TAddByteCommunityHitStatsHCache)(TStatsHCache *stats, TByteCommunityHitStatsHCache amount);
typedef void (* TAddByteHitStatsHCache)(TStatsHCache *stats, TByteHitStatsHCache amount);
typedef void (* TAddByteMissStatsHCache)(TStatsHCache *stats, TByteMissStatsHCache amount);


struct statsHCache{
	//private data
	void *data;

	//public methods (adds, gets, sets)
	TGetHitStatsHCache getHit;
	TGetMissStatsHCache getMiss;
	TGetMaxOccupancyStatsHCache getMaxOccupancy;
	TGetCommunityHitStatsHCache getCommunityHit;
	TGetByteCommunityHitStatsHCache getByteCommunityHit;
	TGetByteHitStatsHCache getByteHit;
	TGetByteMissStatsHCache getByteMiss;

	TSetHitStatsHCache setHit;
	TSetMissStatsHCache setMiss;
	TSetMaxOccupancyStatsHCache setMaxOccupancy;
	TSetCommunityHitStatsHCache setCommunityHit;
	TSetByteCommunityHitStatsHCache setByteCommunityHit;
	TSetByteHitStatsHCache setByteHit;
	TSetByteMissStatsHCache setByteMiss;

	TAddHitStatsHCache addHit;
	TAddMissStatsHCache addMiss;
	TAddMaxOccupancyStatsHCache addMaxOccupancy;
	TAddCommunityHitStatsHCache addCommunityHit;
	TAddByteCommunityHitStatsHCache addByteCommunityHit;
	TAddByteHitStatsHCache addByteHit;
	TAddByteMissStatsHCache addByteMiss;

};

*/


//HCache related implementation
typedef unsigned int TAvailabilityHCache;
typedef unsigned int TSizeHCache;
typedef unsigned int TOccupancyHCache;
typedef struct hcache THCache;


//
//HCache Related functions
//
//THCache *createHCache(int levels );
THCache *createHCache(int levels);

typedef void* (* TFirstKHCache)(THCache *hc, int k);
typedef short (* TUpdateHCache)(THCache *hc, int levels, void *object, void *SystemData);
typedef short (* TInsertHCache)(THCache *hc, int levels, void *object, void *SystemData);
//typedef short (* TPutCacheInHCache)(THCache *hcache, int levels,int size, void* policy);
typedef void (* TPutCacheInHCache)(THCache *hc, int levels,void* cache);
//void putCacheInHCache(THCache *hc, int levels,TCache* cache);
//static THCache* putCacheInHCache(THCache *hcache, int levels,TCache* cache);


typedef short (* TIsHCacheableHCache)(THCache *hc, void *object, void *SystemData);
typedef short (* THasHCache)(THCache *hc, void *object);
typedef short (* TDisposeHCache)(THCache* hc);
typedef void (* TShowHCache)(THCache* hc);
typedef void* (*TSearchHCache)(THCache* hc, void *vObject, int levelInit, int levelEnd);

//typedef static TObject* searchObjectHCache(THCache* hc, TObject *vObject);
//ypedef void (* TRemoveRepHCache)(THCache* hcache);

//static void removeRepHCache(THCache *hcache);

typedef TAvailabilityHCache (* TGetAvailabilityHCache)(THCache* hc);
typedef TSizeHCache (* TGetSizeHCache)(THCache* hc);
typedef void* (* TGetStatsHCache)(THCache* hc, int levels);//
typedef void (* TShowStatsHCache)(THCache* hc, int levels);
typedef void* (* TGetDisposedObjectsHCache)(THCache* hc, int levels);
typedef void* (* TGetObjectsHCache)(THCache* hc, int levels);
typedef void* (* TGetCache)(THCache* hc, int levels);
typedef int (* TGetLevelsHCache)(THCache* hc);
typedef int (* TGetLevelPrincipalHCache)(THCache* hc);
typedef int (* TGetLevelReplicateHCache)(THCache* hc);



typedef TOccupancyHCache (* TGetOccupancyHCache)(THCache *hc);
typedef void (* TAddAvailabilityHCache)(THCache* hc, TAvailabilityHCache amount);

typedef unsigned int (* TGetNumberOfStoredObjectHCache)(THCache* hc, int levels);

struct hcache{
	//private data
	void *data;

	//public methods
	TFirstKHCache firstK;
	TUpdateHCache update;
	TInsertHCache insert;
	TPutCacheInHCache putCache;
	TIsHCacheableHCache isHCacheable;
	THasHCache has;
	TDisposeHCache dispose;
	TShowHCache show;
	TSearchHCache search;
	//TRemoveRepHCache removeRep;

	//gets and sets
	TGetAvailabilityHCache getAvailability;
	TGetSizeHCache getSize;
	TGetStatsHCache getStats;
	TShowStatsHCache showStats;
	TGetDisposedObjectsHCache getEvictedObjects;
	TGetObjectsHCache getObjects;
	TGetLevelsHCache getLevels;
	TGetLevelPrincipalHCache getLevelPrincipal;
	TGetLevelReplicateHCache getLevelReplicate;
	TGetCache getCache;
	TGetOccupancyHCache getOccupancy;
	TAddAvailabilityHCache addAvailability;

	TGetNumberOfStoredObjectHCache getNumberOfStoredObject;


};


#endif /* HIERARCHY_H_ */
