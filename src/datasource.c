/*
 * datasource.c
 *
 *  Created on: Jul 2, 2012
 *      Author: cesar
 */
#include <time.h>
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "system.h"
#include "randomic.h"
#include "object.h"
#include "dictionary.h"
#include "internals.h"
#include "datasource.h"

struct _data_playlist {
	char id[20];
	char onwer[50];
	int duration;
	int inCollection;
	int length;
	int countHint;
	char published[25];
	char updated[25];
	int next;
	int prefetchNext;
	void **objects;
};

typedef int (*TDurationPlaylist)(TPlaylist *);
typedef int (*TFirstkDurationPlaylist)(TPlaylist *, int);
typedef int (*TLengthPlaylist)(TPlaylist *);
typedef void *(*TNextPlaylist)(TPlaylist *);
typedef void *(*TNextPrefetchPlaylist)(TPlaylist *);
typedef TPlaylist *(*TClonePlaylist)(TPlaylist *);
static TPlaylist* initPlaylist(char *id, char *owner, int duration, int length,
		int countHint, char *published, char *updated, void** objects);

struct playlist {
	void *data;
	TDurationPlaylist duration;
	TFirstkDurationPlaylist firstkduration;
	TLengthPlaylist length;
	TNextPlaylist next;
	TNextPrefetchPlaylist nextPrefetch;
	TClonePlaylist clone;
};

static void* nextPlaylist(TPlaylist *pl) {
	struct _data_playlist*data = pl->data;

	if (data->next + 1 == data->length)
		return NULL ;
	data->next++;
	if(data->prefetchNext < data->next)
		data->prefetchNext = data->next;
	return data->objects[data->next];
}

static void* nextPrefetchPlaylist(TPlaylist *pl) {
	struct _data_playlist* data = pl->data;

	if (data->prefetchNext + 1 == data->length)
		return NULL ;
	data->prefetchNext++;
	return data->objects[data->prefetchNext];
}

static int durationPlaylist(TPlaylist *pl) {
	struct _data_playlist*data = pl->data;
	return data->duration;
}
static int firstkDurationPlaylist(TPlaylist *pl, int k) {
	int i, duration = 0;
	struct _data_playlist*data = pl->data;
	for (i = 0; i < k && i < data->length; i++) {
		duration = duration + getLengthObject(data->objects[i]);
	}
	return duration;
}
static int lengthPlaylist(TPlaylist *pl) {
	struct _data_playlist*data = pl->data;
	return data->length;
}

static TPlaylist* clonePlaylist(TPlaylist *src) {
	struct _data_playlist *data = src->data;
	void **objects;
	int i;

	objects = newCatalogObject(data->length);
	for (i = 0; i < data->length; i++) {
		objects[i] = cloneObject(data->objects[i]);
	}

	return initPlaylist(data->id, data->onwer, data->duration, data->length,
			data->countHint, data->published, data->updated, objects);
}

static void disposePlaylist(TPlaylist *src) {
	struct _data_playlist *data = src->data;
	void **objects = data->objects;
	int i;

	for (i = 0; i < data->length; i++) {
		disposeObject(objects[i]);
	}
	free(data->objects);
	free(data);
	free(src);

}

static TPlaylist* initPlaylist(char *id, char *owner, int duration, int length,
		int countHint, char *published, char *updated, void** objects) {
	struct _data_playlist *data = malloc(sizeof(struct _data_playlist));
	TPlaylist *pl = malloc(sizeof(TPlaylist));

	strcpy(data->id, id);
	strcpy(data->onwer, owner);
	strcpy(data->published, published);
	strcpy(data->updated, updated);
	data->countHint = countHint;
	data->duration = duration;
	data->length = length;
	data->objects = objects;

	data->next = -1;
	data->prefetchNext = -1;

	pl->data = data;
	pl->next = nextPlaylist;
	pl->nextPrefetch = nextPrefetchPlaylist;
	pl->duration = durationPlaylist;
	pl->firstkduration = firstkDurationPlaylist;
	pl->length = lengthPlaylist;
	pl->clone = clonePlaylist;
	return pl;
}

