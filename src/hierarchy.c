#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "system.h"
#include "randomic.h"
#include "object.h"
#include "hierarchy.h"
#include "cache.h"
#include "dictionary.h"
#include "internals.h"

/*
#define MAX(x,y) (x>y?x:y)
#define MIN(x,y) (x<y?x:y)
*/

static short insertHCache(THCache *hc, int levels, void *object, void* systemData); //@ parametro: levels=nivel a ser acessado
void putCacheInHCache(THCache *hc, int levels,TCache* cache);
static short updateHCache(THCache *hc, int levels, void *object,void* systemData);
static short isCacheableHCache(THCache *hc, int levels,void *object, void* systemData);
//static void* firstKHCache(THCache* hc, int levels, int K);
//static short hasHCache(THCache *hc, int levels, void *object);//@ retirar levels
static short hasHCache(THCache *hc, void *object);

static short disposeHCache(THCache *hc, int levels);
static void showHCache(THCache *hc, int levels);
//static void* searchObjectHCache(THCache* hc, void *vObject);
static void* searchObjectHCache(THCache* hc, TObject *vObject);

//static void removeRepHCache(THCache *hc, int levels, int levels);

static TAvailabilityHCache getAvailabilityHCache(THCache* hc, int levels);
static TSizeCache getSizeHCache(THCache* hc, int levels);
//static void* getStatsHCache(THCache* hc, int levels);
static void showStatsHCache(THCache* hc, int levels);
static void* getDisposedObjectsHCache(THCache* hc, int levels);
static void* getObjectsHCache(THCache* hc);
static int getLevelsHCache(THCache* hc);
static int getLevelPrincipalHCache(THCache* hc);
static int getLevelReplicateHCache(THCache* hc);
static void* getCache(THCache *hc, int levels );
static TOccupancyHCache getOccupancyHCache(THCache *hc, int levels);
static void addAvailabilityHCache(THCache* hc, int levels, TAvailabilityHCache amount);
static unsigned int getNumberOfStoredObjectHCache(THCache* hc, int levels);

//@ Primeira
typedef struct _data_hc TDataHCache;

struct _data_hc{
	TCache **hcache;
	int levels;
	int levelPrincipal;
	int levelReplicate;

	TListObject *objects; // hcd objects
	TListObject *disposed; // disposed objects by OMPolicyReplacement
	//TSizeHCache size; // in seconds)
	//TAvailabilityHCache availability; // in seconds
	//TStatsHCache *stats;
	//void *policy;
};

//segunda
static TDataHCache *initDataHCache(int levels){ //@
//static TDataHCache *initDataHCache(int levels, int levelPrincipal){ //@

	TDataHCache *data = malloc(sizeof(TDataHCache));

	data->hcache = malloc(levels * sizeof(TCache*));
	//data->cache = malloc(sizeof(TCache*)*levels);
	data->levels = levels;
	data->levelPrincipal=0;
	data->levelReplicate=1;
	//data->levelPrincipal=levelPrincipal;



		//data->objects = createListObject();
		//data->disposed = createListObject();
		//data->size = size;
		//data->availability = size;
		//data->stats = createStatsHCache();
		//data->policy = policy;



	return data;
}


//THCache *createHCache(int levels, int levelPrincipal, int levelReplicate){
THCache *createHCache(int levels){
	THCache *hc = (THCache *)malloc(sizeof(THCache));


	//hc->data = initDataHCache(levels, levelPrincipal);
	hc->data = initDataHCache(levels);

	// agora atualizar metodos da hierarquia
	//hc->firstK=firstKHCache;
	hc->insert=insertHCache;
	hc->putCache=putCacheInHCache;
	hc->update=updateHCache;
	hc->isHCacheable=isCacheableHCache;

	//sets and gets
	hc->show = showHCache;
	hc->search=searchObjectHCache;
	//hc->removeRep = removeRepHCache;
	hc->dispose = disposeHCache;
	hc->has = hasHCache;
	hc->getAvailability=getAvailabilityHCache;
	hc->getSize=getSizeHCache;
	//hc->getStats = getStatsHCache;
	hc->getEvictedObjects=getDisposedObjectsHCache;
	hc->getObjects=getObjectsHCache;//@modificar para trazer todos os objetos, independente de niveis
	hc->getLevels=getLevelsHCache;
	hc->getLevelPrincipal=getLevelPrincipalHCache;
	hc->getLevelReplicate=getLevelReplicateHCache;
	hc->getCache=getCache;
	hc->addAvailability=addAvailabilityHCache;
	hc->getOccupancy = getOccupancyHCache;
	hc->showStats = showStatsHCache;
	hc->getNumberOfStoredObject = getNumberOfStoredObjectHCache;

	return hc;

}

