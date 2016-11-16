/*
 * catalog.c

 *
 *  Created on: Jul 27, 2012
 *      Author: cesar
 */
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "dictionary.h"
#include "cache.h"
#include "randomic.h"
#include "datasource.h"
#include "topology.h"
#include "search.h"
#include "peer.h"
#include "symtable.h"
#include "replicate.h"
#include "channel.h"
#include "community.h"


#define fatal_error(str, pattern) fprintf(stderr,"%s %s\n", str, pattern),exit(0)

void* callerSymTable(TSymTable *log, char *pattern, void *pars);
void getCallerSymTable(TSymTable *log, char *pattern, void **caller);
void getParsSymTable(TSymTable *log, char *pattern, char *pars);
void disposeSymTable(TSymTable *log);

typedef char TParsSymTable [1000];
typedef void*(*TCreateSymTable)(void *pars);

typedef struct _data_CreateSymTable TDataCreateSymTable;
struct _data_CreateSymTable{
	TCreateSymTable create;
	TParsSymTable pars;
};

static void *getContentSymTable(TSymTable *symTable, char *pattern){

	TDictionary *d = (TDictionary*)symTable->data;
	TKeyDictionary key = d->keyGenesis(pattern);

	return d->retrieval(d,key);

}
void* callerSymTable(TSymTable *symTable, char *pattern, void *pars){
	TDataCreateSymTable *content = getContentSymTable(symTable, pattern);

	if(content)
		return content->create(pars);
	else
		fatal_error("symtable.c: Caller unavailable:", pattern);
		return NULL;
}

void getCallerSymTable(TSymTable *symTable, char *pattern, void **caller){
	TDataCreateSymTable *content = getContentSymTable(symTable,pattern);

	if (content)
		*caller = content->create;
	else
		*caller=NULL;

}

void getParsSymTable(TSymTable *symTable, char *pattern, char *pars){
	TDataCreateSymTable *content = getContentSymTable(symTable,pattern);

	if (content)
		strcpy(pars,content->pars);
	else
		strcpy(pars, "\0");
}

void disposeSymTable(TSymTable *symTable){
	if (symTable){
		if (symTable->data){
			TDictionary * d = symTable->data;
			d->ufree(d);
		}
		free(symTable);
	}
}



static void* initDataSourceSymTable(TDictionary* d){
	TKeyDictionary key;
	TDataCreateSymTable *sym;

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("DATASOURCE:COLLECTION");
	sym->create= (TCreateSymTable)createFromCollectionDataSource;
	d->insert(d,key,sym);

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("DATASOURCE:PLAYLIST");
	sym->create= (TCreateSymTable)createFromPlaylistDataSource;
	d->insert(d,key,sym);

	return d;
}


static void* initDataCatalogSymTable(TDictionary* d){
	TKeyDictionary key;
	TDataCreateSymTable *sym;

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("DATACATALOG:FROMCOLLECTIONSINGLETON");
	sym->create= (TCreateSymTable)createFromCollectionSingletonDataCatalog;
	sprintf(sym->pars,"collection;length");
	d->insert(d,key,sym);

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("DATACATALOG:FROMCOLLECTION");
	sym->create= (TCreateSymTable)createFromCollectionDataCatalog;
	sprintf(sym->pars,"collection;length");
	d->insert(d,key,sym);

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("DATACATALOG:FROMPLAYLIST");
	sym->create= (TCreateSymTable)createFromPlaylistDataCatalog;
	sprintf(sym->pars,"setlist;length;collection");
	d->insert(d,key,sym);

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("DATACATALOG:FROMPLAYLISTSINGLETON");
	sym->create= (TCreateSymTable)createFromPlaylistSingletonDataCatalog;
	sprintf(sym->pars,"setlist;length;collection");
	d->insert(d,key,sym);

	return d;
}


