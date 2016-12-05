/*
 * object.c
 *
 *  Created on: Jul 2, 2012
 *      Author: cesar
 */
#include "stdio.h"
#include "time.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "system.h"
#include "object.h"

typedef struct _data_object TDataObject;

struct _data_object {
	TIdObject id; // (YOUTUBEID/char)
	float length; // in seconds or milliseconds
	int lengthBytes;//in Bytes
	float stored; //in seconds
	int replicado; // marca conteudo replicado na cache
	float bitRate; // Overall bit Rate |mode:variable
	int lPopularity; // local Popularity used for LFU
	float accessFrequency; //
	float rating;
	unsigned long int lastAccess;
	float cumulativeValue;
	float normalizedByteServed;
	char upload[26]; // upload data

	//float bitRate;
};

//Object related implementations
struct object {
	void *data;
};

void** newCatalogObject(int size) {
	return (void**) malloc(sizeof(TObject*) * size);
}

void disposeCatalogObject(TObject** catalog, int size) {
	unsigned int i;
	for (i = 0; i < size; i++)
		free(catalog[i]);
}

TObject* initObject(TIdObject id, float length,int lengthBytes, int bitRate, int lPopularity) {

	TDataObject *dataObj = malloc(sizeof(TDataObject));
	TObject *object = (TObject *) malloc(sizeof(TObject));

	if (object == NULL ) {
		printf("PANIC: Run Out of Memory on Init Object\n");
		exit(0);
	}
	strcpy(dataObj->id, id);
	dataObj->length = length; //duration
	dataObj->lengthBytes = lengthBytes;
	dataObj->stored = length;
	dataObj->replicado = 0;
	dataObj->bitRate = bitRate; //Overall bit rate |mode: variable
	dataObj->lPopularity = lPopularity;
	dataObj->rating = 0.0;
	dataObj->cumulativeValue = 0.0;
	dataObj->lastAccess = 0;
	dataObj->accessFrequency = 0;
	dataObj->cumulativeValue = 0;
	dataObj->normalizedByteServed = 0;
	//dataObj->bitRate = gPopularity; //Overall bit rate |mode: variable

	object->data = dataObj;

	return object;
}

void copyObject(TObject *src, TObject *dest) {
	TDataObject *dataSrc = src->data;
	TDataObject *dataDest = dest->data;

	strcpy(dataDest->id, dataSrc->id);
	dataDest->length = dataSrc->length;
	dataDest->lengthBytes = dataSrc->lengthBytes;
	dataDest->stored = dataSrc->stored;
	dataDest->replicado = dataSrc->replicado;
	dataDest->bitRate = dataSrc->bitRate;
	dataDest->lPopularity = dataSrc->lPopularity;
	dataDest->accessFrequency = dataSrc->accessFrequency;
	dataDest->lastAccess = dataSrc->lastAccess;
	dataDest->cumulativeValue = dataSrc->cumulativeValue;
	dataDest->normalizedByteServed = dataSrc->normalizedByteServed;
	strcpy(dataDest->upload, dataSrc->upload);
	dataDest->rating = dataSrc->rating;
	dataDest->bitRate = dataSrc->bitRate;

}

TObject* cloneObject(TObject *p) {

	TObject *clone = (TObject *) malloc(sizeof(TObject));
	TDataObject *dataClone = malloc(sizeof(TDataObject));

	if ((clone == NULL )|| (dataClone == NULL)){
		printf("PANIC: object.c: cloneObject(): Run Out of Memory on clone Object\n" );
		exit(0);
	}

	clone->data = dataClone;
	copyObject(p, clone);

	return clone;
}

