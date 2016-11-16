#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>



// Load Factor has to be defined based on the Global Cache Size 
#define KEYMAXLENGTH_HASH_TABLE  100
#define LOAD_FACTOR_HASH_TABLE 0.4

typedef struct ItemHashTable TItemHashTable;
typedef struct KeeperHashTable TKeeperHashTable;
typedef struct CollisionHashTable TCollisionHashTable;
typedef struct EntryHashTable TEntryHashTable;
typedef struct HashTable THashTable;

typedef char TKeyHashTable [KEYMAXLENGTH_HASH_TABLE];

typedef unsigned long int (*TRedundancyDemandHashTable)(THashTable *hashTable);
typedef void (*TDisposeHashTable)(THashTable* hashTable);
typedef void (*TPrintHashTable)(THashTable* hashTable);
typedef void (*TRemoveEvictedItensHashTable)(THashTable* hashTable, unsigned int idPeer, void *vListDisposed);
typedef void (*TRemoveItemHashTable)(THashTable* hashTable, TItemHashTable *item);
typedef void (*TInsertHashTable)(THashTable* hashTable, TItemHashTable* item);
typedef void (*TInsertCollisionHashTable)(TItemHashTable* item, THashTable* hashTable, int entry);
typedef void* (*TLookupHashTable)(THashTable* hashTable, TKeyHashTable idObject);
typedef void* (*TLookupEntryHashTable)(THashTable* hashTable, TKeyHashTable idObject, int *entry);
typedef void (*TSetHashTable)(THashTable* hashTable);
typedef int (*THashingHashTable)(THashTable* hashTable, TKeyHashTable key);

struct HashTable{
	void *data;
	TRedundancyDemandHashTable redundancyDemand;
	TDisposeHashTable dispose;
	TPrintHashTable print;
	TRemoveEvictedItensHashTable removeEvictedItens;
	TRemoveItemHashTable removeItem;
	TInsertHashTable insert;
	TInsertCollisionHashTable insertCollision;
	TLookupHashTable lookup;
};

THashTable* createHashTable(int size);

typedef void (*TSetItemHashTable)(TItemHashTable *item, int idOwner, void *peer, TKeyHashTable idObject, void *object);
typedef void (*TDisposeItemHashTable)(TItemHashTable *item);

struct ItemHashTable {
	void *data;
	TSetItemHashTable set;
	TDisposeItemHashTable dispose;
};

TItemHashTable *createItemHashTable();

TKeeperHashTable* getNextKeeperHashTable(TKeeperHashTable *keeper);
int getOwnerKeeperHashTable(TKeeperHashTable *keeper);

TKeeperHashTable *createKeeperHashTable(int idOwner, void* peer, void *object);
