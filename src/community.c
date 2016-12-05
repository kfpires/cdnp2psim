/*
 * community.c
 *
 *  Created on: Jan 10, 2013
 *      Author: cesar
 */
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "hash.h"
#include "system.h"
#include "datasource.h"
#include "randomic.h"
#include "cache.h"
#include "symtable.h"
#include "internals.h"
#include "object.h"
#include "peer.h"
#include "topology.h"
#include "search.h"
#include "hierarchy.h"

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/xmlreader.h>

#include "xmlparserconfig.h"

#include "channel.h"
#include "community.h"

typedef struct _tier TTier;
struct _tier{
	int size;
	int startIn;
	TSearch *searching;
	TFluctuation *fluctuation;
};

typedef struct _tiers TTiers;
struct _tiers{
	int numberOf;
	TTier* tier;
};

//typedef struct _data_community TDataCommunity;
struct _data_community{
	TPeer** peers; //TPeer
	TArrayDynamic *alive;
	unsigned int *alivePeers;
	int size;
	TTiers *tiers;
};



static void setupChurnPeerCommunity(int id, TPeer *peer, xmlDocPtr doc, TSymTable *randST){
	char xpath[1000]={[0]='\0'};
	char *xdynamic=NULL, *xleaveDynamic=NULL, *xjoinDynamic=NULL;
	char pars[1000], *parameter=NULL, *value=NULL;
	char entry[1000]={[0]='\0'};
	char *last=NULL;
	void *leaveChurn, *leavePickChurn;
	void *joinChurn, *joinPickChurn;

	sprintf(xpath,"/community/tier[%d]/peer/churn/leave/parameter[@name=\"dynamic\"]",id+1);
	xleaveDynamic = xgetOneParameter(doc, xpath);


	sprintf(xpath,"/community/tier[%d]/peer/churn/leave/pick/parameter[@name=\"dynamic\"]",id+1);
	xdynamic = xgetOneParameter(doc, xpath);
	randST->getPars(randST,xdynamic,pars);
	sprintf(entry+strlen(entry),"%s;",xdynamic);

	entry[0]='\0';
	parameter = strtok_r(pars, PARAMETERS_SEPARATOR, &last);
	while(parameter){
		sprintf(xpath,"/community/tier[%d]/peer/churn/leave/pick/parameter[@name=\"%s\"]",id+1,parameter);
		value = xgetOneParameter(doc,xpath);
		sprintf(entry+strlen(entry),"%s;",value); free(value);
		parameter = strtok_r(NULL, PARAMETERS_SEPARATOR, &last);
	}

	leavePickChurn = randST->caller(randST,xdynamic,entry);
	leaveChurn = randST->caller(randST,xleaveDynamic,leavePickChurn);

	peer->setDynamicLeave(peer,leaveChurn);

	free(xdynamic); // free the dynamic memory allocated by xgetOnParameter
	free(xleaveDynamic);


	sprintf(xpath,"/community/tier[%d]/peer/churn/join/parameter[@name=\"dynamic\"]",id+1);
	xjoinDynamic = xgetOneParameter(doc, xpath);

	// set up the joining dynamic of peers
	sprintf(xpath,"/community/tier[%d]/peer/churn/join/pick/parameter[@name=\"dynamic\"]",id+1);

	xdynamic = xgetOneParameter(doc, xpath);
	randST->getPars(randST,xdynamic,pars);

	entry[0]='\0';
	parameter = strtok_r(pars, PARAMETERS_SEPARATOR, &last);
	while(parameter){
		sprintf(xpath,"/community/tier[%d]/peer/churn/join/pick/parameter[@name=\"%s\"]",id+1,parameter);
		value = xgetOneParameter(doc,xpath);
		sprintf(entry+strlen(entry),"%s;",value); free(value);
		parameter = strtok_r(NULL, PARAMETERS_SEPARATOR, &last);
	}

	joinPickChurn = randST->caller(randST,xdynamic,entry);
	joinChurn = randST->caller(randST,xjoinDynamic,joinPickChurn);

	peer->setDynamicJoin(peer,joinChurn);
	free(xdynamic); // free the dynamic memory allocated by xgetOnParameter
	free(xjoinDynamic);
}

