#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "system.h"
#include "randomic.h"
#include "object.h"
#include "cache.h"
#include "dictionary.h"
#include "internals.h"

#define MAX(x,y) (x>y?x:y)
#define MIN(x,y) (x<y?x:y)

static short insertCache(TCache *cache, void *object, void* systemData);
static short updateCache(TCache *cache, void *object,void* systemData);
static short isCacheableCache(TCache *cache, void *object, void* systemData);
static void* firstKCache(TCache* cache, int K);
static short hasCache(TCache *cache, void *object);
static short disposeCache(TCache *cache);
static void showCache(TCache *cache);
//static void removeRepCache(TCache *cache);

static TAvailabilityCache getAvailabilityCache(TCache* cache);
static TSizeCache getSizeCache(TCache* cache);
static TStatsCache* getStatsCache(TCache* cache);
static void showStatsCache(TCache* cache);
static void* getDisposedObjectsCache(TCache* cache);
static void* getObjectsCache(TCache* cache);
static TOccupancyCache getOccupancyCache(TCache *cache);
static void addAvailabilityCache(TCache* cache, TAvailabilityCache amount);
static unsigned int getNumberOfStoredObjectCache(TCache* cache);


typedef struct _data_cache TDataCache;
struct _data_cache{
	TListObject *objects; // cached objects
	TListObject *disposed; // disposed objects by OMPolicyReplacement
	TSizeCache size; // in seconds)
	TAvailabilityCache availability; // in seconds
	TStatsCache *stats;
	void *policy;
};

static TDataCache *initDataCache(TSizeCache size, void *policy ){
	TDataCache *data = malloc(sizeof(TDataCache));

	data->objects = createListObject();
	data->disposed = createListObject();
	data->size = size;
	data->availability = size;
	data->stats = createStatsCache();
	data->policy = policy;

	return data;
}

TCache *createCache(TSizeCache size, TOMPolicy *policy ){
	TCache *cache = (TCache *)malloc(sizeof(TCache));

	cache->data = initDataCache(size, policy);

	cache->firstK=firstKCache;
	cache->insert=insertCache;
	cache->update=updateCache;
	cache->isCacheable=isCacheableCache;

	//sets and gets
	cache->show = showCache;
	//cache->removeRep = removeRepCache;
	cache->dispose = disposeCache;
	cache->has = hasCache;
	cache->getAvailability=getAvailabilityCache;
	cache->getSize=getSizeCache;
	cache->getStats = getStatsCache;
	cache->getEvictedObjects=getDisposedObjectsCache;
	cache->getObjects=getObjectsCache;
	cache->addAvailability=addAvailabilityCache;
	cache->getOccupancy = getOccupancyCache;
	cache->showStats = showStatsCache;
	cache->getNumberOfStoredObject = getNumberOfStoredObjectCache;

	return cache;

}

static short insertCache(TCache *cache, void *object, void* systemData){
	short status;
	TDataCache *data = cache->data;
	TGeneralPolicy *policy = data->policy;

	TListObject *disposed = data->disposed;
	disposed->cleanup(disposed);

	status = policy->OM->Replace(systemData, cache, object);

	// if (status)
		//    printf("Well done!\n");
	//else
	//    printf("Got problems to insert\n");

	return status;
}

static short updateCache(TCache *cache, void *object, void* systemData){
	short status;
	TDataCache *data = cache->data;
	TGeneralPolicy *policy = data->policy;

	status = policy->OM->Update(systemData, cache, object);

	// if (status)
	//    printf("Well done!\n");
	//else
	//    printf("Got problems to insert\n");

	return status;
}

static short isCacheableCache(TCache *cache, void *object, void* systemData){
	short status;
	TDataCache *data = cache->data;
	TGeneralPolicy *policy = data->policy;

	status = policy->OM->Cacheable(systemData, cache, object);

	// if (status)
	//    printf("Well done!\n");
	//else
	//    printf("Got problems to insert\n");

	return status;
}


static void* firstKCache(TCache *cache, int K){
	TListObject* firstK;
	TDataCache *data = cache->data;
	TGeneralPolicy *policy = data->policy;

	firstK = policy->OM->FirstK(cache, K);

	// if (status)
	//    printf("Well done!\n");
	//else
	//    printf("Got problems to insert\n");


	return firstK;
}

static void showCache(TCache *cache){
	TDataCache *data = cache->data;
	TListObject *listObject = data->objects;

	listObject->show(listObject);

	printf("\n");
}

/*static void removeRepCache(TCache *cache){
	TDataCache *data = cache->data;
	TListObject *listObject = data->objects;

	listObject->removeRep(listObject);

	printf("\n");
}*/

//Returns a status that points out whether or not
//objects were disposed from passed cache
static short disposeCache(TCache* cache){
	short status = 0;
	TObject *tail, *disposed;
	TSizeCache sizeCache;
	TDataCache *data = cache->data;

	sizeCache = data->size;

	while(data->availability < sizeCache){
		tail = data->objects->getTail(data->objects);
		data->availability += getLengthObject(tail);

		// updating list of disposed objects
		// client could use it and has to clean up it
		disposed = cloneObject(tail);
		data->disposed->insertHead(data->disposed, disposed);

		data->objects->removeTail(data->objects);
		status = 1;
	}

	free(cache->data);
	free(cache);

	return status;
}


static short hasCache(TCache *cache, void *object){
	TDataCache *data = cache->data;
	short found = data->objects->hasInto(data->objects, object);

	return found;

}

static TOccupancyCache getOccupancyCache(TCache *cache){
	TDataCache *data = cache->data;

	return (data->size - data->availability);
}

static TAvailabilityCache getAvailabilityCache(TCache* cache){
	TDataCache *data = cache->data;

	return data->availability;
}

static TSizeCache getSizeCache(TCache* cache){
	TDataCache *data = cache->data;

	return data->size;
}

static unsigned int getNumberOfStoredObjectCache(TCache* cache){
	TDataCache *data = cache->data;
	TListObject *objs = data->objects;

	return objs->getHolding(objs);
}


static TStatsCache* getStatsCache(TCache* cache){
	TDataCache *data = cache->data;

	return data->stats;
}

static void* getDisposedObjectsCache(TCache* cache){
	TDataCache *data = cache->data;

	return data->disposed;
}

static void* getObjectsCache(TCache* cache){
	TDataCache *data = cache->data;

	return data->objects;
}

static void addAvailabilityCache(TCache* cache, TAvailabilityCache amount){
	TDataCache *data = cache->data;

	data->availability += amount;
}


static void showStatsCache(TCache* cache){
	TDataCache *data = cache->data;
	TStatsCache *stats = data->stats;

	printf("hit: %d\n", stats->getHit( stats ) );
	printf("miss: %d\n", stats->getMiss( stats ) );
	printf("byte hits: %lf\n", stats->getByteHit( stats ) );
	printf("byte miss: %lf\n", stats->getByteMiss( stats ) );
	printf("on behalf hit: %d\n", stats->getCommunityHit( stats ) );
	printf("on behalf byte hit: %lf\n", stats->getByteCommunityHit( stats ) );

}


// GDSP Policy
struct OMGDSPPolicy{
	// dynamics
	TOMReplaceGeneralPolicy Replace; // Object Management Policy Replacement(LRU/Popularity)
	TOMUpdateGeneralPolicy Update; // Object Management Policy Update cache(LRU/Popularity)

	TOMFirstKGeneralPolicy FirstK; // OMP first K
};

struct _data_GDSPPolicy{
	// data
	TTimeScaleOfInterestGDSPPolicy TimeScaleOfInterest;
	TListObject *disposedLog; // Log of disposed object over time. Used by GDSP policy
	TAgingGDSPPolicy L;
};

struct GDSPPolicy{
	// dynamics
	TOMGDSPPolicy *OM;
	TDATAGDSPPolicy *data;
};

TAgingGDSPPolicy getAgingDataGDSPPolicy(TDATAGDSPPolicy *data){
	return data->L;
}

void setAgingDataGDSPPolicy(TDATAGDSPPolicy *data, TAgingGDSPPolicy L){
	data->L = L;
}