static short insertHCache(THCache *hc, int levels, void *object, void* systemData){
	short status;
	TDataHCache *data = hc->data;
	TCache *cache=data->hcache[levels];


	status=cache->insert(cache,object,systemData);


	return status;
}

//@ poe a cache na hierarquia
void putCacheInHCache(THCache *hc, int levels,TCache* cache){

	TDataHCache *data = hc->data;

	data->hcache[levels]=cache;


}

static short updateHCache(THCache *hc, int levels, void *object, void* systemData){
	short status;
	TDataHCache *data = hc->data;
	TCache *cache=data->hcache[levels];

	status = cache->update(cache, object,systemData);

	return status;
}

static short isCacheableHCache(THCache *hc, int levels, void *object, void* systemData){
	short status;
	TDataHCache *data = hc->data;
	TCache *cache=data->hcache[levels];

	status = cache->isCacheable(cache, object,systemData);



	return status;
}


static void showHCache(THCache *hc, int levels ){
	TDataHCache *data = hc->data;
	TCache *cache=data->hcache[levels];
	TListObject *listObject = cache->getObjects(cache);

	listObject->show(listObject);

	printf("\n");
}

/*static void removeRepHCache(THCache *hc, int levels, int levels){
	TDataHCache *data = hc->data;
	TListObject *listObject = data->objects;

	listObject->removeRep(listObject);

	printf("\n");
}*/

//Returns a status that points out whether or not
//objects were disposed from passed hc
static short disposeHCache(THCache* hc, int levels ){
	short status = 0;


	TDataHCache *data = hc->data;
	TCache *cache=data->hcache[levels];

		status = cache->dispose(cache);


	return status;
}


static short hasHCache(THCache *hc, void *object){
	TDataHCache *data = hc->data;
	int slevel,elevel;
	 int levels=hc->getLevels(hc);
	TCache *cache;
	short found=0;
//
//	if(levels!=-1){
//		slevel=elevel=levels;
//		cache=data->hcache[slevel];//ERRO tratar...
//		found = cache->has(cache, object);
//
//	}else{
		slevel=0;
		elevel=levels;
		while(slevel<elevel && found==0){
			cache=data->hcache[slevel];
			found = cache->has(cache, object);
			slevel++;
		}
    //} //end else
	return found;
}

static TOccupancyHCache getOccupancyHCache(THCache *hc, int levels){
	TDataHCache *data = hc->data;
	TCache *cache=data->hcache[levels];

		return cache->getOccupancy(cache);
}

static TAvailabilityHCache getAvailabilityHCache(THCache* hc, int levels){
	TDataHCache *data = hc->data;
	TCache *cache=data->hcache[levels];

		return cache->getAvailability(cache);
}

static TSizeCache getSizeHCache(THCache* hc, int levels){
	TDataHCache *data = hc->data;
	TCache *cache=data->hcache[levels];

	return cache->getSize(cache);
}

static unsigned int getNumberOfStoredObjectHCache(THCache* hc, int levels){
	TDataHCache *data = hc->data;
	TCache *cache=data->hcache[levels];

	return cache->getNumberOfStoredObject (cache);
}


/*static void* getStatsHCache(THCache* hc, int levels){
	TDataHCache *data = hc->data;
	TCache *cache=data->hcache[levels];

	return cache->getStats(cache);
}*/