static void setupContentPeerCommunity(int id, TPeer *peer, xmlDocPtr doc, TSymTable *randSymTable, TSymTable *DCsymTable, TSymTable *DSsymTable){
	char xpath[1000]={[0]='\0'};
	char *xdynamic=NULL;
	char pars[1000]={[0]='\0'}, *parameter=NULL, *value=NULL;
	char entry[1000]={[0]='\0'};
	char *last=NULL;

	TRandomic *requestContent, *requestDataCatalog;
	TDataSource *dataSource;
	TDataCatalog *dataCatalog;

	sprintf(xpath,"/community/tier[%d]/peer/content/request/parameter[@name=\"dynamic\"]",id+1);

	xdynamic = xgetOneParameter(doc, xpath);
	randSymTable->getPars(randSymTable,xdynamic,pars);

	entry[0]='\0';
	parameter = strtok_r(pars, PARAMETERS_SEPARATOR, &last);
	while(parameter){
		sprintf(xpath,"/community/tier[%d]/peer/content/request/parameter[@name=\"%s\"]",id+1,parameter);
		value = xgetOneParameter(doc,xpath);
		sprintf(entry+strlen(entry),"%s;",value); free(value);
		parameter = strtok_r(NULL, PARAMETERS_SEPARATOR, &last);
	}

	requestContent = randSymTable->caller(randSymTable,xdynamic,entry);//
	peer->setDynamicRequest(peer,requestContent);
	free(xdynamic); // free the dynamic memory allocated by xgetOnParameter

	sprintf(xpath,"/community/tier[%d]/peer/content/datasource/prefetch/parameter[@name=\"dynamic\"]",id+1);
	xdynamic = xgetOneParameter(doc, xpath);

	randSymTable->getPars(randSymTable,xdynamic,pars);
	entry[0]='\0';
	parameter = strtok_r(pars, PARAMETERS_SEPARATOR, &last);
	while(parameter){
		sprintf(xpath,"/community/tier[%d]/peer/content/datasource/prefetch/parameter[@name=\"%s\"]",id+1,parameter);
		value = xgetOneParameter(doc,xpath);
		sprintf(entry+strlen(entry),"%s;",value); free(value);
		parameter = strtok_r(NULL, PARAMETERS_SEPARATOR, &last);
	}

	TPrefetch *prefetch = (TPrefetch*)randSymTable->caller(randSymTable,xdynamic,entry);//
	free(xdynamic); // free the dynamic memory allocated by xgetOnParameter

	sprintf(xpath,"/community/tier[%d]/peer/content/datasource/access/parameter[@name=\"dynamic\"]",id+1);
	xdynamic = xgetOneParameter(doc, xpath);
	randSymTable->getPars(randSymTable,xdynamic,pars);

	entry[0]='\0';
	parameter = strtok_r(pars, PARAMETERS_SEPARATOR, &last);
	while(parameter){
		sprintf(xpath,"/community/tier[%d]/peer/content/datasource/access/parameter[@name=\"%s\"]",id+1,parameter);
		value = xgetOneParameter(doc,xpath);
		sprintf(entry+strlen(entry),"%s;",value); free(value);
		parameter = strtok_r(NULL, PARAMETERS_SEPARATOR, &last);
	}

	requestDataCatalog = randSymTable->caller(randSymTable,xdynamic,entry);//
	free(xdynamic); // free the dynamic memory allocated by xgetOnParameter

	//get content data source dynamic
	sprintf(xpath,"/community/tier[%d]/peer/content/datasource/catalog/parameter[@name=\"dynamic\"]",id+1);
	xdynamic = xgetOneParameter(doc, xpath);
	DCsymTable->getPars(DCsymTable,xdynamic,pars);

	//get content data source parameters
	entry[0]='\0';
	parameter = strtok_r(pars, PARAMETERS_SEPARATOR, &last);
	while(parameter){
		sprintf(xpath,"/community/tier[%d]/peer/content/datasource/catalog/parameter[@name=\"%s\"]",id+1,parameter);
		value = xgetOneParameter(doc,xpath);
		sprintf(entry+strlen(entry),"%s;",value); free(value);
		parameter = strtok_r(NULL, PARAMETERS_SEPARATOR, &last);
	}

	TParsDataCatalog *parsDC = createParsDataCatalog(entry,requestDataCatalog);
	dataCatalog = DCsymTable->caller(DCsymTable, xdynamic, parsDC);
	free(parsDC);

	free(xdynamic); // free the dynamic memory allocated by xgetOnParameter
	sprintf(xpath,"/community/tier[%d]/peer/content/datasource/parameter[@name=\"dynamic\"]",id+1);
	xdynamic = xgetOneParameter(doc, xpath);

	dataSource = DSsymTable->caller(DSsymTable,xdynamic,dataCatalog);
	dataSource->prefetch = prefetch;

	peer->setDataSource(peer,dataSource);

	free(xdynamic); // free the dynamic memory allocated by xgetOnParameter

}

static void setupCachePeerCommunity(int id, TPeer *peer, xmlDocPtr doc, TSymTable *xlog){
	char xpath[1000]={[0]='\0'};
	char *xdynamic=NULL, *xsize;
	char pars[1000]={[0]='\0'}, *parameter=NULL, *value=NULL;
	char entry[1000]={[0]='\0'};
	char *separator = ";";
	char *last=NULL;
	xmlChar *xValue;
	int levels,i;


	sprintf(xpath,"/community/tier[%d]/peer/caches",id+1);
	xValue = xgetPropertyCommunity(doc, (xmlChar *)xpath,(xmlChar*)"levels");
	levels = atoi((char*)xValue);xmlFree(xValue);

	THCache *hc = createHCache(levels);//cria a hierarquia de caches


	for(i=0;i<levels;i++){

		sprintf(xpath,"/community/tier[%d]/peer/caches/cache[%d]/parameter[@name=\"size\"]",id+1,i+1);
		xsize = xgetOneParameter(doc, xpath);
		//
		sprintf(xpath,"/community/tier[%d]/peer/caches/cache[%d]/policy/parameter[@name=\"dynamic\"]",id+1,i+1);
		xdynamic = xgetOneParameter(doc, xpath);

		xlog->getPars(xlog,xdynamic,pars);

		entry[0]='\0';
		parameter = strtok_r(pars, separator, &last);
		while(parameter){
			sprintf(xpath,"/community/tier[%d]/peer/caches/cache[%d]/policy/parameter[@name=\"%s\"]",id+1,i+1,parameter);
			value = xgetOneParameter(doc,xpath);
			sprintf(entry+strlen(entry),"%s;",value); free(value);
			parameter = strtok_r(NULL, separator, &last);
		}

		void *policy = xlog->caller(xlog,xdynamic,entry);//

		TCache *cache= createCache(atoi(xsize),policy);
		hc->putCache(hc,i,cache);

		free(xdynamic); // free the dynamic memory allocated by xgetOnParameter

	}//end for

	peer->setHCache(peer,hc);//Set HCache for peer
}