void showObject(TObject *p) {
	TDataObject *data = p->data;

	printf("%s ", data->id);
	printf("%f ", data->length);
	printf("%d ", data->lengthBytes);
	printf("%f ", data->stored);
	printf("%d ", data->replicado);
	printf("%d ", data->bitRate);
	printf("%d ", data->lPopularity);
	printf("%f ", data->accessFrequency);
	printf("%lu ", data->lastAccess);
	printf("%f ", data->cumulativeValue);
	printf("%f ", data->normalizedByteServed);
	printf("%f ", data->bitRate);
	printf("\n");
}

void disposeObject(TObject *object) {
	if (object != NULL )
		free(object->data);
	free(object);
}

short storedAsCriteriaObject(void* onListObject, void* newObject) {

	return (getStoredObject(onListObject) < getStoredObject(newObject));

}

short lpopularityAsCriteriaObject(void* onListObject, void* newObject) {

	return (getLPopularityObject(onListObject) < getLPopularityObject(newObject));

}

short cumulativeValueAsCriteriaObject(void* onListObject, void* newObject) {
	return (getCumulativeValueObject(onListObject)
			>= getCumulativeValueObject(newObject));
}

float getRatingObject(TObject *object) {
	TDataObject *data = object->data;
	return data->rating;
}

void setRatingObject(TObject *object, float rating) {
	TDataObject *data = object->data;
	data->rating = rating;
}

char* getUploadObject(TObject* object) {
	TDataObject *data = object->data;
	return data->upload;
}

void setUploadObject(TObject * object, char* upload) {
	TDataObject *data = object->data;
	strcpy(data->upload, upload);
}

float getLengthObject(TObject *object) {
	TDataObject *data = object->data;
	return data->length;
}

int getLengthBytesObject(TObject *object) {
	TDataObject *data = object->data;
	return data->lengthBytes;
}

int getStoredObject(TObject *object) {
	TDataObject *data = object->data;
	return data->stored;
}

int getReplicateObject(TObject *object) {
	TDataObject *data = object->data;
	return data->replicado;
}
int getGPopularityObject(TObject *object) {
	TDataObject *data = object->data;
	return data->bitRate;
}

int getLPopularityObject(TObject *object) {
	TDataObject *data = object->data;
	return data->lPopularity;
}

unsigned long int getLastAccessObject(TObject *object) {
	TDataObject *data = object->data;
	return data->lastAccess;
}

float getAccessFrequencyObject(TObject *object) {
	TDataObject *data = object->data;
	return data->accessFrequency;
}

float getCumulativeValueObject(TObject *object) {
	TDataObject *data = object->data;
	return data->cumulativeValue;
}

float getNormalizedByteServedObject(TObject *object) {
	TDataObject *data = object->data;
	return data->normalizedByteServed;
}

void getIdObject(TObject *object, TIdObject id) {
	TDataObject *data = object->data;
	strcpy(id, data->id);
}

float getBitRateObject(TObject *object) {
	TDataObject *data = object->data;
	return data->bitRate;
}


//static TObject* getPrevObject(TObject* object) {
//	TDataObject *data = object->data;
//	return NULL;//data->prev;
//}

void setLPopularityObject(TObject *object, int lPopularity) {
	TDataObject *data = object->data;
	data->lPopularity = lPopularity;
}

void setCumulativeValueObject(TObject *object, float cumulativeValue) {
	TDataObject *data = object->data;
	data->cumulativeValue = cumulativeValue;
}

void setNormalizedByteServedObject(TObject *object, float normalizedByteServed) {
	TDataObject *data = object->data;
	data->normalizedByteServed = normalizedByteServed;
}

void setLastAccessObject(TObject *object, unsigned long int lastAccess) {
	TDataObject *data = object->data;
	data->lastAccess = lastAccess;
}

void setAccessFrequencyObject(TObject *object, float accessFrequency) {
	TDataObject *data = object->data;
	data->accessFrequency = accessFrequency;
}

void setStoredObject(TObject *object, float stored) {
	TDataObject *data = object->data;
	data->stored = stored;
}

void setBitRateObject(TObject *object, float bitRate) {
	TDataObject *data = object->data;
	data->bitRate = bitRate;
}