void *createGDSPPolicy(void *entry){
	TGDSPPolicy *policy = (TGDSPPolicy *)malloc(sizeof(TGDSPPolicy));

	policy->OM = (TOMGDSPPolicy *) malloc(sizeof(TOMGDSPPolicy));
	policy->data = (TDATAGDSPPolicy *) malloc(sizeof(TDATAGDSPPolicy));

	policy->OM->Replace = replaceGDSPPolicy;
	policy->OM->Update = updateGDSPPolicy;

	policy->OM->FirstK = firstKGDSPPolicy;

	// extract parameters from user's entry limited by semi-color
    TParameters *lp = createParameters(entry, PARAMETERS_SEPARATOR);

    lp->iterator(lp);

	policy->data->TimeScaleOfInterest = (TTimeScaleOfInterestGDSPPolicy)atol(lp->next(lp));
	policy->data->disposedLog = createListObject();
	policy->data->L = 0.0;

    lp->dispose(lp);

	return policy;
}

//Returns a status that points out whether or not
//
void* firstKGDSPPolicy(TCache* cache, int k){
	int i=0;
	TListObject *firstK = createListObject();
	TObject *clone=NULL;
	TObject *object; // = getMinimumCumulativeValueListObject(cache->objects);
	TDataCache *data = cache->data;
	TListObject *listObject = data->objects;

	object = listObject->getMaximumCumulativeValue(listObject);

	while ((object != NULL) && (i < k)){

		clone =  cloneObject(object);

		firstK->insertTail(firstK, clone);

		object = listObject->getNotLessThanCumulativeValue(listObject, object );
		i++;
	}

	return firstK;
}

//Returns a status that points out whether or not
short replaceGDSPPolicy(void* vSysInfo, TCache* cache, void* object){
	TSystemInfo *sysInfo = vSysInfo;
	short status = 0;
	float length;
	TObject *clone, *disposed=NULL, *alreadyDisposed;
	float cost = 1; // set a constant cost for the object
	float cumulativeValue = 0;
	float accessFrequency;
	int holdingObject;
	float boundOnCacheSize, boundOnHoldingObject, holdingOnCache;
	TDataCache *data = cache->data;
	TListObject *listObject = data->objects;

	TTimeScaleOfInterestGDSPPolicy timeScale, currentTime, lastAccess, elapsedTime;

	TGDSPPolicy *policy = (TGDSPPolicy *)data->policy;

	TDATAGDSPPolicy *dataPolicy = policy->data;

	TAgingGDSPPolicy L = dataPolicy->L;

	length = getLengthObject(object);

	if( data->size > length ){
		status = 1;
		// 1% of total of cache size
		boundOnCacheSize = (float)(0.01*(data->size));

		// 20% of total number of videos
		boundOnHoldingObject = 0.2*sysInfo->getLengthVideoCatalog(sysInfo);

		while(data->availability < length){
			// get Least Frequently Used Object
			disposed = listObject->getMinimumCumulativeValue(listObject);

			L = getCumulativeValueObject(disposed);

			data->availability += getLengthObject(disposed);;

			// updating list of disposed objects
			// client could use it and has to clean up it
			clone = cloneObject(disposed);
			data->disposed->insertHead(data->disposed, clone);

			// verify conditions 1 and 2, Popularity information paragraph, page 4
			// Popularity-Aware GreedyDual Size web Proxy caching Algorithms
			//
			holdingObject = dataPolicy->disposedLog->getHolding(dataPolicy->disposedLog);

			// space spend with information about disposed object
			// this is convert from bytes to seconds base on this relation for youtube video: 4-5 min video demands 8-9 MB
			// 20 bytes have been used to hold information about each evicted object
			holdingOnCache = (float)(5.0*60.0*holdingObject*20.0)/(float)(9.0*1000000.0);

			//Has to consider how to deal with full log
			//
			if ( (holdingObject <= boundOnHoldingObject) && (holdingOnCache <= boundOnCacheSize) ){
				clone = cloneObject(disposed);
				dataPolicy->disposedLog->insertHead(dataPolicy->disposedLog, clone);
			}
			listObject->remove(listObject, disposed);

		}

		dataPolicy->L = L;

		cost = 1; // set a constant cost for the object
		cumulativeValue = 0;

		timeScale = dataPolicy->TimeScaleOfInterest;
		currentTime = sysInfo->getTime(sysInfo);

		// verify whether or not the object has been evicted.
		alreadyDisposed = dataPolicy->disposedLog->getObject(dataPolicy->disposedLog, object);

		if (alreadyDisposed != NULL){
			// recover the historical data from log disposed object
			//
			accessFrequency =  getAccessFrequencyObject(alreadyDisposed);
			lastAccess = (TTimeSystem) getLastAccessObject(alreadyDisposed);
			//cumulativeValue = getCumulativeValueObject(alreadyDisposed);

			dataPolicy->disposedLog->remove(dataPolicy->disposedLog, alreadyDisposed);

		}else{

			accessFrequency =  getAccessFrequencyObject(object);
			lastAccess = (TTimeSystem) getLastAccessObject(object);
			//cumulativeValue = getCumulativeValueObject(object);

		}

		elapsedTime = currentTime - lastAccess;

		accessFrequency = accessFrequency * pow(2.0,(double)((double)-1.0*(double)elapsedTime/(double)timeScale)) + 1.0;

		cumulativeValue = L + (accessFrequency*cost)/length;

		setLastAccessObject(object, currentTime);
		setAccessFrequencyObject(object,accessFrequency);
		setCumulativeValueObject(object, cumulativeValue);


		listObject->insertHead(listObject, object);

		data->availability -= length;
	}

	//    printf("Call GDSP\n");

	return status;
}


//Returns a status that points out whether or not
//The GDSP Management Policy run okay or not
// GDSP-1 assumes that the cost (c(p)) of objects is constant
//
short updateGDSPPolicy(void* vSysInfo, TCache* cache, void* object){
	TSystemInfo *sysInfo = vSysInfo;
	short status = 0;
	TObject* stored;
	int  cost;
	float length;
	float accessFrequency, cumulativeValue;
	TTimeSystem lastAccess, currentTime, elapsedTime;
	TTimeScaleOfInterestGDSPPolicy timeScale;
	TDataCache *data = cache->data;
	TListObject *listObject = data->objects;

	stored = listObject->getObject(listObject, object);

	if ( !stored ){
		printf("PANIC: Cache.c: GDSP Error on updating object Local Popularity Object\n");
		exit(0);

	}

	TGDSPPolicy *policy = (TGDSPPolicy *)data->policy;

	TDATAGDSPPolicy *dataPolicy = policy->data;

	TAgingGDSPPolicy L = dataPolicy->L;

	cost = 1; // set a constant cost for the object

	accessFrequency =  getAccessFrequencyObject(stored);
	lastAccess = (TTimeSystem) getLastAccessObject(stored);


	timeScale = dataPolicy->TimeScaleOfInterest;
	currentTime = sysInfo->getTime(sysInfo);

	elapsedTime = currentTime - lastAccess;

	accessFrequency = accessFrequency * pow(2.0,(double)((double)-1.0*(double)elapsedTime/(double)timeScale)) + 1.0;

	length = getLengthObject(stored);
	//cumulativeValue = getCumulativeValueObject(stored);


	cumulativeValue = L + (accessFrequency*cost)/length;

	setLastAccessObject(stored, currentTime);
	setAccessFrequencyObject(stored,accessFrequency);
	setCumulativeValueObject(stored, cumulativeValue);

	status = 1;

	return status;
}

struct OMGDSFPolicy{
	TOMReplaceGeneralPolicy Replace;
	TOMUpdateGeneralPolicy Update;

};

struct _data_GDSFPolicy{
	TAgingGDSFPolicy L;
};

struct GDSFPolicy{
	// dynamics
	TOMGDSFPolicy *OM;
	TDATAGDSFPolicy *data;
};

TAgingGDSFPolicy getAgingDataGDSFPolicy (TDATAGDSFPolicy *data){
	return data->L;
}