static void setupTopologyManagerPeerCommunity(int id, TPeer *peer, xmlDocPtr doc, TSymTable *topologyST){
	char xpath[1000]={[0]='\0'};
	char *xdynamic=NULL, *xMaxConnections=NULL, *xMaxAttempts=NULL;
	char pars[1000]={[0]='\0'}, *parameter=NULL, *value=NULL;
	char entry[1000]={[0]='\0'};
	char *separator = ";";
	char *last=NULL;

	sprintf(xpath,"/community/tier[%d]/peer/topology/parameter[@name=\"maxConnections\"]",id+1);
	xMaxConnections = xgetOneParameter(doc, xpath);
	sprintf(entry,"%s;",xMaxConnections); free(xMaxConnections);
	//
	sprintf(xpath,"/community/tier[%d]/peer/topology/parameter[@name=\"maxAttempts\"]",id+1);
	xMaxAttempts = xgetOneParameter(doc, xpath);
	sprintf(entry+strlen(entry),"%s;",xMaxAttempts); free(xMaxAttempts);

	//
	sprintf(entry+strlen(entry),"%d;",peer->getId(peer));

	//
	sprintf(xpath,"/community/tier[%d]/peer/topology/manager/parameter[@name=\"dynamic\"]",id+1);
	xdynamic = xgetOneParameter(doc, xpath);
	topologyST->getPars(topologyST,xdynamic,pars);


	//entry[0]='\0';
	parameter = strtok_r(pars, separator, &last);
	while(parameter){
		sprintf(xpath,"/community/tier[%d]/peer/topology/manager/parameter[@name=\"%s\"]",id+1,parameter);
		value = xgetOneParameter(doc,xpath);
		sprintf(entry+strlen(entry),"%s;",value); free(value);
		parameter = strtok_r(NULL, separator, &last);
	}

	void *topoManager = topologyST->caller(topologyST,xdynamic,entry);//
	peer->setTopologyManager(peer,topoManager);

	free(xdynamic); // free the dynamic memory allocated by xgetOnParameter

}

static void setupProfilePeerCommunity(int id, TPeer *peer, xmlDocPtr doc, TSymTable *symTable){
	char xpath[1000]={[0]='\0'};
	char *xdynamic=NULL;
	char pars[1000]={[0]='\0'}, *parameter=NULL, *value=NULL;
	char entry[1000]={[0]='\0'};
	char *separator = ";";
	char *last;

	//
	sprintf(xpath,"/community/tier[%d]/peer/profile/parameter[@name=\"dynamic\"]",id+1);
	xdynamic = xgetOneParameter(doc, xpath);
	symTable->getPars(symTable,xdynamic,pars);

	entry[0]='\0';
	parameter = strtok_r(pars, separator, &last);
	while(parameter){
		sprintf(xpath,"/community/tier[%d]/peer/profile/parameter[@name=\"%s\"]",id+1,parameter);
		value = xgetOneParameter(doc,xpath);
		sprintf(entry+strlen(entry),"%s;",value); free(value);
		parameter = strtok_r(NULL, separator, &last);
	}

	void *profilePolicy = symTable->caller(symTable,xdynamic,entry);//
	peer->setProfilePolicy(peer,profilePolicy);

	free(xdynamic); // free the dynamic memory allocated by xgetOnParameter

}