void addStoredObject(TObject *object, int quantum) {
	TDataObject *data = object->data;
	data->stored += quantum;

	if (data->stored > data->length) {
		printf("Error: AddStoredObject:Stored > length: %f %f\n", data->stored,
				data->length);
		data->stored = data->length;
	}
}

void addLPopularityObject(TObject *object) {
	TDataObject *data = object->data;
	data->lPopularity++;
}

void addNormalizedByteServedObject(TObject *object, int quantum) {
	TDataObject *data = object->data;
	data->normalizedByteServed += quantum;
}

short isPopularObject(TObject *object) {
	TDataObject *data = object->data;
	return (data->bitRate > 0);
}

short isEqualObject(TObject *first, TObject *second) {
	TDataObject *dataFirst = first->data;
	TDataObject *dataSecond = second->data;
	return (strcmp(dataFirst->id, dataSecond->id) == 0);
}

short isReplicatedObject(TObject *object) {
	TDataObject *data = object->data;
	return data->replicado;
}

void setReplicateObject(TObject *video,short replicado){

TDataObject *data = video->data;
data->replicado=replicado;
}
//
//
// List Object related implementation
typedef struct elemListObject TElemListObject;

struct elemListObject {
	void *object;
	TElemListObject *next;
	TElemListObject *prev;
};

typedef struct _dataListObject TDataListObject;
struct _dataListObject{
	TElemListObject *head;
	TElemListObject *tail;
	int holding;
};



static void *getHeadListObject(TListObject* listObject) {
	TDataListObject *data = listObject->data;
	if (data->head)
		return data->head->object;
	else
		return NULL;
}

static void *getTailListObject(TListObject* listObject) {
	TDataListObject *data = listObject->data;
	if (data->tail)
		return data->tail->object;
	else
		return NULL;
}

static int getHoldingListObject(TListObject* listObject) {
	TDataListObject *data = listObject->data;

	return data->holding;
}

static short isEmptyListObject(TListObject *listObject) {
	TDataListObject *data = listObject->data;

	return (data->head == NULL );
}

static void removeTailListObject(TListObject *listObject) {
	TElemListObject *walker;
	TDataListObject *dataListObject = listObject->data;

	// is empty?
	if (dataListObject->tail != NULL ) {
		// is unitary?
		if (dataListObject->head == dataListObject->tail) {
			disposeObject(dataListObject->head->object);
			free(dataListObject->head);
			dataListObject->head = dataListObject->tail = NULL;
		} else {
			// setting list members
			walker = dataListObject->tail->prev;
			walker->next = NULL;

			disposeObject(dataListObject->tail->object);
			free(dataListObject->tail);
			dataListObject->tail = walker; //object tail
		}
		dataListObject->holding--;
	}
}

static void removeHeadListObject(TListObject* listObject) {
	TElemListObject *walker;

	TDataListObject *dataListObject = listObject->data;
	// is listObject empty?
	if (dataListObject->head != NULL ) {
		walker = dataListObject->head;

		// set the new head
		dataListObject->head = dataListObject->head->next;
		if (dataListObject->head == NULL ) // going to be empty?
			dataListObject->tail = NULL;
		else
			dataListObject->head->prev = NULL;
		disposeObject(walker->object);
		free(walker);

		dataListObject->holding--;

	}
}

static void removeListObject(TListObject* listObject, void* disposable) {
	TElemListObject *walk;
	TDataListObject *dataListObject = listObject->data;

	if ((disposable == NULL )|| (dataListObject->head==NULL) )return;

	walk = dataListObject->head;
	while ( (walk != NULL ) && (!isEqualObject(walk->object, disposable)) ){
		walk = walk->next;
	}

	if (walk != NULL ) { // is listObject empty?
		if (dataListObject->head == dataListObject->tail)
			dataListObject->head = dataListObject->tail = NULL;
		else if (dataListObject->head == walk) {
			dataListObject->head = walk->next;
			walk->next->prev = NULL;
		} else if (dataListObject->tail == walk) {
			dataListObject->tail = walk->prev;
			walk->prev->next = NULL;
		} else {
			walk->prev->next = walk->next;
			walk->next->prev = walk->prev;
		}

		disposeObject(walk->object);
		free(walk);

		dataListObject->holding--;
	}

}