// TO BE IMPROVED: build a static dictionary
// from a XML FILE
static void* initRandomicSymTable(TDictionary *d){
	TKeyDictionary key;
	TDataCreateSymTable *sym;

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("RANDOM:TRUNCATEDGEOMETRIC");
	sym->create= (TCreateSymTable)createTruncatedGeometricRandomic;
	sprintf(sym->pars,"alpha;");
	d->insert(d,key,sym);

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("RANDOM:TRUNCATEDGEOMETRICSINGLETON");
	sym->create= (TCreateSymTable)createSingletonTruncatedGeometricRandomic;
	sprintf(sym->pars,"alpha;");
	d->insert(d,key,sym);

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("RANDOM:PARETO");
	sym->create= (TCreateSymTable)createParetoRandomic;
	sprintf(sym->pars,"alpha;mean;");
	d->insert(d,key,sym);

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("RANDOM:PARETOSINGLETON");
	sym->create= (TCreateSymTable)createSingletonParetoRandomic;
	sprintf(sym->pars,"alpha;mean;");
	d->insert(d,key,sym);


	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("RANDOM:EXPONENTIAL");
	sym->create= (TCreateSymTable)createExponentialRandomic;
	sprintf(sym->pars,"mean;");
	d->insert(d,key,sym);

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("RANDOM:EXPONENTIALSINGLETON");
	sym->create= (TCreateSymTable)createSingletonExponentialRandomic;
	sprintf(sym->pars,"mean;");
	d->insert(d,key,sym);


	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("RANDOM:LOGNORMAL");
	sym->create=(TCreateSymTable)createLognormalRandomic;
	sprintf(sym->pars,"mean;");
	d->insert(d,key,sym);

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("RANDOM:LOGNORMALSINGLETON");
	sym->create=(TCreateSymTable)createSingletonLognormalRandomic;
	sprintf(sym->pars,"mean;");
	d->insert(d,key,sym);

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("RANDOM:CONSTANT");
	sym->create = (TCreateSymTable)createConstantRandomic;
	sprintf(sym->pars,"value;");
	d->insert(d,key,sym);

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("RANDOM:CONSTANTSINGLETON");
	sym->create = (TCreateSymTable)createSingletonConstantRandomic;
	sprintf(sym->pars,"value;");
	d->insert(d,key,sym);

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("RANDOM:ZIPF");
	sym->create = (TCreateSymTable)createZipfRandomic;
	sprintf(sym->pars,"alpha;range;");
	d->insert(d,key,sym);

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("RANDOM:ZIPFSINGLETON");
	sym->create = (TCreateSymTable)createSingletonZipfRandomic;
	sprintf(sym->pars,"alpha;range;");
	d->insert(d,key,sym);

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("RANDOM:POISSON");
	sym->create = (TCreateSymTable)createPoissonRandomic;
	sprintf(sym->pars,"mean;");
	d->insert(d,key,sym);

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("RANDOM:POISSONSINGLETON");
	sym->create = (TCreateSymTable)createSingletonPoissonRandomic;
	sprintf(sym->pars,"mean;");
	d->insert(d,key,sym);

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("RANDOM:UNIFORM");
	sym->create = (TCreateSymTable)createUniformRandomic;
	sprintf(sym->pars,"range;");
	d->insert(d,key,sym);

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("RANDOM:UNIFORMSINGLETON");
	sym->create = (TCreateSymTable)createSingletonUniformRandomic;
	sprintf(sym->pars,"range;");
	d->insert(d,key,sym);

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("RANDOM:FROMFILE");
	sym->create = (TCreateSymTable)createFromFileRandomic;
	sprintf(sym->pars,"source;");
	d->insert(d,key,sym);

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("RANDOM:FROMFILESINGLETON");
	sym->create = (TCreateSymTable)createSingletonFromFileRandomic;
	sprintf(sym->pars,"source;");
	d->insert(d,key, sym);

	return d;
}


static void* initPrefetchSymTable(TDictionary *d){
	TKeyDictionary key;
	TDataCreateSymTable *sym;

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("PREFETCH:NONE");
	sym->create= (TCreateSymTable)createPrefetchNone;
	sprintf(sym->pars,";");
	d->insert(d,key,sym);

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("PREFETCH:NEXTFROMPLAYLIST");
	sym->create= (TCreateSymTable)createPrefetchNextFromPlaylist;
	sprintf(sym->pars,"fraction;");
	d->insert(d,key,sym);

	return d;
}





//
static void* initReplicateSymTable(TDictionary *d){
	TKeyDictionary key;
	TDataCreateSymTable *sym;

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("REPLICATE:NONE");
	sym->create= (TCreateSymTable)createReplicateNone;
	sprintf(sym->pars,";");
	d->insert(d,key,sym);

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("REPLICATE:RANDOM");
	sym->create= (TCreateSymTable)createReplicateRandom;
	sprintf(sym->pars,"bfraction;swindow");
	d->insert(d,key,sym);

	return d;
}


//@ Channel
//Table of Symbols
//Link Management Policy
// Init Link Management Policy Type
static void* initLMPolicySymTable(TDictionary *d){
	TKeyDictionary key;
	TDataCreateSymTable *sym;

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("THROUGHPUT:FROMFILE");
	sym->create = (TCreateSymTable)createFROMFILEPolicy;
	sprintf(sym->pars,"limsup;source");
	d->insert(d,key,sym);

	return d;
}

//*
static void* initChannelFluctuationSymTable(TDictionary *d){
	TKeyDictionary key;
	TDataCreateSymTable *sym;

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("FLUCTUATION:FROMFILE");
	sym->create = (TCreateSymTable)createFluctuationFROMFILEPolicy;
	sprintf(sym->pars,"source;");
	d->insert(d,key,sym);

	return d;
}