typedef struct _data_FromCollection TFromCollectionDataCatalog;
typedef struct _data_FromPlayList TFromPlaylistDataCatalog;

//Catalog Object implementation
//
//

//struct dataCatalog{
//	void *data;
//	TRandomic *dynamic;
//	TDisposeDataCatalog dispose;
//};

struct _data_FromCollection {
	void **objects;
	unsigned int size;
};

struct _data_FromPlayList {
	void **objects;
	TPlaylist *playlist;
	unsigned int size;
};

//data source init
static TFromCollectionDataCatalog *initFromCollectionDataCatalog(char *filename,
		unsigned int size) {
	TFromCollectionDataCatalog *data;
	void **objects;
	FILE* fp;

	char idVideo[400];
	char xuploaded[26];
	xuploaded[0] = '\0';
	int views, min, sec, ms, lengthBytes;//@_05/10
	float length;
	int stars;
	//float ratings;

	unsigned int i = 0;

	fp = fopen(filename, "r");
	if (!fp) {
		fprintf(stderr, "ERROR:datasource.c -- FILE NOT FOUND: %s \n",
				filename);
		exit(0);
	}

	data = (TFromCollectionDataCatalog *) malloc(
			sizeof(TFromCollectionDataCatalog));

	objects = newCatalogObject(size);

	fscanf(fp, "%s %d %d %d %d %d %d %s", idVideo, &min, &sec, &ms, &views, &stars,	&lengthBytes, xuploaded);
	//fscanf(fp, "%s %d %d %d %d %f", idVideo, &min, &sec, &views,  &stars, &ratings);
	while (!feof(fp) && (i < size)) {

		//fscanf(fp, "%s %d %d %d %d %f %s", idVideo, &min, &sec, &views,  &stars, &ratings, xuploaded);

		length = (min * 60*1000 + sec +((float)ms/1000));//Tamanho em tempo de milissegundos

		objects[i] = initObject(idVideo, length,lengthBytes, views, 0);
		setStoredObject(objects[i], length);
		setLPopularityObject(objects[i], 0);
		setUploadObject(objects[i], xuploaded);
		setAccessFrequencyObject(objects[i], 0.094214); // Access Stream dependent. Used by GDSP Policy
		setLastAccessObject(objects[i], 0);
		setCumulativeValueObject(objects[i], 0.0);
		setNormalizedByteServedObject(objects[i], 0.0);

		setBitRateObject(objects[i], 128.f);

		i++;
		//fscanf(fp, "%s %d %d %d %d %f", idVideo, &min, &sec, &views,  &stars, &ratings);
		fscanf(fp, "%s %d %d %d %d %d %d %s", idVideo, &min, &sec, &ms, &views, &stars,
				&lengthBytes, xuploaded);
	}

	if (size > i) {
		fprintf(stderr,
				"ERROR:datasource.c::loadCatalogFromFile: CATALOG SOURCE HAS LESS OBJECTS THAN REQUESTED\n");
		fprintf(stderr,
				"ERROR:datasource.c::loadCatalogFromFile: THE CATALOG SIZE IS %d\n",i);
		exit(0);
	}

	data->objects = objects;
	data->size = size;

	fclose(fp);

	return data;
}

struct _pars_DataCatalog {
	char *entry;
	void *randomic;
};

TParsDataCatalog* createParsDataCatalog(char *entry, void *randomic) {
	TParsDataCatalog *pars = malloc(sizeof(TParsDataCatalog));

	pars->entry = entry;
	pars->randomic = randomic;

	return pars;
}

TDataCatalog *createFromCollectionSingletonDataCatalog(TParsDataCatalog *pars) {
	// store all setup DataCatalog
	static TDictionary *d = NULL;

	TDataCatalog *dataCatalog = NULL;
	char *entry = pars->entry;
	//void *randomic=pars->randomic;
	char *filename;
	// extract parameters from user's entry limited by semi-color
	TParameters *lp = createParameters(entry, PARAMETERS_SEPARATOR);

	lp->iterator(lp);

	// walk through the parameters as setting up
	filename = lp->next(lp);
	if (!d)
		d = createDictionary();

	TKeyDictionary key;

	key = d->keyGenesis(filename);
	if (d->has(d, key))
		dataCatalog = d->retrieval(d, key);
	else {
		dataCatalog = createFromCollectionDataCatalog(pars);
		d->insert(d, key, dataCatalog);
	}

	lp->dispose(lp);

	return dataCatalog;
}