/*static void removeRepListObject(TListObject* listObject) {
	TElemListObject *walk,*aux,*ant;
	TDataListObject *dataListObject = listObject->data;

	if ((dataListObject->head==NULL) )return;

	aux = dataListObject->head;

	while ( (aux != NULL) ){
		walk=aux;
		while ( (walk != NULL ) && (!isReplicatedObject(walk->object) )){
			//ant=walk;
			walk = walk->next;
		}
		aux=walk;
		if (walk != NULL ) { // is listObject empty?

			//tratar remocao do unico elemento
			if (dataListObject->head == dataListObject->tail)
				dataListObject->head = dataListObject->tail = NULL;

			//tratar remocao no comeco
			else if (dataListObject->head == walk) {
				dataListObject->head = walk->next;
				walk->next->prev = NULL;
				aux = dataListObject->head;

				//tratar remocao no final
			} else if (dataListObject->tail == walk) {
				dataListObject->tail = walk->prev;
				walk->prev->next = NULL;
				aux=NULL;

				//tratar remocao intermediaria
			} else {
				aux=walk->next;
				walk->prev->next = walk->next;
				walk->next->prev = walk->prev;
				//aux=ant->next;
			}

			disposeObject(walk->object);
			free(walk);
			dataListObject->holding--;
		}


	}

}*/

static void removeSoftListObject(TListObject* listObject, void* disposable){
	TElemListObject *walk;
	TDataListObject *dataListObject = listObject->data;

	if (dataListObject->head!=NULL) {
		walk = dataListObject->head;
		while((walk) && (!isEqualObject(disposable,walk->object)) )
				walk = walk->next;
		if (walk != NULL){// is listObject empty?
			if (dataListObject->head == dataListObject->tail)
				dataListObject->head = dataListObject->tail = NULL;
			else if (dataListObject->head == walk){
				dataListObject->head = walk->next;
				walk->next->prev = NULL;
			}else if (dataListObject->tail == walk){
				dataListObject->tail = walk->prev;
				walk->prev->next = NULL;
			}else{
				walk->prev->next = walk->next;
				walk->next->prev = walk->prev;
			}
			dataListObject->holding--;
		}

	}
}

static void insertHeadListObject(TListObject *listObject, void *newComer) {

	TElemListObject *elem = malloc(sizeof(TElemListObject));
	TDataListObject *dataListObject = listObject->data;

	elem->next = dataListObject->head;
	elem->prev = NULL;
	elem->object = newComer;

	if (dataListObject->head == NULL )
		dataListObject->tail = elem;
	else
		dataListObject->head->prev = elem;

	dataListObject->head = elem;

	dataListObject->holding++;

}

static void insertTailListObject(TListObject *listObject, void *newComer) {

	TElemListObject *elem = malloc(sizeof(TElemListObject));
	TDataListObject *dataListObject = listObject->data;

	elem->prev = dataListObject->tail;
	elem->object = newComer;

	if (dataListObject->head == NULL )
		dataListObject->head = elem;
	else
		dataListObject->tail->next = elem;

	dataListObject->tail = elem;

	dataListObject->holding++;

}

