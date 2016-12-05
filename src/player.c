/*
 * player.c
 *
 *  Created on: Nov 10, 2016
 *      Author: kratos
 */
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "system.h"
#include "player.h"
#include "object.h"
#include "hierarchy.h"



//prototipos de funcoes Player
static short requestChunk(TPlayer *player, void *object);
static short isFitInWindow(TPlayer *player, void *object);
static void playbackWindow(TPlayer *player);
static short stallPlayback(TPlayer *player);
static short storageBuffered(TPlayer *player, void *object);
static short swapStorageSwindow(TPlayer *player);
static void showWindow(TPlayer *player);
static short hasWindow(TPlayer *player, void *object);
static TAvailabilityWindow getAvailabilityWindow(TPlayer *player);
static TSizeWindow getSizeWindow(TPlayer *player);
static TOccupancyWindow getOccupancyWindow(TPlayer *player);
static unsigned int getNumberOfStoredObjectWindow(TPlayer *player);

static TPlayback *createPlayback(TSizeWindow size);



typedef struct _data_player TDataPlayer; //
struct _data_player{
	TListObject *buffer; // window objects
	THCache *storage;
	TPlayback *playback;
	TWindow *window;
	TSizeWindow size; // in seconds)

	void *policy;
};

static TDataPlayer *initDataPlayer(TSizeWindow size, void *policy ){
	TDataPlayer *data = malloc(sizeof(TDataPlayer));

	data->buffer = createListObject();
	data->playback = createPlayback(size);
	data->size = size;
	data->policy = policy;


	return data;
}


TPlayer *createPlayer(TSizeWindow size, TSchedulingPolicy *policy ){
	TPlayer *player = (TPlayer *)malloc(sizeof(TPlayer));

	player->data = initDataPlayer(size, policy);

	player->request = requestChunk;		 // dispara requição/requisições
	player->isFitInWindow = isFitInWindow; 		//verifica se item cabe no tempo da janela
	player->playback = playbackWindow;	 	//reproduz janela ( o que houver disponivel )
	player->stall = stallPlayback;		// parada por falta de segmentos a reproduzir
	player->storage = storageBuffered; 		//armazena os objetos q serão reproduzidos
	player->swapStorage = swapStorageSwindow; 		//troca os objetos de nivel de cache apos reproducao

	//sets and gets
	player->show = showWindow;
	player->has = hasWindow;
	player->getAvailability = getAvailabilityWindow;
	player->getSize = getSizeWindow;
	player->getOccupancy = getOccupancyWindow;
	player->getNumberOfStoredObject = getNumberOfStoredObjectWindow;

	return player;

}

//Functions
static short requestChunk(TPlayer *player, void *object){
	short status=0;
/*	TDataCache *data = cache->data;
	TGeneralPolicy *policy = data->policy;

	TListObject *disposed = data->disposed;
	disposed->cleanup(disposed);*/

	//verificar se objeto já não está disponivel

	//status = policy->SWM->request(systemData, cache, object);

	return status;
}


static short isFitInWindow(TPlayer *player, void *object){
	short status=0;
	TDataPlayer *data = player->data;
	//TGeneralPolicy *policy = data->policy;

	//status = policy->WM->isFitInSwindow(systemData, cache, object);

	return status;
}

static void playbackWindow(TPlayer *player){

}

static short stallPlayback(TPlayer *player){
	short status=0;


	return status;
}
static short storageBuffered(TPlayer *player, void *object){
	short status=0;


	return status;
}
static short swapStorageSwindow(TPlayer *player){
	short status=0;


	return status;
}

static void showWindow(TPlayer *player){



}

static short hasWindow(TPlayer *player, void *object){
	short status=0;


	return status;
}

static TAvailabilityWindow getAvailabilityWindow(TPlayer *player){

	TAvailabilityWindow availabilitySwindow;

	return availabilitySwindow;
}

static TSizeWindow getSizeWindow(TPlayer *player){
	TDataPlayer *data = player->data;

		return data->size;
}

static TOccupancyWindow getOccupancyWindow(TPlayer *player){

	TOccupancyWindow occupancySwindow;

	return occupancySwindow;

}
static unsigned int getNumberOfStoredObjectWindow(TPlayer *player){

	unsigned int numberOfStoredObjectSwindow;

	return numberOfStoredObjectSwindow;
}
static TPlayback *createPlayback(TSizeWindow size){
	TPlayback *playback;

		return playback;
}


// Políticas de escalonamento



// Greedy Policy

// Greedy Policy
typedef struct SWMGreedyPolicy TSWMGreedyPolicy;
struct SWMGreedyPolicy{
	//
	TSWMReplaceGeneralPolicy Replace; // Object Management Policy Replacement(Greedy/Popularity)
	TSWMUpdateGeneralPolicy Update; // Object Management Policy Update cache(Greedy/Popularity)
	TSWMisFitInSwindowGeneralPolicy IsFitInSwindow; // Object Management Policy Eligibility


};

typedef void TDATAGreedyPolicy;

struct GreedyPolicy{
	TSWMGreedyPolicy *SWM;
	TDATAGreedyPolicy *data;
};


void *createGreedyPolicy(void *entry){

	TGreedyPolicy *policy = (TGreedyPolicy *) malloc(sizeof( TGreedyPolicy ) );

	policy->SWM = (TSWMGreedyPolicy *) malloc(sizeof( TSWMGreedyPolicy ) );

	policy->data = NULL;

	// init dynamics
	policy->SWM->Replace = replaceGreedyPolicy; // Object Management Policy Replacement(Greedy/Popularity)
	policy->SWM->Update = updateGreedyPolicy; // Object Management Policy Update cache(Greedy/Popularity)
	policy->SWM->IsFitInSwindow = isFitInSwindowGreedyPolicy;

	return policy;
}



//Returns a status that points out whether or not
short replaceGreedyPolicy(void* vSysInfo, void* cache, void* object){
	TSystemInfo *sysInfo = vSysInfo;
	short status = 0;


		setLastAccessObject(object,sysInfo->getTime(sysInfo));



	return status;
}

//Returns a status that points out whether or not
short updateGreedyPolicy(void* xSysInfo,void* cache, void* object){
	TSystemInfo *sysInfo = xSysInfo;
	short status = 0;

		setLastAccessObject(object,sysInfo->getTime(sysInfo));

	status = 1;

	//    printf("Call Greedy\n");

	return status;
}

short isFitInSwindowGreedyPolicy(void* swindow, void* object){
	short status = 0;


	return status;
}




//