TDataCatalog *createFromCollectionDataCatalog(TParsDataCatalog *pars) {
	char *filename;
	unsigned int size;
	char *entry = pars->entry;
	void *randomic = pars->randomic;
	TDataCatalog *ds;
	// extract parameters from user's entry limited by semi-color
	TParameters *lp = createParameters(entry, PARAMETERS_SEPARATOR);

	lp->iterator(lp);

	// walk through the parameters as setting up
	filename = lp->next(lp);
	size = atol(lp->next(lp));

	ds = malloc(sizeof(TDataCatalog));
	ds->data = initFromCollectionDataCatalog(filename, size);
	ds->dynamic = randomic;
	ds->dispose = disposeFromCollectionDataCatalog;

	lp->dispose(lp);

	return ds;
}

void disposeFromCollectionDataCatalog(TDataCatalog *dataCatalog) {
	TFromCollectionDataCatalog *data;
	if (dataCatalog) {
		data = dataCatalog->data;
		if (data->objects) {
			disposeCatalogObject((TObject **) data->objects, data->size);
			free(data->objects);
			free(data);
		}
		free(dataCatalog);
	}
}

static void cleanupFromPlayListDataCatalog(TDictionary *d){
	TObject *object = d->first(d);

	char idVideo[400]={[0]='\0'};
	TKeyDictionary key;

	while (object!=NULL){

		getIdObject(object, idVideo);
		key = d->keyGenesis(idVideo);

		object = d->remove(d,key);

		disposeObject(object);
		object = d->first(d);

	}

}

//data catalog from playlist
//functions and definitions
TSetList **initFromPlayListDataCatalog(char *playLists, unsigned int length,
		char *catalog) {
	TDictionary *d = createDictionary();
	void **list;
	TPlaylist *playList;

	FILE* fp;

	void *object;
	TKeyDictionary key;
	char idVideo[400];
	char xuploaded[26];
	int views, min, stars, sec, ms, lengthBytes;
	float ratings,duration;
	char line[6000];
	char *lineptr;

	char *idPL, *ownerPL;
	char *durationPL, *inCollectionPL, *lengthPL;
	char *idVideoPL;
	char *countHintL, *publishedL, *updatedL;

	unsigned int i = 0, k = 0;

	fp = fopen(catalog, "r");
	if (!fp) {
		fprintf(stderr, "ERROR:datasource.c -- FILE NOT FOUND: %s \n", catalog);
		exit(0);
	}

	fscanf(fp, "%s %d %d %d %d %d %d %s", idVideo, &min, &sec, &ms, &views, &stars,
			&lengthBytes, xuploaded);
	while (!feof(fp)) {

		duration = (min * 60*1000 + sec + ((float)ms/1000));//@ to milliseconds


		object = initObject(idVideo, duration,lengthBytes, views, 0);

		//setRatingObject(object, ratings);
		setRatingObject(object, lengthBytes);
		setUploadObject(object, xuploaded);
		setStoredObject(object, duration);

		setLPopularityObject(object, 0);
		setAccessFrequencyObject(object, 0.094214); // Access Stream dependent. Used by GDSP Policy
		setLastAccessObject(object, 0);
		setCumulativeValueObject(object, 0.0);
		setNormalizedByteServedObject(object, 0.0);

		setBitRateObject(object, 128.f);

		key = d->keyGenesis(idVideo);
		d->insert(d, key, object);

		i++;

		fscanf(fp, "%s %d %d %d %d %d %d %s", idVideo, &min, &sec, &ms, &views, &stars,
				&lengthBytes, xuploaded);

	}

	fp = fopen(playLists, "r");
	if (!fp) {
		fprintf(stderr, "ERROR:datasource.c -- FILE NOT FOUND: %s \n", catalog);
		exit(0);
	}

	TSetList **setList = malloc(sizeof(TPlaylist*) * length);
	k = 0;
	fscanf(fp, "%s", line);
	while (!feof(fp)) {

		idPL = strtok_r(line, "|", &lineptr);
		ownerPL = strtok_r(NULL, "|", &lineptr);
		durationPL = strtok_r(NULL, "|", &lineptr);
		inCollectionPL = strtok_r(NULL, "|", &lineptr);
		lengthPL = strtok_r(NULL, "|", &lineptr);
		countHintL = strtok_r(NULL, "|", &lineptr);
		publishedL = strtok_r(NULL, "|", &lineptr);
		updatedL = strtok_r(NULL, "|", &lineptr);

		int iLengthPL = atoi(lengthPL);
		if (iLengthPL > 0){
			i = 0;

			list = newCatalogObject(iLengthPL);

			idVideoPL = strtok_r(NULL, "|", &lineptr);
			while ( (i< iLengthPL) && (idVideoPL) ) {
				object = d->retrieval(d, d->keyGenesis(idVideoPL));
				if (object) {
					list[i] = cloneObject(object);
					i++;
				}
				idVideoPL = strtok_r(NULL, "|", &lineptr);
			}

			playList = initPlaylist(idPL, ownerPL, atoi(durationPL), i,
					atoi(countHintL), publishedL, updatedL, list);

			setList[k] = playList;
			k++;
		}

		fscanf(fp, "%s", line);
	}
	fclose(fp);

	cleanupFromPlayListDataCatalog(d);

	d->ufree(d);

	if (length > k) {
		fprintf(stderr,
				"ERROR:datasource.c::loadCatalogFromFile: CATALOG SOURCE HAS LESS OBJECTS THAN REQUESTED\n");
		fprintf(stderr,
				"ERROR:datasource.c::loadCatalogFromFile: THE CATALOG SIZE IS %d\n",
				i);
		exit(0);
	}

	return setList;

}