static void insertOrdListObject(TListObject *listObject, void *newComer,
		TCriteriaInsertOrdListObject criteriaInsOrdListObject) {
	TElemListObject *walker;
	TElemListObject *elem = malloc(sizeof(TElemListObject));
	TDataListObject *dataListObject = listObject->data;

	walker = dataListObject->head;

	elem->next = NULL;
	elem->prev = NULL;
	elem->object = newComer;

	if (walker == NULL ) {
		dataListObject->tail = elem;
		dataListObject->head = elem;
	} else {
		while (criteriaInsOrdListObject(walker->object, newComer)) {
			walker = walker->next;
			if (walker == NULL )
				break;
		}

		if (walker == NULL ) {
			dataListObject->tail->next = elem;
			elem->prev = dataListObject->tail;
			dataListObject->tail = elem;
		} else if (walker->prev == NULL ) {
			dataListObject->head->prev = elem;
			elem->next = dataListObject->head;
			dataListObject->head = elem;
		} else {
			elem->next = walker;
			elem->prev = walker->prev;
			walker->prev->next = elem;
			walker->prev = elem;
		}

	}

	dataListObject->holding++;

}

static void cleanupListObject(TListObject *listObject) {
	TElemListObject *walker;
	TDataListObject *dataListObject = listObject->data;

	if (dataListObject->head != NULL ) {
		do {
			walker = dataListObject->head;
			dataListObject->head = walker->next;
			disposeObject(walker->object);
			free(walker);
		} while (dataListObject->head != NULL );

		dataListObject->tail = NULL;
		//free(listObject);

		dataListObject->holding=0;

	}
}

static void destroyListObject(TListObject *listObject) {

	//free the objects into list of objects
	cleanupListObject(listObject);

	//also free the list of objects
	free(listObject->data);
	free(listObject);

}

static void showListObject(TListObject *listObject) {
	TElemListObject *walker;
	TDataListObject *dataListObject = listObject->data;

	walker = dataListObject->head;

	printf("\n");
	while (walker != NULL ) {

		showObject(walker->object);

		walker = walker->next;
		printf("\n");
	}

}


/*
 * static void removeRepListObject(TListObject *listObject) {
	TElemListObject *walker;
	TDataListObject *dataListObject = listObject->data;

	walker = dataListObject->head;


	while (walker != NULL ) {

		//Funcao para remover objeto replicado
		if(isReplicatedObject()==1){

		}
		walker = walker->next;

	}

}
*/

static short hasListObject(TListObject *listObject, void *object) {
	TElemListObject *walk;
	TDataListObject *dataListObject = listObject->data;
	short found = 0;

	walk = dataListObject->head;

	while (walk != NULL && !found) {
		if (isEqualObject(walk->object, object))
			found = 1;
		else
			walk = walk->next;
	}

	return found;
}

static void* getNextListObject(TListObject *listObject, void *object) {
	static TElemListObject *last=NULL;
	TDataListObject *dataListObject = listObject->data;

	TObject *obj;

	if ( (object == NULL) && (dataListObject->head != NULL) ) {
		last = dataListObject->head;
		obj = dataListObject->head->object;
	} else if ((last!=NULL) && (last->next != NULL) ){
		obj = last->next->object;
		last = last->next;
	}else{
		obj = NULL;
	}

	return obj;

}

static void* getLFUListObject(TListObject *listObject) {
	TElemListObject *walk;
	TElemListObject *lfu;
	TDataListObject *dataListObject = listObject->data;

	walk = dataListObject->head;
	lfu = walk;

	if (walk != NULL )
		walk = walk->next;

	while (walk != NULL ) {
		if (getLPopularityObject(walk->object) < getLPopularityObject(lfu->object))
			lfu = walk;
		walk = walk->next;
	}
	return lfu->object;
}

static void* getNotLessThanMFUListObject(TListObject* listObject, void* object) {
	TElemListObject *walk;
	TObject *lfu;
	TDataListObject *dataListObject = listObject->data;

	walk = dataListObject->head;
	lfu = getLFUListObject(listObject);

	while (walk != NULL ) {
		if ( (getLPopularityObject(object) > getLPopularityObject(walk->object))
				&& (getLPopularityObject(walk->object) > getLPopularityObject(lfu)) )
			lfu = walk->object;
		walk = walk->next;
	}
	return ((lfu != object ? lfu : NULL ));
}

