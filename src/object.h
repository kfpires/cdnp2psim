/*
 * object.h
 *
 *  Created on: Jul 2, 2012
 *      Author: cesar
 */

#ifndef OBJECT_H_
#define OBJECT_H_

#define IDMAXLENGTH_OBJECT 40

typedef char TIdObject  [IDMAXLENGTH_OBJECT];

typedef struct object TObject;
//typedef struct _data_object TDataObject;//@ definido no cabeçalho pra poder ser chamado na cache

void** newCatalogObject(int size);
void disposeCatalogObject(TObject** catalog, int size);

TObject* initObject(char *id, float lentgh,int lengthBytes, int gPopularity, int lPopularity);
//TObject* initObject(TIdObject id, float length,int lengthBytes, int gPopularity, int lPopularity);
TObject* cloneObject(TObject *object);
float getLengthObject(TObject *object);
int getStoredObject(TObject *object);
int getReplicadoObject(TObject *object);
int getGPopularityObject(TObject *object);
int getLPopularityObject(TObject *object);
float getCumulativeValueObject(TObject *object);
unsigned long int getLastAccessObject(TObject *object);
float getAccessFrequencyObject(TObject *object);
void getIdObject(TObject *object, char* id);
float getRatingObject(TObject *object);
char* getUploadObject(TObject * object);
float getNormalizedByteServedObject(TObject *object);
float getBitRateObject(TObject *object);
//TObject* getNextObject(TObject *object);
//TObject* getPrevObject(TObject *object);

void setLastAccessObject(TObject *object, unsigned long int lastAccess);
void setAccessFrequencyObject(TObject *object, float accessFrequency);
void setLPopularityObject(TObject *object, int lpopularity);
void setCumulativeValueObject(TObject *object, float cumulativeValue);
void setStoredObject(TObject *object, float stored);
void setNormalizedByteServedObject(TObject *object, float normalizedByteServed);
void setRatingObject(TObject *object, float rating);
void setUploadObject(TObject * object, char* upload);
void setBitRateObject(TObject *object, float bitRate);
void setReplicateObject(TObject *video,short replicado);

void addStoredObject(TObject *object, int quantum);
void addNormalizedByteServedObject(TObject *object, int quantum);
void addLPopularityObject(TObject *object);
short isPopularObject(TObject *object);

void showObject(TObject *object);
void disposeObject(TObject *object);
short isEqualObject(TObject *first, TObject *second);
short isReplicatedObject(TObject *object);

short lpopularityAsCriteriaObject(void* oldObject, void* newObject);
short storedAsCriteriaObject(void* onListObject, void* newObject);
short cumulativeValueAsCriteriaObject(void *onListObject, void *newObject);

//List Object related definitions
typedef struct listObject TListObject;
typedef short (* TCriteriaInsertOrdListObject)(void*, void*);


TListObject *createListObject();

typedef void  (*TRemoveListObject)(TListObject* listObject, void* object);
typedef void* (*TGetNextListObject)(TListObject *listObject, void* object);
typedef void* (*TGetObjectListObject)(TListObject *listObject, void* object);
typedef void* (*TGetHeadListObject)(TListObject* listObject);
typedef void* (*TGetTailListObject)(TListObject* listObject);
typedef void* (*TGetLFUListObject)(TListObject* listObject);
typedef void* (*TGetMFUListObject)(TListObject *listObject);
typedef void* (*TGetNotLessThanMFUListObject)(TListObject* listObject, void* object );

typedef void* (*TGetMinimumCumulativeValueListObject)(TListObject* listObject);
typedef void* (*TGetMaximumCumulativeValueListObject)(TListObject* listObject);

typedef void* (*TGetLVOListObject)(TListObject *listObject, void* obj);
typedef void* (*TGetMVOListObject)(TListObject *listObject);

typedef void* (*TGetNotLessThanMVOListObject)(TListObject* listObject, void* object );
typedef void* (*TGetNotLessThanCumulativeValueListObject)(TListObject* listObject, void* object );