static void* getDisposedObjectsHCache(THCache* hc, int levels){
	TDataHCache *data = hc->data;
	TCache *cache=data->hcache[levels];

	return cache->getEvictedObjects(cache);
}

static void* getObjectsHCache(THCache* hc){ //@ Deve retornar todos os objetos da Hierarquia
	TDataHCache *data = hc->data;
	TCache *cache;
	int i, levels=data->levels;
	TListObject *objects,*aux[levels],*cabeca;

	objects=data->objects;

	for(i=0;i<levels;i++){
		cache=data->hcache[i];

		aux[i]=cache->getObjects(cache);

		cabeca=aux[i]->getHead(aux[i]);

		objects->insertTail(objects,cabeca);
	}

	return objects;
}

static int getLevelsHCache(THCache* hc){
	TDataHCache *data = hc->data;
	return data->levels;
}

static int getLevelPrincipalHCache(THCache* hc){
	TDataHCache *data = hc->data;
	return data->levelPrincipal;
}

static int getLevelReplicateHCache(THCache* hc){
	TDataHCache *data = hc->data;
	return data->levelReplicate;
}




static void* getCache(THCache *hc, int levels ){
	TDataHCache *data = hc->data;
	TCache *cache=data->hcache[levels];

	return cache;
}
//
static void* searchObjectHCache(THCache* hc, TObject *vObject){ //@ Deve retornar todos os objetos da Hierarquia
	TDataHCache *data = hc->data;
	TCache *cache;
	TObject *object = vObject;
	int i=0, levels=data->levels;
	TObject *storedObject=NULL;
	TListObject *listObject;

	while(i<levels && storedObject == NULL){
		cache=data->hcache[i];

		listObject = cache->getObjects(cache);
		storedObject = listObject->getObject(listObject, object);//@não esta listando todos os objetos, apenas niveis

		i++;
	}
	return storedObject;
}
//

static void addAvailabilityHCache(THCache* hc, int levels, TAvailabilityHCache amount){
	TDataHCache *data = hc->data;

	TCache *cache=data->hcache[levels];
	cache=cache->data;

		cache->addAvailability(cache,amount);

}


static void showStatsHCache(THCache* hc, int levels){
	TDataHCache *data = hc->data;
	TCache *cache=data->hcache[levels];

	cache->showStats(cache);

	//showStatsCache(data->cache[levels]);

	/*TStatsHCache *stats = data->stats;

	printf("hit: %d\n", stats->getHit( stats ) );
	printf("miss: %d\n", stats->getMiss( stats ) );
	printf("byte hits: %lf\n", stats->getByteHit( stats ) );
	printf("byte miss: %lf\n", stats->getByteMiss( stats ) );
	printf("on behalf hit: %d\n", stats->getCommunityHit( stats ) );
	printf("on behalf byte hit: %lf\n", stats->getByteCommunityHit( stats ) );*/

}