static void* getMFUListObject(TListObject *listObject) {
	TElemListObject *walk;
	TElemListObject *lfu;
	TDataListObject *dataListObject = listObject->data;

	walk = dataListObject->head;
	lfu = walk;

	if (walk != NULL )
		walk = walk->next;

	while (walk != NULL ) {
		if (getLPopularityObject(walk->object) > getLPopularityObject(lfu->object))
			lfu = walk;
		walk = walk->next;
	}
	return lfu->object;
}

//Most Valuable Object
static void* getMVOListObject(TListObject *listObject) {
	TElemListObject *walk;
	TElemListObject *mvo;
	TDataListObject *dataListObject = listObject->data;

	mvo = dataListObject->head;
	walk = mvo->next;

	while (walk != NULL ) {
		if (getNormalizedByteServedObject(walk->object) > getNormalizedByteServedObject(mvo->object))
			mvo = walk;
		walk = walk->next;
	}
	return mvo->object;
}

static void* getNotLessThanMVOListObject(TListObject* listObject, void* object) {
	TElemListObject *walk;
	TObject *MaxCValue;
	TDataListObject *dataListObject = listObject->data;

	walk = dataListObject->head;
	MaxCValue = getMVOListObject(listObject);

	while (walk != NULL ) {
		if ((getNormalizedByteServedObject(object) <= getNormalizedByteServedObject(walk->object))
				&& (getNormalizedByteServedObject(walk->object) < getNormalizedByteServedObject(MaxCValue)) )
			MaxCValue = walk->object;
		walk = walk->next;
	}
	return ((MaxCValue != object ? MaxCValue : NULL ));
}

//Less Valuable Object
static void* getLVOListObject(TListObject *listObject, void* obj) {
	TElemListObject *walk;
	TElemListObject *lvo;
	TDataListObject *dataListObject = listObject->data;

	lvo = dataListObject->head;
	if ((TObject*)lvo->object == obj)
		lvo = lvo->next;
	walk = lvo;

	while (walk != NULL ) {
		if (getNormalizedByteServedObject(walk->object) < getNormalizedByteServedObject(lvo->object)) {
			if ((TObject*)walk->object != obj)
				lvo = walk;
		}
		walk = walk->next;
	}
	return lvo->object;
}


//mean Stored Object size
int getMeanObjectSizeListObject(TListObject *listObject) {
	TElemListObject *walk;
	int meanStoredSize = 0;
	int stored = 0;
	TDataListObject *dataListObject = listObject->data;

	walk = dataListObject->head;

	while (walk != NULL ) {
		stored++;
		meanStoredSize += getStoredObject(walk->object);
		walk = walk->next;
	}
	return (int) (meanStoredSize / stored);
}

static void* getMinimumCumulativeValueListObject(TListObject* listObject) {
	TElemListObject *walk;
	TElemListObject *MCValue;
	TDataListObject *dataListObject = listObject->data;

	walk = dataListObject->head;
	MCValue = walk;
	if (walk != NULL )
		walk = walk->next;

	while (walk != NULL ) {
		if (getCumulativeValueObject(walk->object) < getCumulativeValueObject(MCValue->object))
			MCValue = walk;
		walk = walk->next;
	}
	return MCValue->object;
}

static void* getMaximumCumulativeValueListObject(TListObject* listObject) {
	TElemListObject *walk;
	TElemListObject *MaxCValue;
	TDataListObject *dataListObject = listObject->data;

	walk = dataListObject->head;
	MaxCValue = walk;
	if (walk != NULL )
		walk = walk->next;

	while (walk != NULL ) {
		if (getCumulativeValueObject(walk->object) > getCumulativeValueObject(MaxCValue->object))
			MaxCValue = walk;
		walk = walk->next;
	}
	return MaxCValue->object;
}