TDataCatalog *createFromPlaylistDataCatalog(TParsDataCatalog *pars) {
	char *catalog, *setList;
	unsigned int length;
	char *entry = pars->entry;
	void *randomic = pars->randomic;
	TFromPlaylistDataCatalog *playlistdc = malloc(
			sizeof(TFromPlaylistDataCatalog));
	TDataCatalog *dc = malloc(sizeof(TDataCatalog));

	// extract parameters from user's entry limited by semi-color
	TParameters *lp = createParameters(entry, PARAMETERS_SEPARATOR);

	lp->iterator(lp);

	// walk through the parameters as setting up
	setList = lp->next(lp);
	length = atol(lp->next(lp));
	catalog = lp->next(lp);

	playlistdc->objects = initFromPlayListDataCatalog(setList, length, catalog);
	playlistdc->size = length;


	dc->data = playlistdc;
	dc->dynamic = randomic;
	dc->dispose = disposeFromPlaylistDataCatalog;

	lp->dispose(lp);

	return dc;
}

void disposeFromPlaylistDataCatalog(TDataCatalog *dataCatalog) {

	if (dataCatalog) {
		TFromPlaylistDataCatalog *data = dataCatalog->data;
		if (data->objects)
			disposeCatalogObject((TObject **) data->objects, data->size);
		if (data->playlist) {
			disposePlaylist(data->playlist);
		}
		free(data);
		free(dataCatalog);
	}
}

TDataCatalog *createFromPlaylistSingletonDataCatalog(TParsDataCatalog *pars) {
	// store all Data Catalog from playlist
	static TDictionary *d = NULL;

	TKeyDictionary key;
	TDataCatalog *dc = NULL;
	char *setList;
	char *entry = pars->entry;
	// extract parameters from user's entry limited by semi-color
	TParameters *lp = createParameters(entry, PARAMETERS_SEPARATOR);

	lp->iterator(lp);

	// get the DATa Catalog setlist
	//
	setList = lp->next(lp);

	if (!d)
		d = createDictionary();

	key = d->keyGenesis(setList);
	if (d->has(d, key)) {
		dc = d->retrieval(d, key);
	} else {
		dc = createFromPlaylistDataCatalog(pars);
		dc->dispose = disposeFromPlaylistSingletonDataCatalog;

		d->insert(d, key, dc);
	}

	lp->dispose(lp);

	return dc;
}