void setAgingDataGDSFPolicy(TDATAGDSFPolicy *data, TAgingGDSFPolicy L){
	data->L = L;
}

void *initGDSFPolicy( void *entry ){
	TGDSFPolicy *policy = (TGDSFPolicy *)malloc(sizeof(TGDSFPolicy));

	policy->OM = (TOMGDSFPolicy *) malloc(sizeof(TOMGDSFPolicy));
	policy->data = (TDATAGDSFPolicy *) malloc(sizeof(TDATAGDSFPolicy));

	policy->OM->Replace = replaceGDSFPolicy;
	policy->OM->Update = updateGDSFPolicy;

	policy->data->L = 0.0;

	return policy;
}

//Function GDS to replace object
short replaceGDSFPolicy(void* systemData, TCache* cache, void* object){
	short status = 0;
	int sizeObject;
	TObject *clone, *disposed=NULL;
	float cost = 1.0; // set a constant cost for the object
	float keyValue = 0.0;
	TDataCache *data = cache->data;
	TListObject *listObject = data->objects;

	TGDSFPolicy *policy = (TGDSFPolicy *)data->policy;
	TDATAGDSFPolicy *dataPolicy = policy->data;

	TAgingGDSFPolicy L = getAgingDataGDSFPolicy(dataPolicy);

	sizeObject = getLengthObject(object);

	if( data->size > sizeObject ){
		status = 1.0;
		while(data->availability < sizeObject){
			// get Object that has the minimum cumulative value of the Object stored in cache
			disposed = listObject->getMinimumCumulativeValue(listObject);

			// let valueL minimum cumulative value
			L = getCumulativeValueObject(disposed);
			setAgingDataGDSFPolicy(dataPolicy,L);
			data->availability += getLengthObject(disposed);

			// updating list of disposed objects
			// client could use it and has to clean up it
			clone = cloneObject(disposed);
			data->disposed->insertHead(data->disposed, clone);
			listObject->remove(listObject, disposed);
		}
		addLPopularityObject(object);
		cost = 1.0; // set a constant cost for the object
		keyValue = 0.0;

		keyValue = L + ((float)cost/(float)sizeObject);
		setCumulativeValueObject(object, keyValue);

		listObject->insertHead(listObject, object);

		data->availability -= sizeObject;
	}
	return status;
}


//Function GDSF for update object
short updateGDSFPolicy(void* systemData, TCache* cache, void* object){
	short status = 0;
	TObject* stored;
	int  cost;
	int sizeObject;
	int freqencyAccess;
	float keyValue=0.0;
	TDataCache *data = cache->data;
	TListObject *listObject = data->objects;

	TGDSFPolicy *policy = (TGDSFPolicy *)data->policy;
	TDATAGDSFPolicy *dataPolicy = policy->data;
	TAgingGDSFPolicy L = getAgingDataGDSFPolicy(dataPolicy);

	stored = listObject->getObject(listObject, object);
	if ( !stored ){
		printf("PANIC:  Cache.c: GDSF: Error on updating object Local Popularity Object\n");
		exit(0);
	}
	addLPopularityObject(stored);
	freqencyAccess=getLPopularityObject(stored);
	cost = 1; // set a constant cost for the object

	sizeObject = getLengthObject(stored);
	keyValue = getCumulativeValueObject(stored);
	keyValue = L + ((float)freqencyAccess*(float)cost/(float)sizeObject);

	setCumulativeValueObject(stored, keyValue);

	status = 1;

	return status;
}

//######## End of the GDSF Policy #########


//######## GDS Policy #########

struct OMGDSPolicy{
	TOMReplaceGeneralPolicy Replace;
	TOMUpdateGeneralPolicy Update;

};

struct _data_GDSPolicy{
	TAgingGDSPolicy L;
};

struct GDSPolicy{
	// dynamics
	TOMGDSPolicy *OM;
	TDATAGDSPolicy *data;
};

TAgingGDSPolicy getAgingDataGDSPolicy (TDATAGDSPolicy *data){
	return data->L;
}

void setAgingDataGDSPolicy(TDATAGDSPolicy *data, TAgingGDSPolicy L){
	data->L = L;
}

void *initGDSPolicy(void *entry ){
	TGDSPolicy *policy = (TGDSPolicy *)malloc(sizeof(TGDSPolicy));

	policy->OM = (TOMGDSPolicy *) malloc(sizeof(TOMGDSPolicy));
	policy->data = (TDATAGDSPolicy *) malloc(sizeof(TDATAGDSPolicy));

	policy->OM->Replace = replaceGDSPolicy;
	policy->OM->Update = updateGDSPolicy;

	policy->data->L = 0.0;

	return policy;
}

//Function GDS to replace object
short replaceGDSPolicy(void* systemData, TCache* cache, void* object){
	short status = 0;
	int sizeObject;
	TObject *clone, *disposed=NULL;
	float cost = 1.0; // set a constant cost for the object
	float keyValue = 0.0;
	TDataCache *data = cache->data;
	TListObject *listObject = data->objects;

	TGDSPolicy *policy = (TGDSPolicy *)data->policy;
	TDATAGDSPolicy *dataPolicy = policy->data;

	TAgingGDSPolicy L = getAgingDataGDSPolicy(dataPolicy);

	sizeObject = getLengthObject(object);

	if( data->size > sizeObject ){
		status = 1.0;
		while(data->availability < sizeObject){
			// get Object that has the minimum cumulative value of the Object stored in cache
			disposed = listObject->getMinimumCumulativeValue(listObject);

			// let valueL minimum cumulative value
			L = getCumulativeValueObject(disposed);
			setAgingDataGDSPolicy(dataPolicy,L);
			data->availability += getLengthObject(disposed);

			// updating list of disposed objects
			// client could use it and has to clean up it
			clone = cloneObject(disposed);
			data->disposed->insertHead(data->disposed, clone);
			listObject->remove(listObject, disposed);
		}

		cost = 1.0; // set a constant cost for the object
		keyValue = 0.0;

		keyValue = L + ((float)cost/(float)sizeObject);
		setCumulativeValueObject(object, keyValue);

		listObject->insertHead(listObject, object);

		data->availability -= sizeObject;
	}
	return status;
}


//Function GDS for update object
short updateGDSPolicy(void* systemData, TCache* cache, void* object){
	short status = 0;
	TObject* stored;
	float cost;
	int sizeObject;
	float keyValue=0.0;
	TDataCache *data = cache->data;
	TListObject *listObject = data->objects;

	TGDSPolicy *policy = (TGDSPolicy *)data->policy;
	TDATAGDSPolicy *dataPolicy = policy->data;
	TAgingGDSPolicy L = getAgingDataGDSPolicy(dataPolicy);

	stored = listObject->getObject(listObject, object);
	if ( !stored ){
		printf("PANIC:  Cache.c: GDS: Error on updating object Local Popularity Object\n");
		exit(0);
	}

	cost = 1.0; // set a constant cost for the object

	sizeObject = getLengthObject(stored);
	keyValue = getCumulativeValueObject(stored);
	keyValue = L + (cost/(float)sizeObject);

	setCumulativeValueObject(stored, keyValue);

	status = 1;

	return status;
}




// LRU Policy
typedef struct OMLRUPolicy TOMLRUPolicy;
struct OMLRUPolicy{
	//
	TOMReplaceGeneralPolicy Replace; // Object Management Policy Replacement(LRU/Popularity)
	TOMUpdateGeneralPolicy Update; // Object Management Policy Update cache(LRU/Popularity)
	TOMCacheableGeneralPolicy Cacheable; // Object Management Policy Eligibility

	TOMFirstKGeneralPolicy FirstK; // OMP first K

};

typedef void TDATALRUPolicy;

struct LRUPolicy{
	TOMLRUPolicy *OM;
	TDATALRUPolicy *data;
};


void *createLRUPolicy(void *entry){

	TLRUPolicy *policy = (TLRUPolicy *) malloc(sizeof( TLRUPolicy ) );

	policy->OM = (TOMLRUPolicy *) malloc(sizeof( TOMLRUPolicy ) );

	policy->data = NULL;

	// init dynamics
	policy->OM->Replace = replaceLRUPolicy; // Object Management Policy Replacement(LRU/Popularity)
	policy->OM->Update = updateLRUPolicy; // Object Management Policy Update cache(LRU/Popularity)
	policy->OM->FirstK = firstKLRUPolicy;
	policy->OM->Cacheable = cacheableLRUPolicy;

	return policy;
}