static void* getMaximumGPopularityListObject(TListObject* listObject) {
	TElemListObject *walk, *MaxGPop;
	TDataListObject *dataListObject = listObject->data;

	walk = dataListObject->head;
	MaxGPop = walk;

	while (walk != NULL ) {
		//		if ( (MaxGPop->gPopularity+MaxGPop->lPopularity) > (walk->gPopularity+walk->gPopularity) )
		if (getLPopularityObject(MaxGPop->object) > getLPopularityObject(walk->object))
			MaxGPop = walk;
		walk = walk->next;
	}
	return MaxGPop->object;
}

static void* getMinimumGPopularityListObject(TListObject* listObject) {
	TElemListObject *walk, *MinGPop;
	TDataListObject *dataListObject = listObject->data;

	walk = dataListObject->head;
	MinGPop = walk;

	while (walk != NULL ) {
		//if ( (MinGPop->gPopularity+MinGPop->lPopularity) > (walk->gPopularity+walk->lPopularity) )
		if ( getLPopularityObject(MinGPop->object) > getLPopularityObject(walk->object) )
			MinGPop = walk;
		walk = walk->next;
	}
	return MinGPop->object;
}

static void* getNotLessThanGPopularityListObject(TListObject* listObject, void* object) {
	TElemListObject *walk;
	TObject *MinGPop;
	TDataListObject *dataListObject = listObject->data;

	walk = dataListObject->head;
	MinGPop = getMinimumGPopularityListObject(listObject);

	while (walk != NULL ) {
		if ( ( getLPopularityObject(object) > getLPopularityObject(walk->object) )
		  && ( getLPopularityObject(MinGPop) >= getLPopularityObject(walk->object) )
		  && ( MinGPop != (TObject*)walk->object) )
			MinGPop = walk->object;
		walk = walk->next;
	}
	return ((MinGPop != object ? MinGPop : NULL ));
}



static void* getNotLessThanCumulativeValueListObject(TListObject* listObject, void* object) {
	TElemListObject *walk;
	TObject *MinCValue;
	TDataListObject *dataListObject = listObject->data;

	walk = dataListObject->head;
	MinCValue = getMinimumCumulativeValueListObject(listObject);

	while (walk != NULL ) {
		if ( (getCumulativeValueObject(object) > getCumulativeValueObject(walk->object) )
				&& (getCumulativeValueObject(walk->object) >= getCumulativeValueObject(MinCValue))
				&& (MinCValue != (TObject*)walk->object))
			MinCValue = walk->object;
		walk = walk->next;
	}
	return ((MinCValue != object ? MinCValue : NULL ));
}

static void* getObjectListObject(TListObject *listObject, void* object) {
	TElemListObject *walk;
	short found = 0;
	TDataListObject *dataListObject = listObject->data;

	walk = dataListObject->head;

	while (walk != NULL && !found) {
		if ( isEqualObject(walk->object, object) )
			found = 1;
		else
			walk = walk->next;
	}

	return (found?walk->object:NULL);
}

static short setNewHeadListObject(TListObject *listObject, void* object) {
	TElemListObject *walk;
	short found = 0;
	TDataListObject *dataListObject = listObject->data;

	walk = dataListObject->head;//verificar tipo

	while ( (walk != NULL) && (!found) ) {
		if ( isEqualObject(walk->object, object) )
			found = 1;
		else
			walk = walk->next;
	}

	if ( (found) && (walk != dataListObject->head)) {
		if (walk == dataListObject->tail) { // is it the tail?
			walk->prev->next = NULL;
			dataListObject->tail = walk->prev;

		} else { // it is something in the middle
			walk->prev->next = walk->next;
			walk->next->prev = walk->prev;
		}

		walk->next = dataListObject->head;
		walk->prev = NULL;
		dataListObject->head->prev = walk;
		dataListObject->head = walk;

	}

	return found;
}