void disposeFromPlaylistSingletonDataCatalog(TDataCatalog *dataCatalog) {

	printf(
			"datasource.c: disposeFromPlaylistSingletonDataCatalog: DANGER: Dispose on a SINGLETON\n");

	disposeFromPlaylistDataCatalog(dataCatalog);

}

// data source related functions and definitions
//
float getPrefetchRateDataSource(TDataSource *dataSource) {
	TPrefetch *prefetch = dataSource->prefetch;

	return prefetch->fraction;
}

void *pickFromCollectionDataSource(TDataSource *dataSource) {
	TObject *obj;
	// pick one video from catalog

	unsigned int i;
	TDataCatalog *dataCatalog = dataSource->datacatalog;
	TFromCollectionDataCatalog *data = dataCatalog->data;

	TRandomic *randomic = dataCatalog->dynamic;

	i = randomic->pick(randomic);

	if ((i < 0) || (i >= data->size)) {
		printf("PANIC: Error on pick UP procedure %u\n", i);
		exit(0);
	}

	void **objects = data->objects;

	addLPopularityObject(objects[i]);

	obj = cloneObject(objects[i]);
	setLPopularityObject(obj, 0);

	return obj;
}

void *pickTopKFromCatalogDataSource(TDataSource *catalog, float ratio) {
	TObject *obj, **objects;

	// pick one video from catalog
	//
	TListObject *listObject = createListObject();
	TListObject *topK = createListObject();
	TFromCollectionDataCatalog *dataCatalog = catalog->datacatalog;

	unsigned int K = (unsigned int) (dataCatalog->size * ratio);
	unsigned int i;

	objects = (TObject**) dataCatalog->objects;

	for (i = 0; i < dataCatalog->size; i++) {

		obj = cloneObject(objects[i]);
		listObject->insertOrd(listObject, obj, lpopularityAsCriteriaObject);
	}

	for (i = 0; i < K; i++) {
		obj = cloneObject(listObject->getTail(listObject));
		listObject->removeTail(listObject);
		topK->insertTail(topK, obj);
	}

	listObject->destroy(listObject);

	return topK;
}

int sizeFromPlaylistDataSource(TDataSource *dataSource){

	TDataCatalog *dataCatalog = dataSource->datacatalog;
	TFromPlaylistDataCatalog *data = dataCatalog->data;
	TPlaylist *pl =data->playlist;
	return pl->length(pl);
}
int durationFromPlaylistDataSource(TDataSource *dataSource){

	TDataCatalog *dataCatalog = dataSource->datacatalog;
	TFromPlaylistDataCatalog *data = dataCatalog->data;
	TPlaylist *pl =data->playlist;
	return pl->duration(pl);
}

int firstkDurationFromPlaylistDataSource(TDataSource *dataSource, int k){

	TDataCatalog *dataCatalog = dataSource->datacatalog;
	TFromPlaylistDataCatalog *data = dataCatalog->data;
	TPlaylist *pl =data->playlist;
	return pl->firstkduration(pl,k);
}

void *pickFromPlaylistDataSource(TDataSource *dataSource) {
	void *object, *clone;
	TDataCatalog *dataCatalog = dataSource->datacatalog;
	TFromPlaylistDataCatalog *data = dataCatalog->data;

	TPlaylist *playlist = data->playlist;

	object = playlist->next(playlist);

	while (!object) {
		resetFromPlaylistDataSource(dataSource);
		playlist = data->playlist;
		object = playlist->next(playlist);
	}

	clone = cloneObject(object);

	return clone;
}

void resetFromPlaylistDataSource(TDataSource *dataSource) {
	TPlaylist *pl;
	unsigned int i;
	TDataCatalog *dataCatalog = dataSource->datacatalog;
	TFromPlaylistDataCatalog *data = dataCatalog->data;

	TRandomic *randomic = dataCatalog->dynamic;

	i = randomic->pick(randomic);

	disposePlaylist(data->playlist);

	pl = data->objects[i];

	data->playlist = pl->clone(pl);

}