/*

// statistcs related data and functions and types
static THitStatsHCache getHitStatsHCache(TStatsHCache *stats);
static TMissStatsHCache getMissStatsHCache(TStatsHCache *stats);
static TMaxOccupancyStatsHCache getMaxOccupancyStatsHCache(TStatsHCache *stats);
static TCommunityHitStatsHCache getCommunityHitStatsHCache(TStatsHCache *stats);
static TByteCommunityHitStatsHCache getByteCommunityHitStatsHCache(TStatsHCache *stats);
static TByteHitStatsHCache getByteHitStatsHCache(TStatsHCache *stats);
static TByteMissStatsHCache getByteMissStatsHCache(TStatsHCache *stats);

static void setHitStatsHCache(TStatsHCache *stats, THitStatsHCache hit);
static void setMissStatsHCache(TStatsHCache *stats, TMissStatsHCache miss);
static void setMaxOccupancyStatsHCache(TStatsHCache *stats, TMaxOccupancyStatsHCache ocuppancy);
static void setCommunityHitStatsHCache(TStatsHCache *stats, TCommunityHitStatsHCache communityHits);
static void setByteCommunityHitStatsHCache(TStatsHCache *stats, TByteCommunityHitStatsHCache ByteCommunityHit);
static void setByteHitStatsHCache(TStatsHCache *stats, THitStatsHCache hit);
static void setByteMissStatsHCache(TStatsHCache *stats, TMissStatsHCache miss);

static void addHitStatsHCache(TStatsHCache *stats, THitStatsHCache amount);
static void addMissStatsHCache(TStatsHCache *stats, TMissStatsHCache amount);
static void addMaxOccupancyStatsHCache(TStatsHCache *stats, TMaxOccupancyStatsHCache amount);
static void addCommunityHitStatsHCache(TStatsHCache *stats, TCommunityHitStatsHCache amount);
static void addByteCommunityHitStatsHCache(TStatsHCache *stats, TByteCommunityHitStatsHCache amount);
static void addByteHitStatsHCache(TStatsHCache *stats, TByteHitStatsHCache amount);
static void addByteMissStatsHCache(TStatsHCache *stats, TByteMissStatsHCache amount);

typedef struct _data_StatsHCache TDataStatsHCache;
struct _data_StatsHCache{
	THitStatsHCache  hit;
	TMissStatsHCache miss;
	TMaxOccupancyStatsHCache maxOccupancy;
	TCommunityHitStatsHCache communityHit;
	TByteCommunityHitStatsHCache byteCommunityHit;
	TByteHitStatsHCache byteHit;
	TByteMissStatsHCache byteMiss;
};


TDataStatsHCache *initDataStatsHCache(){
	TDataStatsHCache *data;

	data = malloc(sizeof(TDataStatsHCache));
	data->communityHit = 0; // hits whenInto servicing community
	data->byteCommunityHit = 0; //byte community hits when servicing community
	data->hit = 0; // hits when servicing a peer
	data->miss = 0; // misses when servicing a peer
	data->maxOccupancy = 0; // max Occupancy over time
	data->byteHit = 0; // byte hits when servicing a peer
	data->byteMiss = 0; // byte misses when servicing a peer

	return data;
}
TStatsHCache *createStatsHCache(){
	TStatsHCache *stats;

	stats = (TStatsHCache*)malloc(sizeof(TStatsHCache));

	stats->data = initDataStatsHCache();

	stats->getHit = getHitStatsHCache;
	stats->getMiss = getMissStatsHCache;
	stats->getMaxOccupancy = getMaxOccupancyStatsHCache;
	stats->getCommunityHit = getCommunityHitStatsHCache;
	stats->getByteCommunityHit = getByteCommunityHitStatsHCache;
	stats->getByteHit = getByteHitStatsHCache;
	stats->getByteMiss = getByteMissStatsHCache;

	stats->setHit = setHitStatsHCache;
	stats->setMiss = setMissStatsHCache;
	stats->setMaxOccupancy = setMaxOccupancyStatsHCache;
	stats->setCommunityHit = setCommunityHitStatsHCache;
	stats->setByteCommunityHit = setByteCommunityHitStatsHCache;
	stats->setByteHit = setByteHitStatsHCache;
	stats->setByteMiss = setByteMissStatsHCache;

	stats->addHit = addHitStatsHCache;
	stats->addMiss = addMissStatsHCache;
	stats->addMaxOccupancy = addMaxOccupancyStatsHCache;
	stats->addCommunityHit = addCommunityHitStatsHCache;
	stats->addByteCommunityHit = addByteCommunityHitStatsHCache;
	stats->addByteHit = addByteHitStatsHCache;
	stats->addByteMiss = addByteMissStatsHCache;

	return stats;
}

static THitStatsHCache getHitStatsHCache(TStatsHCache *stats){
	TDataStatsHCache *data=stats->data;

	return data->hit;
}

static TMissStatsHCache getMissStatsHCache(TStatsHCache *stats){
	TDataStatsHCache *data=stats->data;

	return data->miss;
}

static TMaxOccupancyStatsHCache getMaxOccupancyStatsHCache(TStatsHCache *stats){
	TDataStatsHCache *data=stats->data;

	return data->maxOccupancy;
}

static TCommunityHitStatsHCache getCommunityHitStatsHCache(TStatsHCache *stats){
	TDataStatsHCache *data=stats->data;

	return data->communityHit;
}

static TByteCommunityHitStatsHCache getByteCommunityHitStatsHCache(TStatsHCache *stats){
	TDataStatsHCache *data=stats->data;

	return data->byteCommunityHit;
}

static TByteHitStatsHCache getByteHitStatsHCache(TStatsHCache *stats){
	TDataStatsHCache *data=stats->data;

	return data->byteHit;
}

static TByteMissStatsHCache getByteMissStatsHCache(TStatsHCache *stats){
	TDataStatsHCache *data=stats->data;

	return data->byteMiss;
}

static void setHitStatsHCache(TStatsHCache *stats, THitStatsHCache hit){
	TDataStatsHCache *data=stats->data;

	data->hit = hit;
}

static void setMissStatsHCache(TStatsHCache *stats, TMissStatsHCache miss){
	TDataStatsHCache *data=stats->data;

	data->miss = miss;
}

static void setMaxOccupancyStatsHCache(TStatsHCache *stats, TMaxOccupancyStatsHCache ocuppancy){
	TDataStatsHCache *data=stats->data;

	data->maxOccupancy = ocuppancy;
}

static void setCommunityHitStatsHCache(TStatsHCache *stats, TCommunityHitStatsHCache communityHits){
	TDataStatsHCache *data=stats->data;

	data->communityHit = communityHits;
}

static void setByteCommunityHitStatsHCache(TStatsHCache *stats, TByteCommunityHitStatsHCache ByteCommunityHit){
	TDataStatsHCache *data=stats->data;

	data->byteCommunityHit = ByteCommunityHit;
}

static void setByteHitStatsHCache(TStatsHCache *stats, THitStatsHCache hit){
	TDataStatsHCache *data=stats->data;

	data->byteHit = hit;
}

static void setByteMissStatsHCache(TStatsHCache *stats, TMissStatsHCache miss){
	TDataStatsHCache *data=stats->data;

	data->byteMiss = miss;
}

static void addHitStatsHCache(TStatsHCache *stats, THitStatsHCache amount){
	TDataStatsHCache *data=stats->data;

	data->hit += amount;
}

static void addMissStatsHCache(TStatsHCache *stats, TMissStatsHCache amount){
	TDataStatsHCache *data=stats->data;

	data->miss += amount;
}

static void addMaxOccupancyStatsHCache(TStatsHCache *stats, TMaxOccupancyStatsHCache amount){
	TDataStatsHCache *data=stats->data;

	data->maxOccupancy += amount;
}


static void addCommunityHitStatsHCache(TStatsHCache *stats, TCommunityHitStatsHCache amount){
	TDataStatsHCache *data=stats->data;

	data->communityHit += amount;
}

static void addByteCommunityHitStatsHCache(TStatsHCache *stats, TByteCommunityHitStatsHCache amount){
	TDataStatsHCache *data=stats->data;

	data->byteCommunityHit += amount;
}

static void addByteHitStatsHCache(TStatsHCache *stats, TByteHitStatsHCache amount){
	TDataStatsHCache *data=stats->data;

	data->byteHit += amount;
}

static void addByteMissStatsHCache(TStatsHCache *stats, TByteMissStatsHCache amount){
	TDataStatsHCache *data=stats->data;

	data->byteMiss += amount;
}

*/


/*
int main(){
    int i;
    THCache *hc, int levels, int levels;
    TObject *object;
    char idVideo[400], time[6], colon;
    int views, ratings, min, sec;
    float stars;

 // hc with 2000 seconds And LRU Object MaNaGement policy

    hc = initHCache(200000, LRUHCache);

    while(!feof(stdin)){

        scanf("%s", idVideo);
        scanf("%d%d",&min,&sec);
        scanf("%d", &views);
        scanf("%d", &ratings);
        scanf("%f", &stars);

        object = initObject(idVideo, (min*60+sec), views, 0);
        insertHCache(hc, object);

        printf("%d %d\n", getAvailabilityHCache(hc), getSizeHCache(hc));

    };

}

 */