void* firstKLRUPolicy(TCache* cache, int k){
	int i=0;
	TListObject *firstK = createListObject();
	TObject *clone=NULL;
	TObject *object; // = getMinimumCumulativeValueListObject(cache->objects);
	TDataCache *data = cache->data;
	TListObject *listObject = data->objects;

	object = listObject->getNext(listObject, NULL);

	while ((object != NULL) && (i < k)){

		clone =  cloneObject(object);

		firstK->insertTail(firstK, clone);

		object = listObject->getNext(listObject, object );
		i++;
	}

	return firstK;
}

//Returns a status that points out whether or not
short replaceLRUPolicy(void* vSysInfo, TCache* cache, void* object){
	TSystemInfo *sysInfo = vSysInfo;
	short status = 0;
	float length;
	TObject *tail, *disposed;
	TDataCache *data = cache->data;
	TListObject *listObject = data->objects;

	length = getLengthObject(object);

	if( data->size > length ){
		status = 1;
		while(data->availability < length){
			tail = listObject->getTail(listObject);
			data->availability += getLengthObject(tail);
			// updating list of disposed objects
			// client could use it and has to clean up it
			disposed = cloneObject(tail);
			data->disposed->insertHead(data->disposed, disposed);

			listObject->removeTail(listObject);
		}

		setLastAccessObject(object,sysInfo->getTime(sysInfo));

		listObject->insertHead(listObject, object);

		data->availability -= length;
	}

	//    printf("Call LRU\n");

	return status;
}

//Returns a status that points out whether or not
short updateLRUPolicy(void* xSysInfo, TCache* cache, void* object){
	TSystemInfo *sysInfo = xSysInfo;
	short status = 0;
	TDataCache *data = cache->data;
	TListObject *listObject = data->objects;

	if (listObject->setNewHead(listObject, object) ){

		setLastAccessObject(object,sysInfo->getTime(sysInfo));

	}else{
		printf("PANIC:  Cache.c: LRU Error on set new head on List Object\n");
		exit(0);

	}
	status = 1;

	//    printf("Call LRU\n");

	return status;
}

short cacheableLRUPolicy(void* systemData, TCache* cache, void* object){
	short status = 0;
	TDataCache *data = cache->data;
	TListObject *listObject = data->objects;

	TObject *lastObject = listObject->getTail(listObject);

	if (lastObject != NULL){
		if ( getLastAccessObject(lastObject) < getLastAccessObject(object) ){
			status = 1;
		}
	}else{
		status = 1;
	}

	return status;
}

//@ FIFO Policy

typedef struct OMFIFOPolicy TOMFIFOPolicy;
struct OMFIFOPolicy{
	//
	TOMReplaceGeneralPolicy Replace; // Object Management Policy Replacement(FIFO/Popularity)
	TOMUpdateGeneralPolicy Update; // Object Management Policy Update cache(FIFO/Popularity)
	TOMCacheableGeneralPolicy Cacheable; // Object Management Policy Eligibility

	TOMFirstKGeneralPolicy FirstK; // OMP first K

};

typedef void TDATAFIFOPolicy;

struct FIFOPolicy{
	TOMFIFOPolicy *OM;
	TDATAFIFOPolicy *data;
};


void *createFIFOPolicy(void *entry){

	TFIFOPolicy *policy = (TFIFOPolicy *) malloc(sizeof( TFIFOPolicy ) );

	policy->OM = (TOMFIFOPolicy *) malloc(sizeof( TOMFIFOPolicy ) );

	policy->data = NULL;

	// init dynamics
	policy->OM->Replace = replaceFIFOPolicy; // Object Management Policy Replacement(FIFO/Popularity)
	policy->OM->Update = updateFIFOPolicy; // Object Management Policy Update cache(FIFO/Popularity)
	//policy->OM->FirstK = firstKFIFOPolicy;
	policy->OM->Cacheable = cacheableFIFOPolicy;

	return policy;
}


void* firstKFIFOPolicy(TCache* cache, int k){
	int i=0;
	TListObject *firstK = createListObject();
	TObject *clone=NULL;
	TObject *object; // = getMinimumCumulativeValueListObject(cache->objects);
	TDataCache *data = cache->data;
	TListObject *listObject = data->objects;

	object = listObject->getNext(listObject, NULL);

	while ((object != NULL) && (i < k)){

		clone =  cloneObject(object);

		firstK->insertTail(firstK, clone);

		object = listObject->getNext(listObject, object );
		i++;
	}

	return firstK;
}

//Returns a status that points out whether or not
short replaceFIFOPolicy(void* vSysInfo, TCache* cache, void* object){
	TSystemInfo *sysInfo = vSysInfo;
	short status = 0;
	float length;
	TObject *head, *disposed;
	TDataCache *data = cache->data;
	TListObject *listObject = data->objects;

	length = getLengthObject(object);

	if( data->size > length ){
		status = 1;
		while(data->availability < length){
			head = listObject->getHead(listObject);
			data->availability += getLengthObject(head);
			// updating list of disposed objects
			// client could use it and has to clean up it
			disposed = cloneObject(head);
			data->disposed->insertHead(data->disposed, disposed);

			listObject->removeHead(listObject);
		}

		//setLastAccessObject(object,sysInfo->getTime(sysInfo));

		listObject->insertTail(listObject, object);

		data->availability -= length;
	}

	//    printf("Call FIFO\n");

	return status;
}

//Returns a status that points out whether or not
short updateFIFOPolicy(void* xSysInfo, TCache* cache, void* object){

	short status = 1;


	return status;
}

short cacheableFIFOPolicy(void* systemData, TCache* cache, void* object){
	short status = 0;
	float length;
	TDataCache *data = cache->data;
	TListObject *listObject = data->objects;


	length = getLengthObject(object);

	if( data->size > length ){

		status = 1;

	}else{
		status = 0;
	}

	return status;
}

//END FIFO Policy


//@ LRU for Replicated
// LRURep Policy
typedef struct OMLRURepPolicy TOMLRURepPolicy;
struct OMLRURepPolicy{
	//
	TOMReplaceGeneralPolicy Replace; // Object Management Policy Replacement(LRURepRep/Popularity)
	TOMUpdateGeneralPolicy Update; // Object Management Policy Update cache(LRURep/Popularity)
	TOMCacheableGeneralPolicy Cacheable; // Object Management Policy Eligibility

	TOMFirstKGeneralPolicy FirstK; // OMP first K

};

typedef void TDATALRURepPolicy;

struct LRURepPolicy{
	TOMLRURepPolicy *OM;
	TDATALRURepPolicy *data;
};


void *createLRURepPolicy(void *entry){

	TLRURepPolicy *policy = (TLRURepPolicy *) malloc(sizeof( TLRURepPolicy ) );

	policy->OM = (TOMLRURepPolicy *) malloc(sizeof( TOMLRURepPolicy ) );

	policy->data = NULL;

	// init dynamics
	policy->OM->Replace = replaceLRURepPolicy; // Object Management Policy Replacement(LRURep/Popularity)
	policy->OM->Update = updateLRURepPolicy; // Object Management Policy Update cache(LRURep/Popularity)
	policy->OM->FirstK = firstKLRURepPolicy;
	policy->OM->Cacheable = cacheableLRURepPolicy;//Retorna 1 em qq condicao !

	return policy;
}


void* firstKLRURepPolicy(TCache* cache, int k){ //@ será utilizada ?
	int i=0;
	TListObject *firstK = createListObject();
	TObject *clone=NULL;
	TObject *object; // = getMinimumCumulativeValueListObject(cache->objects);
	TDataCache *data = cache->data;
	TListObject *listObject = data->objects;

	object = listObject->getNext(listObject, NULL);

	while ((object != NULL) && (i < k)){

		clone =  cloneObject(object);

		firstK->insertTail(firstK, clone);

		object = listObject->getNext(listObject, object );
		i++;
	}

	return firstK;
}