static void* setupSearchingCommunity(int id, xmlDocPtr doc, TSymTable *searchingST){
	char xpath[1000]={[0]='\0'};
	char *xdynamic=NULL;
	char pars[1000]={[0]='\0'}, *parameter=NULL, *value=NULL;
	char entry[1000]={[0]='\0'};
	char *separator = ";";
	char *last=NULL;

	sprintf(xpath,"/community/tier[%d]/search/policy/parameter[@name=\"dynamic\"]",id+1);
	xdynamic = xgetOneParameter(doc, xpath);
	searchingST->getPars(searchingST,xdynamic,pars);

	entry[0]='\0';
	parameter = strtok_r(pars, separator, &last);
	while(parameter){
		sprintf(xpath,"/community/tier[%d]/search/policy/parameter[@name=\"%s\"]",id+1,parameter);
		value = xgetOneParameter(doc,xpath);
		sprintf(entry+strlen(entry),"%s;",value); free(value);
		parameter = strtok_r(NULL, separator, &last);
	}

	void *searching = searchingST->caller(searchingST,xdynamic,entry);//

	free(xdynamic); // free the dynamic memory allocated by xgetOnParameter

	return searching;
}
// ############### Setup Fluctuation
static void* setupFluctuationCommunity(int id, xmlDocPtr doc, TSymTable *fluctuationST){
	char xpath[1000]={[0]='\0'};
	char *xdynamic=NULL;
	char pars[1000]={[0]='\0'}, *parameter=NULL, *value=NULL;
	char entry[1000]={[0]='\0'};
	char *separator = ";";
	char *last=NULL;

	sprintf(xpath,"/community/tier[%d]/fluctuation/policy/parameter[@name=\"dynamic\"]",id+1);
	xdynamic = xgetOneParameter(doc, xpath);
	fluctuationST->getPars(fluctuationST,xdynamic,pars);

	entry[0]='\0';
	parameter = strtok_r(pars, separator, &last);
	while(parameter){
		sprintf(xpath,"/community/tier[%d]/fluctuation/policy/parameter[@name=\"%s\"]",id+1,parameter);
		value = xgetOneParameter(doc,xpath);
		sprintf(entry+strlen(entry),"%s;",value); free(value);
		parameter = strtok_r(NULL, separator, &last);
	}

	void *policy = fluctuationST->caller(fluctuationST,xdynamic,entry);//

	TFluctuation *fluctuation = createDataFluctuation(policy);

	free(xdynamic); // free the dynamic memory allocated by xgetOnParameter

	return fluctuation;
}
// ############### End Setup Fluctuation


//Setup Canal
static void setupChannelPeerCommunity(int id, TPeer *peer, xmlDocPtr doc, TSymTable *channelST){
	char xpath[1000]={[0]='\0'};
	char *xCapacity=NULL, *xRateUplink=NULL,*xRateDownlink=NULL;
	char pars[1000]={[0]='\0'}, *parameter=NULL, *value=NULL;
	char entry[1000]={[0]='\0'};
	char *separator = ";";
	char *last=NULL;
	float capacity;
	float rateUplink, rateDownlink;
	TChannel *channel;

	char *xdynamic=NULL;

	//void *uplinkTPChannel;
	//void *downlinkTPChannel;
	//
	sprintf(xpath,"/community/tier[%d]/peer/channel/parameter[@name=\"capacity\"]",id+1);
	xCapacity = xgetOneParameter(doc, xpath);
	capacity = atof((char*)xCapacity);
	free(xCapacity);

	// modificacao do Channel
	sprintf(xpath,"/community/tier[%d]/peer/channel/uplink/parameter[@name=\"rateUplink\"]",id+1);
	//xuplinkDynamic = xgetOneParameter(doc, xpath);
	xRateUplink = xgetOneParameter(doc, xpath);
	rateUplink = atof((char*)xRateUplink);
	free(xRateUplink);

	sprintf(xpath,"/community/tier[%d]/peer/channel/uplink/throughput/parameter[@name=\"dynamic\"]",id+1);
	xdynamic = xgetOneParameter(doc, xpath);
	channelST->getPars(channelST,xdynamic,pars);

	entry[0]='\0';
	parameter = strtok_r(pars, PARAMETERS_SEPARATOR, &last);
	while(parameter){
		sprintf(xpath,"/community/tier[%d]/peer/channel/uplink/throughput/parameter[@name=\"%s\"]",id+1,parameter);
		value = xgetOneParameter(doc,xpath);
		sprintf(entry+strlen(entry),"%s;",value); free(value);
		parameter = strtok_r(NULL, PARAMETERS_SEPARATOR, &last);

	}

	void *policy = channelST->caller(channelST,xdynamic,entry);//
	TLink *upLink = createDataLink(rateUplink, policy);

	free(xdynamic); // free the dynamic memory allocated by xgetOnParameter

	sprintf(xpath,"/community/tier[%d]/peer/channel/downlink/parameter[@name=\"rateDownlink\"]",id+1);
	xRateDownlink = xgetOneParameter(doc, xpath);
	rateDownlink = atof((char*)xRateDownlink);
	free(xRateDownlink);

	// set up the joining dynamic of peers
	sprintf(xpath,"/community/tier[%d]/peer/channel/downlink/throughput/parameter[@name=\"dynamic\"]",id+1);
	xdynamic = xgetOneParameter(doc, xpath);
	channelST->getPars(channelST,xdynamic,pars);

	entry[0]='\0';
	parameter = strtok_r(pars, PARAMETERS_SEPARATOR, &last);
	while(parameter){
		sprintf(xpath,"/community/tier[%d]/peer/channel/downlink/throughput/parameter[@name=\"%s\"]",id+1,parameter);
		value = xgetOneParameter(doc,xpath);
		sprintf(entry+strlen(entry),"%s;",value); free(value);
		parameter = strtok_r(NULL, PARAMETERS_SEPARATOR, &last);
	}

	policy = channelST->caller(channelST,xdynamic,entry);

	TLink *downLink= createDataLink(rateDownlink, policy);

	free(xdynamic); // free the dynamic memory allocated by xgetOnParameter

	channel = createDataChannel(capacity, upLink,downLink);
	peer->setChannel(peer,channel);
}


