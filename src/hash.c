//#include "cache.h"
#include "hierarchy.h"
#include "peer.h"
#include "object.h"
#include "hash.h"

typedef struct _data_ItemHashTable TDataItemHashTable;
struct _data_ItemHashTable {
    int idOwner; // Id for peers
    TPeer *peer;
    TKeyHashTable idObject; //
    TObject *object;
};



struct CollisionHashTable {
  TKeyHashTable idObject; // Object ID
  TKeeperHashTable *keepers; // list of object holder
  TCollisionHashTable* next; // list of objects that share a hash table entry
  TCollisionHashTable* prev; // list of objects that share a hash table entry
};

struct EntryHashTable{
	TCollisionHashTable* head;
	TCollisionHashTable* tail;
};

typedef struct _data_HashTable TDataHashTable;
struct _data_HashTable{
    TEntryHashTable *entries;
    int size;
    int *weight;
    int maxKeyLength;
};





static void setItemHashTable(TItemHashTable *item, int idOwner, void *peer, TKeyHashTable idObject, void *object){

//	item = (TItemHashTable*)malloc(sizeof(TItemHashTable));
	TDataItemHashTable *data = item->data;

	data->idOwner = idOwner;
	data->peer = (TPeer*)peer;

//	item->idObject = (TKeyHash*)malloc(sizeof(TKeyHash)*strlen(idObject)+1);
	strcpy(data->idObject, idObject);
	data->object = object;

}


static void disposeItemHashTable(TItemHashTable *item){
	free(item->data);
	free(item);
}


TItemHashTable *createItemHashTable(){

	TItemHashTable *item = (TItemHashTable *)malloc(sizeof(TItemHashTable));
	TDataItemHashTable *data = malloc(sizeof(TDataItemHashTable));


	data->idOwner = -1;
	data->idObject[0] = '\0';
	data->object = NULL;
	data->peer = NULL;

	item->data = data;

	item->set = setItemHashTable;
	item->dispose = disposeItemHashTable;

	return item;
}

static short emptyEntryHashTable(TEntryHashTable* entry){

	return (entry->head == NULL);
}  

struct KeeperHashTable {
    int idOwner; // Id for peers
    TPeer *peer;
    TObject *object;
    TKeeperHashTable *next;
};


TKeeperHashTable* getNextKeeperHashTable(TKeeperHashTable *keeper){

	return keeper->next;
}

void setNextKeeperHashTable(TKeeperHashTable *keeper, void *next){
	keeper->next = next;
}

int getOwnerKeeperHashTable(TKeeperHashTable *keeper){
	return keeper->idOwner;
}


static void updateKeeperHashTable(TItemHashTable *item, TCollisionHashTable *collision){
    TKeeperHashTable *keeper, *preview;
	TDataItemHashTable *data = item->data;
    int found;

    found = 0;
    keeper = collision->keepers;
    while( (keeper!=NULL) && (!found)){
        if(keeper->idOwner == data->idOwner)
            found = 1;
        else{
            preview = keeper;
            keeper = keeper->next;
        }
    }
    if (!found){
        keeper = createKeeperHashTable(data->idOwner, data->peer, data->object);
//        keeper->idOwner = data->idOwner;
//        keeper->peer = data->peer;
//        keeper->object = data->object;
//        keeper->next = NULL;
        preview->next = keeper;
    }       
}


static void removeKeeperHashTable(TItemHashTable *item, TCollisionHashTable *collision){
    TKeeperHashTable *keeper, *preview;
	TDataItemHashTable *data = item->data;
	int found;

    found = 0;
    preview = NULL;
    keeper = collision->keepers;
    while( (keeper!=NULL) && (!found)){
        if(keeper->idOwner == data->idOwner)
            found = 1;
        else{
            preview = keeper;
            keeper = keeper->next;
        }
    }
    if (found){
    	if (preview == NULL) //removing head
    		collision->keepers = keeper->next;
    	else{
    		preview->next = keeper->next;
    	}
        free(keeper);
    }

}

TKeeperHashTable *createKeeperHashTable(int idOwner, void *peer, void *object){
	TKeeperHashTable *keeper = (TKeeperHashTable *)malloc(sizeof(TKeeperHashTable));

	keeper->idOwner = idOwner;
	keeper->peer = peer;
	keeper->object = object;

	keeper->next = NULL;

	return keeper;
}
static void setWeightHashTable(THashTable* hashTable){  /* -Gera valores randomicos entre 1 e 10.000- */
	int i;
	TDataHashTable *data = hashTable->data;
    int *weight = data->weight;

 	srand((int) 5000);
	for (i = 0; i < data->maxKeyLength; i++)
		weight[i] = (int) (1+(int) (10000.0*rand()/(RAND_MAX+1.0)));
}


static int hashingHashTable(THashTable* hashTable, TKeyHashTable key){
	int i; 
	unsigned int entry = 0; 
	int len = strlen(key);
	TDataHashTable *data = hashTable->data;
	
	for (i = 0; i < len; i++)
		entry += abs((unsigned int)key[i] * data->weight[i]);

    entry = (entry % data->size); // hashing function = division
    
	return entry;
}