//Returns a status that points out whether or not
short replaceLRURepPolicy(void* vSysInfo, TCache* cache, void* object){ //@ LRU Replicated atualizada para nao excluir calda replicada
	TSystemInfo *sysInfo = vSysInfo;
	short status = 0;
	float length;
	TObject *tail, *disposed,*replicated;
	TDataCache *data = cache->data;
	//TDataObject *isReplicated; //@ replicated object ?
	TListObject *listObject = data->objects;

	length = getLengthObject(object);

	if( data->size > length ){
		status = 1;
		while(data->availability < length){
			tail = listObject->getTail(listObject);

			//@ Replace LRU Replicated


			if (isReplicatedObject(tail) == 1){ //@ move objeto para topo, caso replicado

				replicated = cloneObject(tail);
				//data->disposed->insertHead(data->disposed, replicated);
				listObject->removeTail(listObject);
				listObject->insertHead(listObject, replicated);

			}else{

			data->availability += getLengthObject(tail);
			// updating list of disposed objects
			// client could use it and has to clean up it
			disposed = cloneObject(tail);
			data->disposed->insertHead(data->disposed, disposed);

			listObject->removeTail(listObject);
			}
		}

		setLastAccessObject(object,sysInfo->getTime(sysInfo));

		listObject->insertHead(listObject, object);

		data->availability -= length;
	}

	//    printf("Call LRURep\n");

	return status;
}

//Returns a status that points out whether or not
short updateLRURepPolicy(void* xSysInfo, TCache* cache, void* object){ //@ alterar?
	TSystemInfo *sysInfo = xSysInfo;
	short status = 0;
	TDataCache *data = cache->data;
	TListObject *listObject = data->objects;

	if (listObject->setNewHead(listObject, object) ){

		setLastAccessObject(object,sysInfo->getTime(sysInfo));

	}else{
		printf("PANIC:  Cache.c: LRURep Error on set new head on List Object\n");
		exit(0);

	}
	status = 1;

	//    printf("Call LRURep\n");

	return status;
}

short cacheableLRURepPolicy(void* systemData, TCache* cache, void* object){//@ status recebe 1 em qq condicao
	short status = 0;
	TDataCache *data = cache->data;
	TListObject *listObject = data->objects;

	TObject *lastObject = listObject->getTail(listObject);

	if (lastObject != NULL){
		if ( getLastAccessObject(lastObject) < getLastAccessObject(object) ){
			status = 1;
		}
	}else{
		status = 1;
	}

	return status;
}
//@ End LRURep

//Popularity
typedef struct OMPOPULARITYPolicy TOMPOPULARITYPolicy;
struct OMPOPULARITYPolicy{
	TOMReplaceGeneralPolicy Replace;
	TOMUpdateGeneralPolicy Update;
	TOMCacheableGeneralPolicy Cacheable;

	TOMFirstKGeneralPolicy FirstK;
};

//Popularity
typedef struct _data_POPULARITYPolicy TDATAPOPULARITYPolicy;
struct _data_POPULARITYPolicy{
	TThresholdPOPULARITYPolicy threshold;
};

//Popularity
struct POPULARITYPolicy{
	TOMPOPULARITYPolicy *OM;
	TDATAPOPULARITYPolicy *data;
};

void *initPOPULARITYPolicy(void *entry){
	TPOPULARITYPolicy *policy = (TPOPULARITYPolicy *) malloc(sizeof( TPOPULARITYPolicy ));
	TThresholdPOPULARITYPolicy threshold;

    // extract parameters from user's entry limited by semi-color
    TParameters *lp = createParameters(entry, PARAMETERS_SEPARATOR);

    lp->iterator(lp);

    threshold = atol(lp->next(lp));

	policy->OM = (TOMPOPULARITYPolicy *) malloc(sizeof( TOMPOPULARITYPolicy ));
	policy->data = (TDATAPOPULARITYPolicy *) malloc(sizeof( TDATAPOPULARITYPolicy ));

	policy->OM->Replace = replacePOPULARITYPolicy;
	policy->OM->Update = updatePOPULARITYPolicy;
	policy->OM->Cacheable = cacheablePOPULARITYPolicy;

	policy->OM->FirstK = firstKPOPULARITYPolicy;

	policy->data->threshold = threshold;

	lp->dispose(lp);

	return policy;
}

void* firstKPOPULARITYPolicy(TCache* cache, int k){
	int i=0;
	TListObject *firstK = createListObject();
	TObject *clone=NULL;
	TObject *object;
	TDataCache *data = cache->data;
	TListObject *listObject = data->objects;


	object =listObject->getMaximumGPopularity(listObject);

	while ((object != NULL) && (i < k)){

		clone =  cloneObject(object);

		firstK->insertTail(firstK, clone);

		object = listObject->getNotLessThanGPopularity(listObject, object);

		i++;
	}

	return firstK;
}


short replacePOPULARITYPolicy(void* systemData, TCache* cache, void* object){
	short status = 0;
	int gPopularity;
	float length;
	TObject *tail, *disposed;
	TDataCache *data = cache->data;
	TListObject *listObject = data->objects;

	length = getLengthObject(object);
	gPopularity = getGPopularityObject(object);

	TPOPULARITYPolicy *policy = (TPOPULARITYPolicy *)data->policy;

	TDATAPOPULARITYPolicy *dataPolicy = (TDATAPOPULARITYPolicy *)policy->data;

	if( (data->size > length ) && (gPopularity >= dataPolicy->threshold) ){
		status = 1;
		while(data->availability < length){
			tail = listObject->getTail(listObject);
			data->availability += getLengthObject(tail);
			// updating list of disposed objects
			// client could use it and has to clean up it
			disposed = cloneObject(tail);
			data->disposed->insertHead(data->disposed, disposed);

			listObject->removeTail(listObject);
		}

		listObject->insertHead(listObject, object);

		data->availability -= length;
	}

	//    printf("Call POPULARITY\n");

	return status;
}

/*//remove objeto com campo replicado==1
short removeReplicate(TCache* cache){

	TDataCache *data = cache->data;
	TListObject *listObject = data->objects;
			listObject->removeRep(listObject);
	return 1;



}*/

short updatePOPULARITYPolicy(void* systemData, TCache* cache, void* object){
	short status = 1;

	return status;
}

short cacheablePOPULARITYPolicy(void* systemData, TCache* cache, void* object){
	short status = 0;
	TDataCache *data = cache->data;
	TListObject *listObject = data->objects;

	TObject *lastObject = listObject->getMinimumGPopularity(listObject);

	if (lastObject != NULL){
		if ( getGPopularityObject(lastObject) < getGPopularityObject(object) ){
			status = 1;
		}
	}else{
		status = 1;
	}

	return status;
}

// LFU POLICY related data, definitions and functions
typedef struct OMLFUPolicy TOMLFUPolicy;
struct OMLFUPolicy{
	// dynamics
	TOMReplaceGeneralPolicy Replace; // Object Management Policy Replacement(LRU/Popularity)
	TOMUpdateGeneralPolicy Update; // Object Management Policy Update cache(LRU/Popularity)
	TOMCacheableGeneralPolicy Cacheable;

	TOMFirstKGeneralPolicy FirstK;
};

typedef void TDATALFUPolicy;

// LFU POLICY related data, definitions and functions
struct LFUPolicy{
	// dynamics
	TOMLFUPolicy *OM; // Object Management Policy Replacement(LRU/Popularity)
	TDATALFUPolicy *data;

};

void *createLFUPolicy(void *entry){
	TLFUPolicy *policy = (TLFUPolicy *) malloc(sizeof(TLFUPolicy));

	policy->OM = (TOMLFUPolicy *) malloc(sizeof(TOMLFUPolicy));
	policy->data = NULL;

	policy->OM->Replace = replaceLFUPolicy;
	policy->OM->Update = updateLFUPolicy;
	policy->OM->Cacheable = cacheableLFUPolicy;

	policy->OM->FirstK = firstKLFUPolicy;


	return policy;
}