//*
//
//Table of Symbols
//Object management Policy
// Init Object Management Policy Type
static void* initOMPolicySymTable(TDictionary *d){
	TKeyDictionary key;
	TDataCreateSymTable *sym;


	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("OMP:GDSA");
	sym->create = createGDSAPolicy;
	sprintf(sym->pars,";");
	d->insert(d,key,sym);

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("OMP:GDSP");
	sym->create = createGDSPPolicy;
	sprintf(sym->pars,"timescale;");
	d->insert(d,key,sym);

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("OMP:LFU");
	sym->create = createLFUPolicy;
	sprintf(sym->pars,";");
	d->insert(d,key,sym);

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("OMP:LRU");
	sym->create=createLRUPolicy;
	sprintf(sym->pars,";");
	d->insert(d,key,sym);

	//@ FIFO for Replicated
	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("OMP:FIFO");
	sym->create=createFIFOPolicy;
	sprintf(sym->pars,";");
	d->insert(d,key,sym);

	//@ LRU for Replicated
	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("OMP:LRUREP");
	sym->create=createLRURepPolicy;
	sprintf(sym->pars,";");
	d->insert(d,key,sym);

	return d;
}


//
// Table of Symbols
//
// Init Search Policy Type
static void* initSearchingSymTable(TDictionary *d){
	TDataCreateSymTable *sym;
	TKeyDictionary key;

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("SEARCH:RANDOMWALK");
	sym->create = (TCreateSymTable)createRandomWalkSearch;
	sprintf(sym->pars,"maxTries;");
	d->insert(d,key,sym);

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("SEARCH:FLOADING");
	sym->create = (TCreateSymTable)createFloadingSearch;
	sprintf(sym->pars,"maxLevels;");
	d->insert(d,key,sym);

	return d;
}

//
// Table of Symbols
//
// Init Topology Management Policy Type
static void* initSessionLastingPolicySymTable(TDictionary *d){
	TDataCreateSymTable *sym;
	TKeyDictionary key;

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("SESSIONLASTING:PLAYLIST");
	sym->create = createPlaylistSessionLasting;
	sprintf(sym->pars,";");
	d->insert(d,key,sym);

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("SESSIONLASTING:PARTIALPLAYLIST");
	sym->create = createPartialPlaylistSessionLasting;
	sprintf(sym->pars,"pick;");
	d->insert(d,key,sym);


	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("SESSIONLASTING:RANDOM");
	sym->create = createRandomSessionLasting;
	sprintf(sym->pars,"pick;");
	d->insert(d,key,sym);

	return d;
}


//
// Table of Symbols
//
// Init Topology Management Policy Type
static void* initTopologyManagerSymTable(TDictionary *d){
	TDataCreateSymTable *sym;
	TKeyDictionary key;

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("TOPOLOGY:RANDOM");
	sym->create = createRandomTopology;
	sprintf(sym->pars,";");
	d->insert(d,key,sym);

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("TOPOLOGY:PROACTIVE");
	sym->create = createProactiveTopology;
	sprintf(sym->pars,";");
	d->insert(d,key,sym);

	//@ Topologia baseada no inicio de Sessão
	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("TOPOLOGY:SESSION");
	sym->create = createSessionTopology;
	sprintf(sym->pars,";");
	d->insert(d,key,sym);

	return d;
}


//
// Table of Symbols
//
// Init the Profile Policy for peers
static void* initProfilePeerSymTable(TDictionary *d){
	TDataCreateSymTable *sym;
	TKeyDictionary key;

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("PROFILE:JACCARD");
	sym->create = createJaccardContentProfilePeer;
	sprintf(sym->pars,";");
	d->insert(d,key,sym);

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("PROFILE:ANDERBERG");
	sym->create = createJaccardContentProfilePeer;
	sprintf(sym->pars,";");
	d->insert(d,key,sym);

	sym = malloc(sizeof(TDataCreateSymTable));
	key = d->keyGenesis("PROFILE:SESSION");
	sym->create = createStartSessionProfilePeer;
	sprintf(sym->pars,";");
	d->insert(d,key,sym);

	return d;
}


TSymTable *createSymTable(){
	TSymTable *symTable = malloc(sizeof(struct symTable));
	TDictionary* dic = createDictionary();

	initDataCatalogSymTable(dic);
	initPrefetchSymTable(dic);
	initDataSourceSymTable(dic);
	initTopologyManagerSymTable(dic);
	initSessionLastingPolicySymTable(dic);
	initSearchingSymTable(dic);
	initReplicateSymTable(dic);
	initOMPolicySymTable(dic);
	initRandomicSymTable(dic);
	initProfilePeerSymTable(dic);
	initLMPolicySymTable(dic);
	initChannelFluctuationSymTable(dic);

	symTable->data = dic;

	symTable->caller = callerSymTable;
	symTable->getCaller = getCallerSymTable;
	symTable->getPars = getParsSymTable;
	symTable->dispose = disposeSymTable;

	return symTable;
}