static void setHashTable(THashTable* hashTable){
	int i;
	TDataHashTable *data = hashTable->data;

	for (i = 0; i < data->size; i++){
    	data->entries[i].head = NULL;
    	data->entries[i].tail = NULL;
    }
}


static TCollisionHashTable* lookupEntryHashTable(THashTable* hashTable, TKeyHashTable idObject, int *entry){
	/* -- Obs.: Apontador de retorno aponta para o item anterior da lista -- */
    int found;
    
	TCollisionHashTable *walker = NULL;
	TDataHashTable *data = hashTable->data;
	
	*entry = hashingHashTable(hashTable,idObject);
	 
	found = 0;
    walker = data->entries[*entry].head;
	while (	(walker != NULL) && (!found) ) {
		
		if( strcmp(idObject, walker->idObject) == 0 )
		    found = 1;
		else	    
		    walker = walker->next;
    }
	
	return walker;
}  


static void* lookupHashTable(THashTable* hashTable, TKeyHashTable idObject){
	/* -- Obs.: Apontador de retorno aponta para o item anterior da lista -- */
    int found;
    int ent;
	TDataHashTable *data = hashTable->data;

	TCollisionHashTable *walker = NULL;

	ent = hashingHashTable(hashTable, idObject);

	found = 0;
    walker = data->entries[ent].head;
	while (	(walker != NULL) && (!found) ) {

		if( strcmp(idObject, walker->idObject) == 0 )
		    found = 1;
		else
		    walker = walker->next;
    }
	if (found)
		return walker->keepers;
	else
		return  NULL;
}


static void insertCollisionHashTable(TItemHashTable* item, THashTable* hashTable, int entry){

        TCollisionHashTable *collision;
        TKeeperHashTable *keeper;
    	TDataHashTable *data = hashTable->data;
    	TDataItemHashTable *dataItem = item->data;

    	keeper = createKeeperHashTable(dataItem->idOwner,dataItem->peer,dataItem->object);

        collision = (TCollisionHashTable *)malloc(sizeof(TCollisionHashTable));
        collision->next = NULL;
        collision->prev = NULL;
        
        strcpy(collision->idObject, dataItem->idObject);
        collision->keepers = keeper;
        
        if(data->entries[entry].head == NULL){
           	data->entries[entry].tail = collision;
    	    data->entries[entry].head = collision;
        }else{	
	        collision->prev = data->entries[entry].tail;
	        data->entries[entry].tail->next = collision;
    	    data->entries[entry].tail = collision;
        }  
}


static void insertHashTable(THashTable* hashTable, TItemHashTable* item){
	TCollisionHashTable *collision;
	TDataItemHashTable *dataItem = item->data;
	int entry;
	
    collision = lookupEntryHashTable(hashTable, dataItem->idObject, &entry);
	
	if (collision == NULL)
	   insertCollisionHashTable(item, hashTable, entry);
	else
       updateKeeperHashTable(item, collision);
   
} 



static void removeItemHashTable(THashTable* hashTable, TItemHashTable *item){
	int entry;
	TCollisionHashTable *collision;
	TDataHashTable *data = hashTable->data;
	TDataItemHashTable *dataItem = item->data;

	collision = lookupEntryHashTable(hashTable, dataItem->idObject, &entry);

	if (collision != NULL){

		//printEntryHashTable(&(hashTable->entries[entry]));

		removeKeeperHashTable(item, collision);

		if (collision->keepers == NULL){
			if (data->entries[entry].head == collision){
				data->entries[entry].head = collision->next;
				if (collision->next == NULL)
					data->entries[entry].tail = NULL;
			}else if (data->entries[entry].tail == collision){
				data->entries[entry].tail->prev->next = NULL;
				data->entries[entry].tail = collision->prev;
			}else{
				collision->next->prev = collision->prev;
				collision->prev->next = collision->next;
			}

			free(collision);
		}
		//printEntryHashTable(&(hashTable->entries[entry]));
	}

}


static void removeEvictedItensHashTable(THashTable* hashTable, unsigned int idPeer, void *vListDisposed){
	TListObject *listDisposed = vListDisposed;
	TItemHashTable* item;
	TObject *disposed;
	TIdObject idVideo;

	item = createItemHashTable();

	TIteratorListObject *it = createIteratorListObject(listDisposed);

	it->reset(it); disposed = it->current(it);

	while(disposed){

		getIdObject(disposed, idVideo);
		//
		setItemHashTable(item, idPeer, NULL, idVideo, NULL);

		removeItemHashTable(hashTable, item);

		it->next(it); disposed = it->current(it);
	}
	item->dispose(item);

	it->ufree(it);
}

void printEntryHashTable(TEntryHashTable* entry){
 
	TCollisionHashTable *walker;
	TKeeperHashTable *keepers;
	
	walker = entry->head;
	while (walker != NULL){ 
		printf("%s:: ", walker->idObject);
  
        keepers = walker->keepers;
        printf("%d", keepers->idOwner);
        keepers = keepers->next;
        while (keepers != NULL){
            printf(", ");
            printf("%d", keepers->idOwner);
            keepers = keepers->next;
        }
        printf(" ");
		walker = walker->next;
	}
} 