void* firstKLFUPolicy(TCache* cache, int k){
	int i=0;
	TListObject *firstK = createListObject();
	TObject *clone=NULL;
	TObject *object;
	TDataCache *data = cache->data;
	TListObject *listObject = data->objects;

	object = listObject->getMFU(listObject);

	while ((object != NULL) && (i < k)){

		clone =  cloneObject(object);

		firstK->insertTail(firstK, clone);

		object = listObject->getNotLessThanMFU(listObject, object );
		i++;
	}

	return firstK;
}


//Returns a status that points out whether or not
short replaceLFUPolicy(void* systemData, TCache* cache, void* object){
	short status = 0;
	float length;
	TObject *lfu, *disposed;
	TDataCache *data = cache->data;
	TListObject *listObject = data->objects;

	length = getLengthObject(object);

	if( data->size > length ){
		status = 1;
		while(data->availability < length){

			// get Least Frequently Used Object
			lfu = listObject->getLFU(listObject);
			data->availability += getLengthObject(lfu);

			// updating list of disposed objects
			// client could use it and has to clean up it
			disposed = cloneObject(lfu);
			data->disposed->insertHead(data->disposed, disposed);

			listObject->remove(listObject, lfu);
		}

		setLPopularityObject(object,1);
		listObject->insertHead(listObject, object);

		data->availability -= length;
	}

	//    printf("Call LRU\n");

	return status;
}

//Returns a status that points out whether or not
//The LFU Management Policy run okay or not
short updateLFUPolicy(void* systemData, TCache* cache, void* object){
	short status = 0;
	TObject* stored;
	TDataCache *data = cache->data;
	TListObject *listObject = data->objects;

	stored = listObject->getObject(listObject, object);

	if ( !stored ){
		printf("PANIC:  Cache.c: LFU Error on updating object Local Popularity Object\n");
		exit(0);

	}

	addLPopularityObject(stored);

	status = 1;

	//    printf("Call LRU\n");

	return status;
}

//Returns a status that points out whether or not
short cacheableLFUPolicy(void* systemData, TCache* cache, void* object){
	short status = 0;
	TObject *lfu;
	TDataCache *data = cache->data;
	TListObject *listObject = data->objects;

	lfu = listObject->getLFU(listObject);
	if (lfu != NULL){
		if (getLPopularityObject(lfu) < getLPopularityObject(object)){
			status = 1;
		}
	}else{
		status = 1;
	}

	return status;
}


typedef struct OMPARTIALPolicy TOMPARTIALPolicy;
struct OMPARTIALPolicy{
	// dynamics
	TOMReplaceGeneralPolicy Replace; // Object Management Policy Replacement(LRU/Popularity)
	TOMUpdateGeneralPolicy Update; // Object Management Policy Update cache(LRU/Popularity)

};

typedef struct _data_PARTIALPolicy TDATAPARTIALPolicy;
struct _data_PARTIALPolicy{
	float minimumSegmentFaction;
};

struct PARTIALPolicy{
	TOMPARTIALPolicy *OM; // Object Management Policy Replacement(LRU/Popularity)
	TDATAPARTIALPolicy *data; // Object Management Policy Update cache(LRU/Popularity)

};

void *initPARTIALPolicy( void *entry ){
	TPARTIALPolicy *policy = (TPARTIALPolicy *) malloc(sizeof(TPARTIALPolicy));

    // extract parameters from user's entry limited by semi-color
    TParameters *lp = createParameters(entry, PARAMETERS_SEPARATOR);

    lp->iterator(lp);

	policy->OM = (TOMPARTIALPolicy *) malloc(sizeof(TOMPARTIALPolicy));
	policy->data = (TDATAPARTIALPolicy *) malloc(sizeof(TDATAPARTIALPolicy));

	policy->OM->Replace = replacePARTIALPolicy;
	policy->OM->Update = updatePARTIALPolicy;

	policy->data->minimumSegmentFaction = atof(lp->next(lp));

    lp->dispose(lp);

	return policy;
}

TListObject* firstKPARTIALPolicy(TCache* cache, int k){
	int i = 0;
	TListObject *firstK = createListObject();
	TObject *clone = NULL;
	TObject *object;
	TDataCache *data = cache->data;
	TListObject *listObject = data->objects;

	object = listObject->getMVO(listObject);

	while ((object != NULL) && (i < k)) {

		clone = cloneObject(object);

		firstK->insertTail(firstK, clone);

		object = listObject->getNotLessThanMVO(listObject, object);
		i++;
	}

	return firstK;
}


short replacePARTIALPolicy(void* systemData, TCache* cache, void* object){
	short status = 0;
	int segmentSize, segmentSizeDemand, lvoStoredLength;
	float length;
	TObject *disposed, *lvo;
	TDataCache *data = cache->data;
	TListObject *listObject = data->objects;

	TPARTIALPolicy *policy = (TPARTIALPolicy *)data->policy;

	TDATAPARTIALPolicy *dataPolicy = policy->data;

	length = getLengthObject(object);

	if (data->size > length){
		// segmentSize has to be integer
		//
		status = 1;

		// need to be reviewed
		// we had to reassure that the minimum object size in 10secs, see
		// InitCatalogObject in this file.
		segmentSize = (int)(length*dataPolicy->minimumSegmentFaction);

		segmentSizeDemand = segmentSize;

		while(data->availability < segmentSize){
			// get Less Valuable Object
			// NULL means that any object has to be considered
			lvo = listObject->getLVO(listObject, NULL);

			lvoStoredLength = getStoredObject(lvo);

			if ( lvoStoredLength <= segmentSizeDemand ){
				segmentSizeDemand -= lvoStoredLength;
				data->availability += lvoStoredLength;

				disposed = cloneObject(lvo);

				data->disposed->insertHead(data->disposed, disposed);

				listObject->remove(listObject, lvo);

			} else {

				data->availability += segmentSizeDemand;
				setStoredObject(lvo, lvoStoredLength-segmentSizeDemand);

			}

		}

		// insert the object
		setStoredObject(object, segmentSize);

		listObject->insertHead(listObject, object);

		data->availability -= segmentSize;
	}

	return status;

}

short updatePARTIALPolicy(void* systemData, TCache* cache, void* object){
	short status = 0;
	TObject *stored, *disposed;
	TObject *mvObject; // must valuable object
	TObject *lvObject; // must valuable object
	int missed, segmentSize, hit, k, numSegments, segmentSizeDemand;
	int lvoStoredLength;
	float length;
	TDataCache *data = cache->data;
	TListObject *listObject = data->objects;

	TPARTIALPolicy *policy = (TPARTIALPolicy *)data->policy;

	TDATAPARTIALPolicy *dataPolicy = policy->data;

	stored = listObject->getObject(listObject, object);

	if (!stored) {
		printf("PANIC:  Cache.c: PARTIAL Error on updating object Value\n");
		exit(0);

	}

	mvObject = listObject->getMVO(listObject);

	length = getLengthObject(stored);

	hit = getStoredObject(stored);

	// need to be reviewed
	// we had to reassure that the minimum object size in 10secs, see
	// InitCatalogObject in this file.
	segmentSize = (int)(length*dataPolicy->minimumSegmentFaction);

	if (segmentSize == 0){
		printf("Error: segment size of object");
		exit(0);
	}

	// (hit / cached size of i) = 1
	// object value += hit / cachedSize of object i
	addNormalizedByteServedObject(stored, 1);


	missed = (length - hit)/segmentSize;

	numSegments = (getNormalizedByteServedObject(stored)/getNormalizedByteServedObject(mvObject)) * listObject->getMeanObjectSize(listObject);

	//
	k = MIN(missed, MAX(1, numSegments));

	if (k == 0){
		segmentSize = (length - hit);
	}else{
		segmentSize = k*segmentSize;
	}

	segmentSizeDemand = segmentSize;

	// make room for k segments
	while(data->availability < segmentSize){

		// get Less Valuable Object
		// excluding the stored one
		lvObject = listObject->getLVO(listObject, stored);

		lvoStoredLength = getStoredObject(lvObject);

		if ( lvoStoredLength <= segmentSizeDemand ){
			segmentSizeDemand -= lvoStoredLength;
			data->availability += lvoStoredLength;

			disposed = cloneObject(lvObject);

			data->disposed->insertHead(data->disposed, disposed);

			listObject->remove(listObject, lvObject);

		} else {

			data->availability += segmentSizeDemand;
			setStoredObject(lvObject, lvoStoredLength-segmentSizeDemand);

		}

	}

	// add k segments to object i
	addStoredObject(stored, segmentSize);

	data->availability -= segmentSize;

	status = 1;

	//    printf("Call LRU\n");

	return status;

}


