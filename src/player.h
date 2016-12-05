/*
 * player.h
 *
 *  Created on: Nov 10, 2016
 *      Author: kratos
 */

#ifndef PLAYER_H_
#define PLAYER_H_


//Swindow related implementation
typedef int TAvailabilityWindow;
typedef int TSizeWindow;
typedef int TOccupancyWindow;
typedef struct player TPlayer;

typedef void TSchedulingPolicy;
//
//Swindow Related functions
//
TPlayer *createPlayer(TSizeWindow size, TSchedulingPolicy *policy );



typedef short (* TRequestPlayer)(TPlayer *player, void *object);
typedef void (* TPlaybackPlayer)(TPlayer *player);
typedef short (* TStallPlayer)(TPlayer *player);
typedef short (* TStoragePlayer)(TPlayer *player, void *object);
typedef short (* TSwapStoragePlayer)(TPlayer *player);

typedef short (* TIsFitInWindow)(TPlayer *player, void *object);
typedef short (* THasWindow)(TPlayer *player, void *object);
typedef short (* TDisposeWindow)(TPlayer* player);
typedef void (* TShowWindow)(TPlayer* player);
typedef TAvailabilityWindow (* TGetAvailabilityWindow)(TPlayer* player);
typedef TSizeWindow (* TGetSizeWindow)(TPlayer* player);
typedef TOccupancyWindow(* TGetOccupancyWindow)(TPlayer *player);
typedef unsigned int (* TGetNumberOfStoredObjectWindow)(TPlayer* player);

struct player{
	//private data
	void *data;

	//public methods


	TRequestPlayer request;
	TPlaybackPlayer playback;
	TStallPlayer stall;
	TStoragePlayer storage;
	TSwapStoragePlayer swapStorage;
	TIsFitInWindow isFitInWindow;
	THasWindow has;
	TShowWindow show;

	//gets and sets
	TGetAvailabilityWindow getAvailability;
	TGetSizeWindow getSize;
	TGetOccupancyWindow getOccupancy;
	TGetNumberOfStoredObjectWindow getNumberOfStoredObject;


};

// Scheduling Policy


typedef struct playback TPlayback;
typedef struct chunk TChunk;
typedef struct window TWindow;

//playback
struct playback{

	//a definir
	void *availableChunks;
	void *scheduleChunks;


};


//Window
struct window{

	int size;
	float occupancy;
	TChunk **scheduledChunks;
	void *policy;

};


//chunk
struct chunk{

	int ID;
	float initFlow;
	float endFlow;
	float lengthBytes;
	float duration;
	int available;  // 0 | 1
	int reproduced; // 0 | 1
};



// Politica generica


// Policy related declaration
//
typedef void TSWMPolicy;

// General Policy stuff declaration
typedef short (* TSWMReplaceGeneralPolicy)(void* systemData, void* cache, void* object);
typedef short (* TSWMUpdateGeneralPolicy)(void* systemData, void* cache, void* object);
typedef short (* TSWMisFitInSwindowGeneralPolicy)(void* player, void* object);


//
typedef struct GeneralPolicySwm TGeneralPolicySwm;

// Policy related data/function definition
//
typedef struct SWMGeneralPolicy TSWMGeneralPolicy;

struct SWMGeneralPolicy{
	TSWMReplaceGeneralPolicy Replace; // Object Management Policy Replacement(LRU/Popularity)
	TSWMUpdateGeneralPolicy Update; // Object Management Policy Update cache(LRU/Popularity)
	TSWMisFitInSwindowGeneralPolicy IsFitInSwindow; // Eligibility criteria

};

typedef void TDATAGeneralPolicySwm;

struct GeneralPolicySwm{
	TSWMGeneralPolicy *SWM;
	TDATAGeneralPolicySwm *data;
};
//fim politica generica

//
//Greedy
typedef struct GreedyPolicy TGreedyPolicy;
void *createGreedyPolicy(void *entry);
short replaceGreedyPolicy(void* vSysInfo, void* cache, void* object);
short updateGreedyPolicy(void* systemData,void* cache, void* object);
short isFitInSwindowGreedyPolicy(void* systemData, void* object);











#endif /* PLAYER_H_ */