TListObject *createListObject() {

	TListObject *listObject = malloc(sizeof(TListObject));
	TDataListObject *data = malloc(sizeof(TDataListObject));

	data->head = NULL;
	data->tail = NULL;
	data->holding = 0;

	listObject->data = data;

	listObject->remove = removeListObject;
	listObject->getObject = getObjectListObject;
	listObject->getNext = getNextListObject;
	listObject->getHead = getHeadListObject;
	listObject->getTail = getTailListObject;
	listObject->getLFU = getLFUListObject;
	listObject->getMFU = getMFUListObject;
	listObject->getNotLessThanMFU = getNotLessThanMFUListObject;

	listObject->getMinimumCumulativeValue = getMinimumCumulativeValueListObject;
	listObject->getMaximumCumulativeValue = getMaximumCumulativeValueListObject;

	listObject->getLVO = getLVOListObject;
	listObject->getMVO = getMVOListObject;

	listObject->getNotLessThanMVO = getNotLessThanMVOListObject;
	listObject->getNotLessThanCumulativeValue = getNotLessThanCumulativeValueListObject;

	listObject->getMinimumGPopularity = getMinimumGPopularityListObject;
	listObject->getMaximumGPopularity = getMaximumGPopularityListObject;
	listObject->getNotLessThanGPopularity = getNotLessThanGPopularityListObject;


	listObject->getMeanObjectSize = getMeanObjectSizeListObject;
	listObject->getHolding = getHoldingListObject;

	listObject->removeTail = removeTailListObject;
	listObject->insertTail = insertTailListObject;
	listObject->removeHead = removeHeadListObject;
	listObject->removeSoft = removeSoftListObject;
	listObject->insertHead = insertHeadListObject;
	listObject->insertOrd = insertOrdListObject;
	listObject->cleanup = cleanupListObject;
	listObject->destroy = destroyListObject;
	listObject->show = showListObject;
	//listObject->removeRep = removeRepListObject;

	listObject->hasInto = hasListObject;
	listObject->isEmpty = isEmptyListObject;
	listObject->setNewHead = setNewHeadListObject;

	return listObject;
}

typedef struct _data_iteratorListObject TDataIteratorListObject;


struct _data_iteratorListObject{
	TElemListObject *head;
	TElemListObject *current;
	TElemListObject *tail;
};


static void *currentIteratorListObject(TIteratorListObject *it){
	TDataIteratorListObject *data = it->data;

	if (data->current!=NULL)
		return data->current->object;
	else
		return NULL;
}

static void resetIteratorListObject(TIteratorListObject *it){
	TDataIteratorListObject *data = it->data;

	data->current = data->head;

}

static void nextIteratorListObject(TIteratorListObject *it){
	TDataIteratorListObject *data = it->data;
	if (data->current != NULL)
		data->current = data->current->next;
}


static short hasNextIteratorListObject(TIteratorListObject *it){
	TDataIteratorListObject *data = it->data;
	if (data->current == NULL)
		return 0;
	else if(data->current->next!=NULL)
		return 1;
	else
		return 0;
}

void ufreeIteratorListObject(TIteratorListObject *it){
	TDataIteratorListObject *data = it->data;
	free(data);
	free(it);
}

TIteratorListObject* createIteratorListObject(TListObject *l){
	TDataIteratorListObject *data = malloc(sizeof(TDataIteratorListObject));
	TIteratorListObject *it = malloc(sizeof(TIteratorListObject));
	data->head = data->current= data->tail = NULL;

	TDataListObject *dataLO = l->data;
	data->head = dataLO->head;
	data->tail = dataLO->tail;
	data->current = dataLO->head;

	it->data = data;
	it->current = currentIteratorListObject;
	it->hasNext = hasNextIteratorListObject;
	it->reset = resetIteratorListObject;
	it->next = nextIteratorListObject;
	it->ufree = ufreeIteratorListObject;

	return it;
}