//######## GDSA Policy #########
struct OMGDSAPolicy{
	TOMReplaceGeneralPolicy Replace;
	TOMUpdateGeneralPolicy Update;
};

struct _data_GDSAPolicy{
	TAgingGDSAPolicy L;
	int maxViews,countHeadShorterTail,countHeadLargerTail;
};

struct GDSAPolicy{
	// dynamics
	TOMGDSAPolicy *OM;
	TDATAGDSAPolicy *data;
};


//Functions get
//get L value
TAgingGDSAPolicy getAgingDataGDSAPolicy (TDATAGDSAPolicy *data){
	return data->L;
}

//get rate that head is shorter than tail
float getHeadShorterTailAccessDataGDSAPolicy (TDATAGDSAPolicy *data){
	return ((float)data->countHeadShorterTail/(data->countHeadShorterTail+data->countHeadLargerTail));
}

//get maximum view
TAgingGDSAPolicy getViewsDataGDSAPolicy (TDATAGDSAPolicy *data){
	return data->maxViews;
}

//set L value
void setAgingDataGDSAPolicy(TDATAGDSAPolicy *data, TAgingGDSAPolicy L){
	data->L = L;
}

//set maximum view
void setMaxViewsDataGDSAPolicy(TDATAGDSAPolicy *data, void *object){
	if(getViewsDataGDSAPolicy(data) < getGPopularityObject(object))
	           data->maxViews = getGPopularityObject(object);
}

//count the times that head is larger than tail
void countHeadLargerTailDataGDSAPolicy(TListObject *listObjects, TDATAGDSAPolicy *data){
	TObject *head = listObjects->getHead(listObjects);
	TObject *tail;
	if ( head != NULL) {
		tail = listObjects->getTail(listObjects);
		if( getGPopularityObject(head) > getGPopularityObject(tail)){
			data->countHeadLargerTail++;
		}else{
			data->countHeadShorterTail++;
		}
	}
}


float calcKeyValueGDSAPolicy(TDATAGDSAPolicy *data, void *object, TAgingGDSAPolicy L,float decision){
	float rateFGlobal,frequencyLocal;
	float sizeObject = getLengthObject(object);
	float cost = 1.0;

	frequencyLocal = (float)getLPopularityObject(object);

	if(frequencyLocal == 1.0){
		if(getHeadShorterTailAccessDataGDSAPolicy(data) >= decision){
			return (L + 1.0*((float)cost/(float)sizeObject));
		}else{
			rateFGlobal=(getGPopularityObject(object)/(float)getViewsDataGDSAPolicy(data));
			return (L + rateFGlobal*((float)cost/(float)sizeObject));
		}
	}else{
		if(getHeadShorterTailAccessDataGDSAPolicy(data) >= decision){
			return (L + frequencyLocal*((float)cost/(float)sizeObject));
		}else{
			rateFGlobal=(getGPopularityObject(object)/(float)getViewsDataGDSAPolicy(data));
			return (L + (rateFGlobal+frequencyLocal)*((float)cost/(float)sizeObject));
		}
	}
}


void *createGDSAPolicy( void *entry ){
	TGDSAPolicy *policy = (TGDSAPolicy *)malloc(sizeof(TGDSAPolicy));

	policy->OM = (TOMGDSAPolicy *) malloc(sizeof(TOMGDSAPolicy));
	policy->data = (TDATAGDSAPolicy *) malloc(sizeof(TDATAGDSAPolicy));

	policy->OM->Replace = replaceGDSAPolicy;
	policy->OM->Update = updateGDSAPolicy;
	policy->data->L = 0.0;
	policy->data->maxViews = 1;
	policy->data->countHeadLargerTail = 0;
	policy->data->countHeadShorterTail = 0;
	return policy;
}

//Function GDSA to replace object
short replaceGDSAPolicy(void* systemData, TCache* cache, void* object){
    short status = 0;
    int sizeObject;
    TObject *clone, *disposed=NULL;
    float keyValue = 0.0;
	TDataCache *data = cache->data;
	TListObject *listObject = data->objects;

    TGDSAPolicy *policy = (TGDSAPolicy *)data->policy;
    TDATAGDSAPolicy *dataPolicy = policy->data;

    //get lenth object
    sizeObject = getLengthObject(object);
    //get L value
    TAgingGDSAPolicy L = getAgingDataGDSAPolicy(dataPolicy);
    //add local access
    addLPopularityObject(object);
    //count if head is larger then tail or if head is shorter then tail
    countHeadLargerTailDataGDSAPolicy(listObject,dataPolicy);
    //set max view value
    setMaxViewsDataGDSAPolicy(dataPolicy,object);
    //pre k value calculation
    keyValue=calcKeyValueGDSAPolicy(dataPolicy,object,0.0,0.5);

    if((data->size > sizeObject) && (keyValue >= L)){
        while(data->availability < sizeObject){
           // get Object that has the minimum cumulative value of the Object stored in cache
           disposed = listObject->getMinimumCumulativeValue(listObject);

           // let valueL minimum cumulative value
           L = getCumulativeValueObject(disposed);
           setAgingDataGDSAPolicy(dataPolicy,L);
           data->availability += getLengthObject(disposed);

           clone = cloneObject(disposed);
    	   data->disposed->insertHead(data->disposed, clone);
           listObject->remove(listObject, disposed);
        }

        //k value calculation
        keyValue = calcKeyValueGDSAPolicy(dataPolicy,object,L,0.5);
        setCumulativeValueObject(object, keyValue);
        listObject->insertOrd(listObject,object,cumulativeValueAsCriteriaObject);
        //insertHeadListObject(cache->objects, object);
        data->availability -= sizeObject;
        status = 1;
    }
    return status;
}

//Function GDSA for update object
short updateGDSAPolicy(void* systemData, TCache* cache, void* object){
    short status = 0;
    TObject* stored;
    float keyValue=0.0;
	TDataCache *data = cache->data;
	TListObject *listObject = data->objects;

    TGDSAPolicy *policy = (TGDSAPolicy *)data->policy;
    TDATAGDSAPolicy *dataPolicy = policy->data;
    TAgingGDSAPolicy L = getAgingDataGDSAPolicy(dataPolicy);

    stored = listObject->getObject(listObject, object);
    if ( !stored ){
		printf("PANIC:  Cache.c: GDSA Error on updating object Local Popularity Object\n");
		exit(0);
	}
    //set max view value
    setMaxViewsDataGDSAPolicy(dataPolicy,stored);
    addLPopularityObject(stored);

    keyValue = calcKeyValueGDSAPolicy(dataPolicy,stored,L,0.5);

    setCumulativeValueObject(stored, keyValue);
    listObject->removeSoft(listObject, stored);
    listObject->insertOrd(listObject,stored,cumulativeValueAsCriteriaObject);
    status = 1;
    return status;
}

//######## End of the GDSA Policy #########

// statistcs related data and functions and types
static THitStatsCache getHitStatsCache(TStatsCache *stats);
static TMissStatsCache getMissStatsCache(TStatsCache *stats);
static TMaxOccupancyStatsCache getMaxOccupancyStatsCache(TStatsCache *stats);
static TCommunityHitStatsCache getCommunityHitStatsCache(TStatsCache *stats);
static TByteCommunityHitStatsCache getByteCommunityHitStatsCache(TStatsCache *stats);
static TByteHitStatsCache getByteHitStatsCache(TStatsCache *stats);
static TByteMissStatsCache getByteMissStatsCache(TStatsCache *stats);