void printHashTable(THashTable* hashTable){
	TDataHashTable *data = hashTable->data;

	int i;
	
    TEntryHashTable* entries = data->entries;
	for (i = 0; i < data->size; i++){
		printf("%d: ", i);
		if (!emptyEntryHashTable(&(entries[i]))){
			printEntryHashTable(&(entries[i]));
		}
	    putchar('\n');
    }
} 


static void disposeEntryHashTable(TEntryHashTable* entry){
 
	TCollisionHashTable *collisions, *collision;
	TKeeperHashTable *keepers, *keeper;
	
	collisions = entry->head;
	while (collisions != NULL){ 
  
        keepers = collisions->keepers;
        while (keepers != NULL){
            keeper = keepers;
            keepers = keepers->next;
            free(keeper);
        }
        collision = collisions;
		collisions = collisions->next;
		free(collision);
	}
	
	entry->head = entry->tail = NULL;
} 


void disposeHashTable(THashTable* hashTable){
	TDataHashTable *data = hashTable->data;

	int i;
	
    TEntryHashTable* entries = data->entries;
	for (i = 0; i < data->size; i++){
		if (!emptyEntryHashTable(&(entries[i]))){
			disposeEntryHashTable(&(entries[i]));
		}
    }
} 

static unsigned long int redundancyDemandHashTable(THashTable *hashTable){
	int i;
	float stored;


	unsigned long int demand=0;
	TKeeperHashTable *keeper;
	THCache *hc;
	TListObject *listClones;
	TObject *object, *clone, *walk, *tmp;
	TDataHashTable *data = hashTable->data;

	listClones = createListObject();

	for (i = 0; i < data->size; i++){
		TCollisionHashTable *collision = data->entries[i].head;

		while(collision!=NULL){

			keeper = collision->keepers;

			while (keeper!=NULL){
				TPeer *peer = keeper->peer;
				if (peer->isUp(peer)){
					hc = peer->getHCache(peer);
					//levels=hc->getLevels(hc);

					//TCache *cache=hc->getCache(hc,i);
					//listObjects = cache->getObjects(cache);
					//object = listObjects->getObject(listObjects,keeper->object);
					object = hc->search(hc,keeper->object);
					clone = cloneObject(object);
					listClones->insertOrd(listClones, clone, storedAsCriteriaObject);




				}
				keeper = keeper->next;
			}

			while(!listClones->isEmpty(listClones)){
				object = listClones->getNext(listClones, NULL);
				walk = listClones->getNext(listClones, object);

				stored = getStoredObject(object);
				//				demand += stored;

				listClones->removeHead(listClones);

				while(walk!=NULL){
					demand += stored; // add replicated segment
					addStoredObject(walk,-1*stored);
					if (getStoredObject(walk) <= 0){
						tmp = listClones->getNext(listClones, walk);
						listClones->remove(listClones, walk);
						walk = tmp;
					}else{
						walk = listClones->getNext(listClones, walk);
					}
				}
			}

			collision = collision->next;
		}
	}

	listClones->destroy( listClones );

	return demand;
}

THashTable* createHashTable(int size){
	TDataHashTable *data;
    THashTable *hashTable;

	int *weight;
	TEntryHashTable* entries;

	data = malloc(sizeof(TDataHashTable));
	hashTable = (THashTable*)malloc(sizeof(THashTable));
	entries = (TEntryHashTable*)malloc(sizeof(TEntryHashTable)*size);

	weight = (int*)malloc(sizeof(int)*KEYMAXLENGTH_HASH_TABLE);

    data->entries = entries;
    data->size = size;
    data->weight = weight;
    data->maxKeyLength = KEYMAXLENGTH_HASH_TABLE;

    hashTable->data = data;

	hashTable->redundancyDemand = redundancyDemandHashTable;
	hashTable->dispose = disposeHashTable;
	hashTable->print = printHashTable;
	hashTable->removeEvictedItens = removeEvictedItensHashTable;
	hashTable->removeItem = removeItemHashTable;
	hashTable->insert = insertHashTable;
	hashTable->insertCollision = insertCollisionHashTable;
	hashTable->lookup = lookupHashTable;

	setHashTable(hashTable);
	setWeightHashTable(hashTable);


    return hashTable;
}


/*
int main(int argc, char *argv[])
{ 
    int i,k;
    float f;
	TItem item;

	THashTable* hashTable;

    //item.idObject = (TKeyHash *)malloc(sizeof(TKeyHash)*200);

    hashTable = createHashTable(100,200);
        
    for(i=1;i<895;i++){
        item.idPeer = i % 20;
    	scanf("%s %d %d %d %d %f", item.idObject, &k, &k, &k, &k, &f);

        insertHashTable(hashTable, &item);
    }   
         
    printHashTable(hashTable);
    disposeHashTable(hashTable);
	printHashTable(hashTable);

	return 0;
} 
*/