typedef void* (*TGetMinimumGPopularityListObject)(TListObject* listObject );
typedef void* (*TGetMaximumGPopularityListObject)(TListObject* listObject);
typedef void* (*TGetNotLessThanGPopularityListObject)(TListObject* listObject, void* object );


typedef int (*TGetMeanObjectSizeListObject)(TListObject *listObject);
typedef int (*TGetHoldingListObject)(TListObject* listObject);

typedef void (*TRemoveTailListObject)(TListObject* listObject);
typedef void (*TInsertTailListObject)(TListObject* listObject, void* object);
typedef void (*TRemoveHeadListObject)(TListObject* listObject);
typedef void (*TRemoveSoftListObject)(TListObject* listObject, void* object);
typedef void (*TInsertHeadListObject)(TListObject* listObject, void* object);
typedef void (*TInsertOrdListObject)(TListObject* listObject, void* object, TCriteriaInsertOrdListObject criteriaInsertOrdListObject);
typedef void (*TCleanupListObject)(TListObject *listObject);
typedef void (*TDestroyListObject)(TListObject *listObject);
typedef void (*TShowListObject)(TListObject *listObject);
typedef void (*TRemoveRepListObject)(TListObject *listObject);


typedef short (*THasIntoListObject)(TListObject *listObject, void* object);
typedef short (*TIsEmptyListObject)(TListObject *listObject);
typedef short (*TSetNewHeadListObject)(TListObject *listObject, void* object);


struct listObject {
	void *data;
	TRemoveListObject remove;
	TGetObjectListObject getObject;
	TGetNextListObject getNext;
	TGetHeadListObject getHead;
	TGetTailListObject getTail;
	TGetLFUListObject getLFU;
	TGetMFUListObject getMFU;
	TGetNotLessThanMFUListObject getNotLessThanMFU;

	TGetMinimumCumulativeValueListObject getMinimumCumulativeValue;
	TGetMaximumCumulativeValueListObject getMaximumCumulativeValue;

	TGetLVOListObject getLVO;
	TGetMVOListObject getMVO;

	TGetNotLessThanMVOListObject getNotLessThanMVO;
	TGetNotLessThanCumulativeValueListObject getNotLessThanCumulativeValue;

	TGetMinimumGPopularityListObject getMinimumGPopularity;
	TGetMaximumGPopularityListObject getMaximumGPopularity;
	TGetNotLessThanGPopularityListObject getNotLessThanGPopularity;


	TGetMeanObjectSizeListObject getMeanObjectSize;
	TGetHoldingListObject getHolding;

	TRemoveTailListObject removeTail;
	TInsertTailListObject insertTail;
	TRemoveHeadListObject removeHead;
	TRemoveSoftListObject removeSoft;
	TInsertHeadListObject insertHead;
	TInsertOrdListObject insertOrd;
	TCleanupListObject cleanup;
	TDestroyListObject destroy;
	TShowListObject show;
	TRemoveRepListObject removeRep;

	THasIntoListObject hasInto;
	TIsEmptyListObject isEmpty;
	TSetNewHeadListObject setNewHead;

};

typedef struct iteratorListObject TIteratorListObject;
typedef void *(*TCurrentIteratorListObject)(TIteratorListObject *it);
typedef void (*TResetIteratorListObject)(TIteratorListObject *it);
typedef short (*THasNextIteratorListObject)(TIteratorListObject *it);
typedef void (*TUFreeIteratorListObject)(TIteratorListObject *it);
typedef void (*TNextIteratorListObject)(TIteratorListObject *it);

struct iteratorListObject{
	void *data;
	TCurrentIteratorListObject current;
	TResetIteratorListObject reset;
	THasNextIteratorListObject hasNext;
	TNextIteratorListObject next;
	TUFreeIteratorListObject ufree;
};

TIteratorListObject *createIteratorListObject();

#endif /* OBJECT_H_ */