/*
//Setup Fluctutation Channel
static void setupFluctuationChannelPeerCommunity(int id, TPeer *peer, xmlDocPtr doc, TSymTable *fluctuationST){
	char xpath[1000]={[0]='\0'};
	char *xdynamic=NULL;
	char pars[1000]={[0]='\0'}, *parameter=NULL, *value=NULL;
	char entry[1000]={[0]='\0'};
	char *separator = ";";
	char *last=NULL;

	sprintf(xpath,"/community/tier[%d]/fluctuation/policy/parameter[@name=\"dynamic\"]",id+1);
	xdynamic = xgetOneParameter(doc, xpath);
	fluctuationST->getPars(fluctuationST,xdynamic,pars);

	entry[0]='\0';
	parameter = strtok_r(pars, separator, &last);
	while(parameter){
		sprintf(xpath,"/community/tier[%d]/fluctuation/policy/parameter[@name=\"%s\"]",id+1,parameter);
		value = xgetOneParameter(doc,xpath);
		sprintf(entry+strlen(entry),"%s;",value); free(value);
		parameter = strtok_r(NULL, separator, &last);
	}

	void *fluctuation = fluctuationST->caller(fluctuationST,xdynamic,entry);//

	free(xdynamic); // free the dynamic memory allocated by xgetOnParameter
	peer->setFluctuation(peer,fluctuation);

}
//End Fluctuation
*/

static xmlDocPtr readConfigForCommunity(const char *fname){
	const xmlChar *pattern = (const xmlChar *)"community";

	/* Init libxml */
	xmlInitParser();
	LIBXML_TEST_VERSION

	/* open and rede the XML*/
	xmlTextReaderPtr reader = xmlReaderForFile(fname, NULL, 0);

	/* extract the community description */
	xmlDocPtr doc = extract_subdocument(reader, pattern );

	xmlTextReaderClose(reader);

	return doc;
}


static short hasIntoCommunity(TCommunity* community, void *vObject, void *vHashTable,  unsigned int *idPeer){
	short found = 0;
	float storedCandidate;
	float length,stored;
	TObject *object = vObject;
	THashTable *hashTable = vHashTable;
	TKeyHashTable idObject;
	TKeeperHashTable *keepers;
	THCache *hc;  //@
	TObject *storedObject;
	unsigned int id = -1;
	int levelInit=NULL,levelEnd=NULL;
	//TListObject *listObject;
	TDataCommunity *data = community->data;

	getIdObject(object, idObject);

	keepers = hashTable->lookup(hashTable, idObject);

	while(!found && keepers!=NULL){ // find the holder with best quality video
		*idPeer = (unsigned int) getOwnerKeeperHashTable(keepers);
		TPeer *peer = data->peers[*idPeer];
		if ( !peer->isDown(peer) ){
			// lookup the Object into the peer's Cache

			hc = peer->getHCache(peer);
			storedObject = hc->search(hc,object,levelInit,levelEnd);


			if (storedObject == NULL){
				printf("PANIC: stored Object on hasCommunity!!");
				exit(0);
			}

			length = getLengthObject(storedObject);
			stored = getStoredObject(storedObject);

			if (id == -1){ // first holder that is UP
				storedCandidate = stored;
				id = *idPeer;
				if ( stored == length )
					found = 1;
				else
					keepers = getNextKeeperHashTable(keepers);

			}else if ( stored == length ) {  // has best quality object?
				id = *idPeer;
				found = 1;
			}else if ( storedCandidate < stored ) {
				storedCandidate = stored;
				id = *idPeer;
				keepers = getNextKeeperHashTable(keepers);
			}else{
				keepers = getNextKeeperHashTable(keepers);

			}
		} else {
			keepers = getNextKeeperHashTable(keepers);
			*idPeer = -1;
		}
	}
	if (id != -1){
		found = 1;
		*idPeer = id;
	}

	return found;
}

static int howManyReplicateIntoCommunity(TCommunity* community, void *vObject, void *vHashTable){
	int totalReplicate = 0;
	unsigned int idPeer = -1;
	TObject *object = vObject;
	THashTable *hashTable = vHashTable;
	TKeyHashTable idObject;
	TKeeperHashTable *keepers;
	TDataCommunity *data = community->data;
	getIdObject(object, idObject);

	keepers = hashTable->lookup(hashTable,idObject);

	while(keepers!=NULL){ // find the holder with best quality video
		idPeer = (unsigned int) getOwnerKeeperHashTable(keepers);
		TPeer *peer = data->peers[idPeer];
		if ( !peer->isDown(peer) )
			totalReplicate++;

		keepers = getNextKeeperHashTable(keepers);
	}

	return totalReplicate;
}