static void setHitStatsCache(TStatsCache *stats, THitStatsCache hit);
static void setMissStatsCache(TStatsCache *stats, TMissStatsCache miss);
static void setMaxOccupancyStatsCache(TStatsCache *stats, TMaxOccupancyStatsCache ocuppancy);
static void setCommunityHitStatsCache(TStatsCache *stats, TCommunityHitStatsCache communityHits);
static void setByteCommunityHitStatsCache(TStatsCache *stats, TByteCommunityHitStatsCache ByteCommunityHit);
static void setByteHitStatsCache(TStatsCache *stats, THitStatsCache hit);
static void setByteMissStatsCache(TStatsCache *stats, TMissStatsCache miss);

static void addHitStatsCache(TStatsCache *stats, THitStatsCache amount);
static void addMissStatsCache(TStatsCache *stats, TMissStatsCache amount);
static void addMaxOccupancyStatsCache(TStatsCache *stats, TMaxOccupancyStatsCache amount);
static void addCommunityHitStatsCache(TStatsCache *stats, TCommunityHitStatsCache amount);
static void addByteCommunityHitStatsCache(TStatsCache *stats, TByteCommunityHitStatsCache amount);
static void addByteHitStatsCache(TStatsCache *stats, TByteHitStatsCache amount);
static void addByteMissStatsCache(TStatsCache *stats, TByteMissStatsCache amount);

typedef struct _data_StatsCache TDataStatsCache;
struct _data_StatsCache{
	THitStatsCache  hit;
	TMissStatsCache miss;
	TMaxOccupancyStatsCache maxOccupancy;
	TCommunityHitStatsCache communityHit;
	TByteCommunityHitStatsCache byteCommunityHit;
	TByteHitStatsCache byteHit;
	TByteMissStatsCache byteMiss;
};


TDataStatsCache *initDataStatsCache(){
	TDataStatsCache *data;

	data = malloc(sizeof(TDataStatsCache));
	data->communityHit = 0; // hits whenInto servicing community
	data->byteCommunityHit = 0; //byte community hits when servicing community
	data->hit = 0; // hits when servicing a peer
	data->miss = 0; // misses when servicing a peer
	data->maxOccupancy = 0; // max Occupancy over time
	data->byteHit = 0; // byte hits when servicing a peer
	data->byteMiss = 0; // byte misses when servicing a peer

	return data;
}
TStatsCache *createStatsCache(){
	TStatsCache *stats;

	stats = (TStatsCache*)malloc(sizeof(TStatsCache));

	stats->data = initDataStatsCache();

	stats->getHit = getHitStatsCache;
	stats->getMiss = getMissStatsCache;
	stats->getMaxOccupancy = getMaxOccupancyStatsCache;
	stats->getCommunityHit = getCommunityHitStatsCache;
	stats->getByteCommunityHit = getByteCommunityHitStatsCache;
	stats->getByteHit = getByteHitStatsCache;
	stats->getByteMiss = getByteMissStatsCache;

	stats->setHit = setHitStatsCache;
	stats->setMiss = setMissStatsCache;
	stats->setMaxOccupancy = setMaxOccupancyStatsCache;
	stats->setCommunityHit = setCommunityHitStatsCache;
	stats->setByteCommunityHit = setByteCommunityHitStatsCache;
	stats->setByteHit = setByteHitStatsCache;
	stats->setByteMiss = setByteMissStatsCache;

	stats->addHit = addHitStatsCache;
	stats->addMiss = addMissStatsCache;
	stats->addMaxOccupancy = addMaxOccupancyStatsCache;
	stats->addCommunityHit = addCommunityHitStatsCache;
	stats->addByteCommunityHit = addByteCommunityHitStatsCache;
	stats->addByteHit = addByteHitStatsCache;
	stats->addByteMiss = addByteMissStatsCache;

	return stats;
}

static THitStatsCache getHitStatsCache(TStatsCache *stats){
	TDataStatsCache *data=stats->data;

	return data->hit;
}

static TMissStatsCache getMissStatsCache(TStatsCache *stats){
	TDataStatsCache *data=stats->data;

	return data->miss;
}

static TMaxOccupancyStatsCache getMaxOccupancyStatsCache(TStatsCache *stats){
	TDataStatsCache *data=stats->data;

	return data->maxOccupancy;
}

static TCommunityHitStatsCache getCommunityHitStatsCache(TStatsCache *stats){
	TDataStatsCache *data=stats->data;

	return data->communityHit;
}

static TByteCommunityHitStatsCache getByteCommunityHitStatsCache(TStatsCache *stats){
	TDataStatsCache *data=stats->data;

	return data->byteCommunityHit;
}

static TByteHitStatsCache getByteHitStatsCache(TStatsCache *stats){
	TDataStatsCache *data=stats->data;

	return data->byteHit;
}

static TByteMissStatsCache getByteMissStatsCache(TStatsCache *stats){
	TDataStatsCache *data=stats->data;

	return data->byteMiss;
}

static void setHitStatsCache(TStatsCache *stats, THitStatsCache hit){
	TDataStatsCache *data=stats->data;

	data->hit = hit;
}

static void setMissStatsCache(TStatsCache *stats, TMissStatsCache miss){
	TDataStatsCache *data=stats->data;

	data->miss = miss;
}

static void setMaxOccupancyStatsCache(TStatsCache *stats, TMaxOccupancyStatsCache ocuppancy){
	TDataStatsCache *data=stats->data;

	data->maxOccupancy = ocuppancy;
}

static void setCommunityHitStatsCache(TStatsCache *stats, TCommunityHitStatsCache communityHits){
	TDataStatsCache *data=stats->data;

	data->communityHit = communityHits;
}

static void setByteCommunityHitStatsCache(TStatsCache *stats, TByteCommunityHitStatsCache ByteCommunityHit){
	TDataStatsCache *data=stats->data;

	data->byteCommunityHit = ByteCommunityHit;
}

static void setByteHitStatsCache(TStatsCache *stats, THitStatsCache hit){
	TDataStatsCache *data=stats->data;

	data->byteHit = hit;
}

static void setByteMissStatsCache(TStatsCache *stats, TMissStatsCache miss){
	TDataStatsCache *data=stats->data;

	data->byteMiss = miss;
}

static void addHitStatsCache(TStatsCache *stats, THitStatsCache amount){
	TDataStatsCache *data=stats->data;

	data->hit += amount;
}

static void addMissStatsCache(TStatsCache *stats, TMissStatsCache amount){
	TDataStatsCache *data=stats->data;

	data->miss += amount;
}

static void addMaxOccupancyStatsCache(TStatsCache *stats, TMaxOccupancyStatsCache amount){
	TDataStatsCache *data=stats->data;

	data->maxOccupancy += amount;
}


static void addCommunityHitStatsCache(TStatsCache *stats, TCommunityHitStatsCache amount){
	TDataStatsCache *data=stats->data;

	data->communityHit += amount;
}

static void addByteCommunityHitStatsCache(TStatsCache *stats, TByteCommunityHitStatsCache amount){
	TDataStatsCache *data=stats->data;

	data->byteCommunityHit += amount;
}

static void addByteHitStatsCache(TStatsCache *stats, TByteHitStatsCache amount){
	TDataStatsCache *data=stats->data;

	data->byteHit += amount;
}

static void addByteMissStatsCache(TStatsCache *stats, TByteMissStatsCache amount){
	TDataStatsCache *data=stats->data;

	data->byteMiss += amount;
}


/*
int main(){
    int i;
    TCache *cache;
    TObject *object;
    char idVideo[400], time[6], colon;
    int views, ratings, min, sec;
    float stars;

 // cache with 2000 seconds And LRU Object MaNaGement policy 

    cache = initCache(200000, LRUCache); 

    while(!feof(stdin)){

        scanf("%s", idVideo);
        scanf("%d%d",&min,&sec);
        scanf("%d", &views);
        scanf("%d", &ratings);
        scanf("%f", &stars);

        object = initObject(idVideo, (min*60+sec), views, 0);
        insertCache(cache, object); 

        printf("%d %d\n", getAvailabilityCache(cache), getSizeCache(cache));

    };

}

 */