void* pickForPrefetchDataSource(TDataSource *dataSource){
	TPrefetch *prefetch = dataSource->prefetch;

	return prefetch->dynamic(dataSource);
}

void* pickForPrefetchNothingDataSource(TDataSource *dataSource){
	return NULL;
}

void* pickForPrefetchPlaylistDataSource(TDataSource *dataSource){
	void *object, *clone;
	TDataCatalog *dataCatalog = dataSource->datacatalog;
	TFromPlaylistDataCatalog *data = dataCatalog->data;

	TPlaylist *playlist = data->playlist;

	object = playlist->nextPrefetch(playlist);

	if(object == NULL)
		return NULL;

	clone = cloneObject(object);

	return clone;
}

void* createFromPlaylistDataSource(TDataCatalog *dataCatalog) {
	TDataSource *dataSource;
	TFromPlaylistDataCatalog *data = dataCatalog->data;

	dataSource = (TDataSource*) malloc(sizeof(TDataSource));

	TRandomic *randomic = dataCatalog->dynamic;

	unsigned int i = randomic->pick(randomic);

	data->playlist = clonePlaylist(data->objects[i]);

	dataSource->getPrefetchRate = getPrefetchRateDataSource;
	dataSource->datacatalog = dataCatalog;
	dataSource->pick = pickFromPlaylistDataSource;
	dataSource->pickForPrefetch = pickForPrefetchDataSource;
	dataSource->reset = resetFromPlaylistDataSource;
	dataSource->size = sizeFromPlaylistDataSource;
	dataSource->duration = durationFromPlaylistDataSource;
	dataSource->firstkduration = firstkDurationFromPlaylistDataSource;

	return dataSource;
}


//
//
int sizeFromCollectionDataSource(TDataSource *dataSource){
	return 0;
}

int durationFromCollectionDataSource(TDataSource *dataSource){
	return 0;
}

int firstkDurationFromCollectionDataSource(TDataSource *dataSource, int k){
	return 0;
}

void resetFromCollectionDataSource(TDataSource *dataSource){

}


//general data source
void disposeDataSource(TDataSource *dataSource) {
	if (dataSource) {
		TDataCatalog *dataCatalog = dataSource->datacatalog;
		if (dataCatalog)
			dataCatalog->dispose(dataCatalog);
		free(dataSource);
	}
}

void *pickDataSource(TDataSource *dataSource) {

	return dataSource->pick(dataSource);

}

void resetDataSource(TDataSource *dataSource) {

	if (dataSource->reset)
		dataSource->reset(dataSource);

}


void* createFromCollectionDataSource(TDataCatalog *dataCatalog) {
	TDataSource *dataSource;

	dataSource = (TDataSource*) malloc(sizeof(TDataSource));

	dataSource->datacatalog = dataCatalog;

	dataSource->getPrefetchRate = getPrefetchRateDataSource;
	dataSource->pick = pickFromCollectionDataSource;
	dataSource->pickForPrefetch = pickForPrefetchDataSource;
	dataSource->reset = resetFromCollectionDataSource;
	dataSource->size = sizeFromCollectionDataSource;
	dataSource->duration = durationFromCollectionDataSource;//@ Criar tamanho em Bytes da colecao ?
	dataSource->firstkduration = firstkDurationFromCollectionDataSource;

	return dataSource;
}


void *createPrefetchNone(void *pars) {
	TPrefetch *prefetch;

	prefetch = (TPrefetch*) malloc(sizeof(TPrefetch));

	prefetch->dynamic = pickForPrefetchNothingDataSource;
	prefetch->fraction = 0.f;

	return prefetch;
}


void *createPrefetchNextFromPlaylist(char *pars) {
	TPrefetch *prefetch;
	TParameters *lp = createParameters(pars, PARAMETERS_SEPARATOR);

	lp->iterator(lp);

	prefetch = (TPrefetch*) malloc(sizeof(TPrefetch));

	prefetch->dynamic = pickForPrefetchPlaylistDataSource;
	prefetch->fraction = atof(lp->next(lp));

	lp->dispose(lp);

	return prefetch;
}