static void printStatCommunity(TCommunity* community){
	int i,j,levels;
	THCache *hc;
	TStatsCache *statsCache;
	TStatsPeer *statsPeer;
	TDataCommunity *data = community->data;

	unsigned long int totalHit=0, totalMiss=0;

	unsigned long int totalByteHit=0, totalByteMiss=0;
	unsigned int totalRequest=0;
	unsigned int totalUpTime=0, totalDownTime=0;

	for(i=0;i<data->size;i++){
		TPeer *peer = data->peers[i];
		hc = peer->getHCache(peer);
		levels=hc->getLevels(hc);
		//pegar as estatiticas de todos os niveis
		j=0;
		while(j<levels){


			statsCache = hc->getStats(hc,j);// @



			totalHit += statsCache->getHit( statsCache );
			totalHit += statsCache->getCommunityHit( statsCache );

			totalMiss += statsCache->getMiss( statsCache );

			totalByteHit += statsCache->getByteHit( statsCache );
			totalByteHit += statsCache->getByteCommunityHit( statsCache );

			totalByteMiss += statsCache->getByteMiss( statsCache );



			printf("===>Id peer: %d<===\n", i);
			printf("===>Level Cache: %d<===\n", j);
			hc->showStats(hc,j);
			j++;
		}//laço dos niveis da hierarquia de cache de um par

		statsPeer = peer->getOnStats(peer);
		totalRequest += getRequestStatsPeer(statsPeer);
		totalUpTime += getUpTimeStatsPeer(statsPeer);
		totalDownTime += getDownTimeStatsPeer(statsPeer);

			printf("Total hits: %ld \n", totalHit);
			printf("Total misses: %ld\n", totalMiss);
			printf("Total Byte hits: %ld\n", totalByteHit);
			printf("Total Byte misses: %ld\n", totalByteMiss);

			printf("Hit rate: %f \n", (float)totalHit/(float)(totalHit+totalMiss));
			printf("Miss rate: %f\n", (float)totalMiss/(float)(totalHit+totalMiss));
			printf("Byte hit rate: %f\n", (float)totalByteHit/(float)(totalByteHit+totalByteMiss));
			printf("Byte miss rate: %f\n", (float)totalByteMiss/(float)(totalByteHit+totalByteMiss));

			printf("Requests: %d \n", totalRequest);
			printf("UpTime  : %d \n", totalUpTime);
			printf("DownTime: %d \n", totalDownTime);


			printf("%f %f ", (float)totalHit/(float)(totalHit+totalMiss), (float)totalMiss/(float)(totalHit+totalMiss));


	}
}

static void collectStatCommunity(TCommunity* community, float *hitRate, float *missRate, float *byteHitRate, float *byteMissRate, unsigned long int *totalRequests, int *peersUp, float *hitRateCom){
	int i,levels;
	THCache *hc;
	TStatsCache *statsCache;
	TStatsPeer *statsPeer;
	TDataCommunity *data = community->data;

	unsigned long int totalHit=0, totalMiss=0;
	unsigned long int totalHitCom=0;

	unsigned long int totalByteHit=0, totalByteMiss=0;
	unsigned int totalUpTime=0, totalDownTime=0;

	*peersUp = 0;
	*totalRequests = 0;

	for(i=0;i<data->size;i++){
		TPeer *peer = data->peers[i];
		if (peer->isUp(peer)){

			*peersUp += 1;
		}
		hc = peer->getHCache(peer); //@ recuperar hierarquia de memoria do par
		levels=hc->getLevels(hc);

		int j=0;
		while(j<levels){

			statsCache = hc->getStats(hc,j); //@ Coletar estatisticas da hierarquia



			totalHit += statsCache->getHit( statsCache );
			totalHit += statsCache->getCommunityHit( statsCache );
			totalHitCom += statsCache->getCommunityHit( statsCache );

			totalMiss += statsCache->getMiss( statsCache );

			totalByteHit += statsCache->getByteHit( statsCache );
			totalByteHit += statsCache->getByteCommunityHit( statsCache );

			totalByteMiss += statsCache->getByteMiss( statsCache );




			//printf("===>Id peer: %d<===\n", i);
			//printOnStatsCache(cache);
			j++;
		}
		statsPeer = peer->getOnStats(peer);
		*totalRequests += getRequestStatsPeer(statsPeer);
		totalUpTime += getUpTimeStatsPeer(statsPeer);
		totalDownTime += getDownTimeStatsPeer(statsPeer);
	}

	*hitRate = (float)totalHit/(float)(totalHit+totalMiss);
	*missRate = (float)totalMiss/(float)(totalHit+totalMiss);

	*byteHitRate = (float)totalByteHit/(float)(totalByteHit+totalByteMiss);
	*byteMissRate = (float)totalByteMiss/(float)(totalByteHit+totalByteMiss);

	*hitRateCom = (float)totalHitCom/(totalMiss+totalHitCom);
}


static void collectStatTiersCommunity(TCommunity* community, unsigned long int timestamp){
	int i,k,levels;
	THCache *hc;
	TStatsCache *statsCache;
	TStatsPeer *statsPeer;
	TDataCommunity *data = community->data;
	float hitRate, missRate, byteHitRate, byteMissRate;
	float hitRateCom;
	int peersUp;
	unsigned long int totalRequests;
	unsigned long int totalHit=0, totalMiss=0;
	unsigned long int totalHitCom=0;

	unsigned long int totalByteHit=0, totalByteMiss=0;
	unsigned int totalUpTime=0, totalDownTime=0;

	long int zero=0;
	peersUp = 0;
	totalRequests = 0;

	for(k=0;k<data->tiers->numberOf;k++){
		totalHit=0, totalMiss=0;
		totalHitCom=0;

		totalByteHit=0, totalByteMiss=0;
		totalUpTime=0, totalDownTime=0;

		peersUp = 0;
		totalRequests = 0;
		int start = data->tiers->tier[k].startIn;
		int size = data->tiers->tier[k].size;
		for(i=start;i<start+size;i++){
			TPeer *peer = data->peers[i];
			if (peer->isUp(peer)){
				peersUp += 1;
			}
			hc = peer->getHCache(peer);//@ Hierarquia de memoria do par
			levels=hc->getLevels(hc);
			int j=0;
			while(j<levels){
				statsCache = hc->getStats(hc,j);//@ estatisticas da comunidade



				totalHit += statsCache->getHit( statsCache );
				totalHit += statsCache->getCommunityHit( statsCache );
				totalHitCom += statsCache->getCommunityHit( statsCache );

				totalMiss += statsCache->getMiss( statsCache );

				totalByteHit += statsCache->getByteHit( statsCache );
				totalByteHit += statsCache->getByteCommunityHit( statsCache );

				totalByteMiss += statsCache->getByteMiss( statsCache );



				//printf("===>Id peer: %d<===\n", i);
				//printOnStatsCache(cache);
				j++;
			}
			statsPeer = peer->getOnStats(peer);
			totalRequests += getRequestStatsPeer(statsPeer);
			totalUpTime += getUpTimeStatsPeer(statsPeer);
			totalDownTime += getDownTimeStatsPeer(statsPeer);

		}

		hitRate = (float)totalHit/(float)(totalHit+totalMiss);
		missRate = (float)totalMiss/(float)(totalHit+totalMiss);

		byteHitRate = (float)totalByteHit/(float)(totalByteHit+totalByteMiss);
		byteMissRate = (float)totalByteMiss/(float)(totalByteHit+totalByteMiss);

		hitRateCom = (float)totalHitCom/(totalMiss+totalHitCom);

		printf("%lu ", timestamp);
		printf("%f %f ", hitRate, missRate);
		printf("%f %f ", byteHitRate, byteMissRate);
		printf("%f ", hitRateCom);

		printf("%ld ", zero);
		printf("%ld ", zero);
		printf("%ld ",totalRequests);
		printf("%d\n",peersUp);

	}
}


static void resetStatCommunity(TCommunity* community){
	int i, levels;
	THCache *hc;
	TCache *cache;
	TStatsCache *statsCache;
	TStatsPeer *statsPeer;
	TDataCommunity *data = community->data;

	for(i=0;i<data->size;i++){
		TPeer *peer = data->peers[i];
		hc = peer->getHCache(peer);

		levels=hc->getLevels(hc);

		int j=0;
		while(j<levels){
			cache=hc->getCache(hc,j);
			statsCache = cache->getStats(cache);

			statsCache->setHit( statsCache, 0 );
			statsCache->setCommunityHit( statsCache, 0 );

			statsCache->setMiss( statsCache, 0 );

			statsCache->setByteHit( statsCache, 0 );
			statsCache->setByteCommunityHit( statsCache, 0 );

			statsCache->setByteMiss( statsCache, 0 );



			//printf("===>Id peer: %d<===\n", i);
			//printOnStatsCache(cache);
			j++;
		}
		statsPeer = peer->getOnStats(peer);
		setRequestStatPeer(statsPeer, 0);
		setUpTimeStatsPeer(statsPeer, 0);
		setDownTimeStatsPeer(statsPeer, 0);

	}

}


static unsigned long int onCacheCommunity(TCommunity* community){
	int i, lPrincipal;//@ Levels
	THCache *hc;
	TCache *cache;
	TDataCommunity *data = community->data;

	unsigned long int totalCached=0;

	for(i=0;i<data->size;i++){
		TPeer *peer = data->peers[i];
		if (peer->isUp(peer)){
			hc = peer->getHCache(peer);
			lPrincipal=hc->getLevelPrincipal(hc);
			cache=hc->getCache(hc,lPrincipal);

			//pegar cache principal
			totalCached += cache->getOccupancy(cache);//@ pegar a ocupacao da cache principal

		}
	}
	return totalCached;
}

static void *getPeerCommunity(TCommunity* community, unsigned int idPeer){
	TDataCommunity *data = community->data;

	return data->peers[idPeer];
}

static int getSizeCommunity(TCommunity* community){
	TDataCommunity *data = community->data;

	return data->size;
}

static void setAlivePeerCommunity(TCommunity *community, unsigned int idPeer){
	TDataCommunity *data = community->data;

	TArrayDynamic *alive = data->alive;
	TPeer *p = community->getPeer(community,idPeer);

	alive->insert(alive, idPeer, p);

}

static void unsetAlivePeerCommunity(TCommunity *community, unsigned int idPeer){
	TDataCommunity *data = community->data;

	TArrayDynamic *alive = data->alive;

	alive->remove(alive, idPeer);

}

static short isAlivePeerCommunity(TCommunity *community, unsigned int idPeer){
	TDataCommunity *data = community->data;
	TArrayDynamic *alive = data->alive;

	return (alive->retrieval(alive,idPeer)?1:0);

}

static void* getAlivePeerCommunity(TCommunity *community){
	TDataCommunity *data = community->data;
	TArrayDynamic *alive = data->alive;

	return alive;
}


static int getNumberOfAlivePeerCommunity(TCommunity *community){
	TDataCommunity *data = community->data;
	TArrayDynamic *alive = data->alive;

	return alive->getOccupancy(alive);
}

static void* searchingCommunity(TCommunity *community, void *vpeer, void *object, unsigned int clientId, float prefetchRate){
	TDataCommunity *data = community->data;
	TPeer *peer = vpeer;
	int tierPeer = peer->getTier(peer);

	TSearch *search = data->tiers->tier[tierPeer-1].searching;

	return search->run(search,peer,object,clientId, prefetchRate);

}

//fluctuationCommunity
static void fluctuationCommunity(TCommunity *community){
	TDataCommunity *data = community->data;
	TPeer *peer;

/*	TFluctuation *fluctuation = data->tiers->tier[0].fluctuation;

	fluctuation->updateFluctuation (fluctuation);*/

	int sizeComm = community->getSize(community);
	unsigned int i;

	for(i=0;i<sizeComm;i++){

		peer = community->getPeer(community, i);
		if (peer->isUp(peer)){
			peer->updateTimeForFluctuation(peer);
		}
	}
}





//

static void disposeCommunity(TCommunity* community){
	int i;
	TDataCommunity *data = community->data;

	for(i=0;i<data->size;i++)
		free(data->peers[i]);

	free(data->peers);
	free(data);
	free(community);
}

TCommunity* createCommunity(int simTime, char *scenarios){
	xmlChar *xValue;
	int xTiers, xSize;
	int i,j,k;
	char xExp[1000];

	TPeer *p;

	TSymTable *symTable = createSymTable();

	TCommunity* community = (TCommunity *)malloc(sizeof(TCommunity));
	TDataCommunity *dataComm = malloc(sizeof(TDataCommunity));

	xmlDocPtr doc = readConfigForCommunity(scenarios);

	xValue = xgetPropertyCommunity(doc, (xmlChar *)"/community",(xmlChar*)"tiers");
	xTiers = atoi((char*)xValue);xmlFree(xValue);

	xValue = xgetPropertyCommunity(doc, (xmlChar *)"/community", (xmlChar*)"size");
	xSize = atoi((char*)xValue);xmlFree(xValue);

	dataComm->peers = (TPeer **)malloc(sizeof(TPeer*)*xSize);
	dataComm->size = xSize;
	dataComm->alive = createArrayDynamic((int)(xSize*.1));
	dataComm->alivePeers = (unsigned int *)malloc(sizeof(unsigned int)*xSize);
	dataComm->tiers = malloc(sizeof(TTiers));;
	dataComm->tiers->numberOf = xTiers;
	dataComm->tiers->tier = malloc(sizeof(TTier)*xTiers);
	k=0;
	for(i=0;i<dataComm->tiers->numberOf;i++){

		sprintf(xExp,"/community/tier[%d]",i+1);
		xValue = xgetPropertyCommunity(doc, (xmlChar *)xExp,(xmlChar*)"size");
		xSize = atoi((char*)xValue);xmlFree(xValue);
		dataComm->tiers->tier[i].size = xSize;
		dataComm->tiers->tier[i].startIn = k;

		dataComm->tiers->tier[i].searching = setupSearchingCommunity(i, doc, symTable);
		dataComm->tiers->tier[i].fluctuation = setupFluctuationCommunity(i, doc, symTable);

		for(j=0;j<xSize;j++){

			p = createPeer((unsigned int)k, i+1, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL );

			// setup
			setupChurnPeerCommunity(i, p, doc, symTable);
			setupContentPeerCommunity(i, p, doc, symTable, symTable, symTable);
			setupCachePeerCommunity(i, p, doc, symTable);
			setupTopologyManagerPeerCommunity(i, p, doc, symTable);
			setupProfilePeerCommunity(i, p, doc, symTable);
			//Canal
			setupChannelPeerCommunity(i, p, doc, symTable);
			//setupFluctuationChannelPeerCommunity(i, p, doc, symTable);

			dataComm->peers[k] = p;
			k++;

		}
	}

	community->data = dataComm;

	community->collectStatistics = collectStatCommunity;
	community->collectStatsOnTiers = collectStatTiersCommunity;
	community->resetStatistics = resetStatCommunity;
	community->printStatistics = printStatCommunity;

	community->howManyReplicate = howManyReplicateIntoCommunity;
	community->onCache = onCacheCommunity;
	community->getPeer = getPeerCommunity;
	community->getSize = getSizeCommunity;
	community->setAlivePeer = setAlivePeerCommunity;
	community->unsetAlivePeer = unsetAlivePeerCommunity;
	community->isAlivePeer = isAlivePeerCommunity;
	community->has = hasIntoCommunity;

	community->getAlivePeer= getAlivePeerCommunity;
	community->getNumberOfAlivePeer = getNumberOfAlivePeerCommunity;
	community->searching = searchingCommunity;
	community->fluctuation = fluctuationCommunity;

	community->dispose = disposeCommunity;

	symTable->dispose(symTable);

	return community;
}







