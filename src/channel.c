/*
 * channel.c
 *
 *  Created on: 07/09/2013
 *      Author: cesar
 */
#include "stdlib.h"
#include "stdio.h"
#include "dictionary.h"
#include "internals.h"
#include "channel.h"


//static void setAverageThroughputLink(TLink *link, float lastRate, float newRate);
static float getCurrentRateLink(TLink *link);
static float getMaxRateLink(TLink *link);
static float getAverageThroughputLink(TLink *link);
//static float getNextRateLink(TLink *link);
static short updateLink(TLink *link);
static int getLmRateLink(TLink *link);


typedef struct socket_data_channel{
	int idPeerSrc;
	int idPeerDst;
} TSocketDataChannel;

static TSocketDataChannel *createSocketDataChannel(int idPeerSrc, int idPeerDst ){
	TSocketDataChannel *socket = malloc(sizeof(TSocketDataChannel));

	socket->idPeerSrc = idPeerSrc;
	socket->idPeerDst = idPeerDst;

	return socket;
}

typedef struct _ongoing_data_channel{
	float eb; // effective Bandwidth
	TSocketDataChannel sock;
} TOngoingDataChannel;

TOngoingDataChannel *createOngoingDataChannel(float rate, int idPeerSrc, int idPeerDst){
	TOngoingDataChannel *data = malloc(sizeof(TOngoingDataChannel));

	data->eb = rate;
	data->sock.idPeerSrc = idPeerSrc;
	data->sock.idPeerDst = idPeerDst;

	return data;
}

typedef struct _data_channel{

	float capacity; // Mbps

	float max_uplink;
	float max_downlink;
	float rate_uplink; // Mbps
	float rate_downlink; // Mbps

	TLink *upLink;
	TLink *downLink;
	TDictionary *ongoingUL; // opened data channel on UPLink
	TDictionary *ongoingDL; // opened data channel on DOWNLink

} TDataChannel;

// LINK Management
typedef struct _data_link{

	FILE *fpRates;
	float maxRate;
	int lmRates;
	void *throughputPolicy;
	float currentRate;
	float averageThroughput;

} TDataLink;


static TDataChannel *initDataChannel(float capacity,TLink *upLink,TLink *downLink){
	TDataChannel *data = malloc(sizeof(TDataChannel));

	TDataLink *dataUpLink=upLink->data;
	TDataLink *dataDownLink=downLink->data;

	data->capacity =capacity;
	data->upLink = upLink;
	data->downLink = downLink;
	data->max_uplink = dataUpLink->maxRate ;
	data->max_downlink = dataDownLink->maxRate;
	data->rate_uplink = data->max_uplink;
	data->rate_downlink = data->max_downlink;

	//data->rate_downlink = data->max_downlink;
	data->ongoingUL = createDictionary();
	data->ongoingDL = createDictionary();
	return data;
}

/*bandwidthBroker(){

}*/

enum {UPLINK = 1, DOWNLINK=2, UNDEFINED=3};

static short canStreamDataChannel(TChannel *channel, float rate){
	TDataChannel *data = channel->data;

	return data->rate_uplink > rate ? 1 : 0;
}

static float getULRateChannel(TChannel *channel){
	TDataChannel *data = channel->data;

	return data->max_uplink - data->rate_uplink;
}

static float getDLRateChannel(TChannel *channel){
	TDataChannel *data = channel->data;

	return data->max_downlink - data->rate_downlink;
}


//
static TLink *getUpLinkChannel(TChannel *channel){

	TDataChannel *data = channel->data;

	return data->upLink;
}
static TLink *getDownLinkChannel(TChannel *channel){

	TDataChannel *data = channel->data;

	return data->downLink;
}
//




static void updateRatesLinks(TChannel *channel){

	TDataChannel *data = channel->data;

	TLink *upLink = channel->getUpLink(channel);
	TLink *downLink = channel->getDownLink(channel);

	float lastRate, nextRate,newRate;

	//Update upLink
	lastRate = upLink->getCurrentRate(upLink) ;
	upLink->update(upLink);
	nextRate = upLink->getCurrentRate(upLink) ;

	if(nextRate > lastRate){
		newRate = nextRate - lastRate;
		data->rate_uplink +=newRate;
	}else{
		newRate = lastRate - nextRate;
		data->rate_uplink +=newRate;
	}

	//update downLink
	lastRate = downLink->getCurrentRate(downLink) ;
	downLink->update(downLink);
	nextRate = downLink->getCurrentRate(downLink) ;

	if(nextRate > lastRate){
		newRate = nextRate - lastRate;
		data->rate_downlink +=newRate;
	}else{
		newRate = lastRate - nextRate;
		data->rate_downlink +=newRate;
	}

}


static short hasDownlinkChannel(TChannel *channel, float bitRate) {
	TDataChannel *data = channel->data;

	return data->rate_downlink >= bitRate ? 1 : 0;
}



static void closeDLDataChannel(TChannel *channel, unsigned int idPeerDst){
	TOngoingDataChannel *ongoingDC;
	TDataChannel *data = channel->data;

	ongoingDC = data->ongoingDL->remove(data->ongoingDL,idPeerDst);
	data->rate_downlink += ongoingDC->eb;


	free(ongoingDC);
}


static void closeULDataChannel(TChannel *channel, unsigned int idPeerDst){
	TOngoingDataChannel *ongoingDC;
	TDataChannel *data = channel->data;

	ongoingDC = data->ongoingUL->remove(data->ongoingUL,idPeerDst);
	data->rate_uplink += ongoingDC->eb;

	free(ongoingDC);
}

static short openULDataChannel(TChannel *channel, int idPeerSrc, int idPeerDst, float eb){
	TDataChannel *data = channel->data;
	TLink *upLink = channel->getUpLink(channel);
	float currentRate = upLink->getCurrentRate(upLink);
	short opened = 0; // status of requested data channel

	//if(data->rate_uplink <= currentRate){

		//data->rate_uplink = data->rate_uplink + (currentRate - data->rate_uplink);

		if (data->rate_uplink >= eb){
			TOngoingDataChannel *ongDC = createOngoingDataChannel(eb, idPeerSrc, idPeerDst);
			data->ongoingUL->insert(data->ongoingUL, idPeerDst, ongDC);
			data->rate_uplink -= eb;
			opened = 1; // true
		}else{
			printf("Failed to open UL channel!\n");
			printf("eb: %f, rate_uplink: %f\n", eb, data->rate_uplink);
			fflush(stdout);
		}
	//}

	return opened;
}


static short openDLDataChannel(TChannel *channel, int idPeerSrc, int idPeerDst, float eb){
	TDataChannel *data = channel->data;
	TLink *downLink = channel->getDownLink(channel);
	float currentRate = downLink->getCurrentRate(downLink);

	short opened=0; // status of requested data channel


	//if(data->rate_downlink <= currentRate){

		if( (data->rate_downlink>= eb) ){
			TOngoingDataChannel *ongoingDC = createOngoingDataChannel(eb, idPeerSrc, idPeerDst);
			data->ongoingDL->insert(data->ongoingDL, idPeerDst, ongoingDC);
			data->rate_downlink -= eb;
			opened = 1; // true
		}else{
			printf("Failed to open DL channel!\n");
			printf("eb: %f, rate_downlink: %f\n", eb, data->rate_downlink);
			fflush(stdout);
		}

	//}


	return opened;
}


TChannel *createDataChannel(float capacity, void *upLink, void *downLink){

	TChannel *channel = malloc(sizeof(TChannel));

	channel->data = initDataChannel(capacity, upLink, downLink);

	channel->canStream = canStreamDataChannel;
	channel->getULRate = getULRateChannel;
	channel->getDLRate = getDLRateChannel;
	channel->getUpLink = getUpLinkChannel;
	channel->getDownLink = getDownLinkChannel;
	channel->updateRates = updateRatesLinks;
	channel->closeDL = closeDLDataChannel;
	channel->openDL = openDLDataChannel;
	channel->closeUL = closeULDataChannel;
	channel->openUL = openULDataChannel;
	channel->hasDownlink = hasDownlinkChannel;


	return channel;
}

// LINK Management

static TDataLink *initDataLink(float maxRate, TGeneralLinkPolicy *policy){

	TDataLink *data = malloc(sizeof(TDataLink));

	data->maxRate = maxRate;
	data->throughputPolicy = policy;
	data->currentRate = maxRate;
	data->averageThroughput = 0.0;

	return data;
}
TLink *createDataLink(float maxRate, void *policy){

	TLink *link = malloc(sizeof(TLink));
	link->data = initDataLink(maxRate, policy);
	link->update = updateLink;
	link->getCurrentRate = getCurrentRateLink;
	link->getMaxRate = getMaxRateLink;
	//link->getNextRate = getNextRateLink;
	link->getlmRates = getLmRateLink;
	link->getAverageThroughput = getAverageThroughputLink;
	//link->setAverageThroughput = setAverageThroughputLink;

	return link;
}

//Metodos LINK
static int getLmRateLink(TLink *link){
	int status;
	TDataLink *data = link->data;
	TGeneralLinkPolicy *policy = data->throughputPolicy;

	status = policy->LM->getLmLinkRates(link);

	return status;
}

static float getCurrentRateLink(TLink *link){

	TDataLink *data=link->data;
	float currentRate=data->currentRate;

	return currentRate;
}

static float getMaxRateLink(TLink *link){

	TDataLink *data=link->data;
	//float maxRate=data->maxRate;

	return data->maxRate;

}

static float getAverageThroughputLink(TLink *link){

	TDataLink *data=link->data;

	return data->averageThroughput;
}

/*static float getNextRateLink(TLink *link){

	TDataLink *data=link->data;
	TGeneralLinkPolicy *policy = data->throughputPolicy;


	float nextRate = policy->LM->getNextRate(link);
	float maxRate = data->maxRate;

	if(nextRate > maxRate){
		printf("ERROR: The next rate exceeds the maximum rate \n");
		exit (1);
	}

	return nextRate;
}*/


static short updateLink(TLink *link){
	short status;
	TDataLink *data = link->data;
	TGeneralLinkPolicy *policy = data->throughputPolicy;
	//float lastRate, nextRate,newRate;

	//lastRate = data->currentRate;

	status = policy->LM->Update(link);//update Current Rate (  Fluctuation )
/*
	nextRate = data->currentRate;

	if(nextRate > lastRate){

		newRate = nextRate - lastRate;

	}else{

		newRate = lastRate - nextRate;
	}*/


	return status;
}




//####################	 LINK MANAGER

// LINK Policy
typedef struct LMFROMFILEPolicy TLMFROMFILEPolicy;
struct LMFROMFILEPolicy{
	//

	TLMUpdateGeneralPolicy Update; //
	TLMGetLmRatesGeneralPolicy getLmRates;
	TLMGetNextRateGeneralPolicy getNextRate;
	TLMSetAverageThroughputGeneralPolicy setAverageThroughput;

};

struct _data_FROMFILEPolicy{
	TlmLinkRatesFROMFILEPolicy lmRates;
	TFpRatesFROMFILEPolicy *fpRates;
};
//#
struct FROMFILEPolicy{
	TLMFROMFILEPolicy *LM;
	TDATAFROMFILEPolicy *data;
};

void *createFROMFILEPolicy(void *entry){

	TFROMFILEPolicy *policy = (TFROMFILEPolicy *) malloc(sizeof( TFROMFILEPolicy ) );
	policy->LM = (TLMFROMFILEPolicy *) malloc(sizeof( TLMFROMFILEPolicy ) );
	policy->data =(TDATAFROMFILEPolicy *) malloc(sizeof(TDATAFROMFILEPolicy));;
	// init dynamics

	policy->LM->Update = updateFROMFILEPolicy; // Object Management Policy Update LINK
	policy->LM->getLmRates = getLmRatesFROMFILEPolicy;
	policy->LM->getNextRate = getNextRateFROMFILEPolicy;
	policy->LM->setAverageThroughput = setAverageThroughputFROMFILEPolicy;

	char *fileName;

	TParameters *lp = createParameters(entry, PARAMETERS_SEPARATOR);

	lp->iterator(lp);

	policy->data->lmRates = (TlmLinkRatesFROMFILEPolicy)atoi(lp->next(lp));
	fileName=lp->next(lp);

	policy->data->fpRates = fopen(fileName, "r");
	if (policy->data->fpRates == NULL){
		printf("PANIC: ERROR! FILE WITH RATES LINK, NOT FOUND\n");
		printf("channel.c, line 395 \n");
		exit(0);
	}

	lp->dispose(lp);



	return policy;
}


//Returns a status that points out whether or not
short updateFROMFILEPolicy(TLink *link){

	short status = 0;
	float lastRate, nextRate;
	TDataLink *data = link->data;
	TFROMFILEPolicy *policy = data->throughputPolicy;

	lastRate = data->currentRate;

	data->currentRate = policy->LM->getNextRate(link);

	policy->LM->setAverageThroughput(link, lastRate, data->currentRate);

	status = 1;


	return status;
}

///#
int getLmRatesFROMFILEPolicy(TLink *link){
	int lmRates;
	TDataLink *data = link->data;
	TFROMFILEPolicy *policy = data->throughputPolicy;

	lmRates = policy->data->lmRates;

	return lmRates;
}

///# NEXT RATE FROMFILE POLICY
float getNextRateFROMFILEPolicy(TLink *link){
	float nextRate;
	TDataLink *data = link->data;
	TFROMFILEPolicy *policy = data->throughputPolicy;
	FILE *fp=policy->data->fpRates;

	if( !feof(fp) )
		fscanf(fp,"%f",&nextRate);
	else{
		fseek(fp, 0L, SEEK_SET);
		fscanf(fp,"%f",&nextRate);
	}

	float maxRate = data->maxRate;

		if(nextRate > maxRate){
			printf("ERROR: The next rate exceeds the maximum rate \n");
			exit (1);
		}


	return nextRate;
}

void setAverageThroughputFROMFILEPolicy(TLink *link, float lastRate, float newRate){

	TDataLink *data=link->data;

	// calcular media ponderada;

	if (data->averageThroughput > 0) {
		data->averageThroughput = (0.25 * lastRate + ((1 - 0.25) * newRate));
	}else{

		data->averageThroughput=newRate;

	}

}



//#### Fluctuation Policy #################


static float getCurrentTimeForFluctuation(TFluctuation *fluctuation);
static float getAverageTimeForFluctuation(TFluctuation *fluctuation);

static short updateFluctuation(TFluctuation *fluctuation);
//static int getLmTimeForFluctuation(TFluctuation *fluctuation);



// FLUCTUATION Management
typedef struct _data_fluctuation{

	FILE *fpIFT;
	//float nextIFT;
	//int lmIFT;
	void *policy;
	float currentIFT;
	float averageIFT;

} TDataFluctuation;



/*//
static TLink *getUpLinkChannel(TChannel *channel){

	TDataChannel *data = channel->data;

	return data->upLink;
}
static TLink *getDownLinkChannel(TChannel *channel){

	TDataChannel *data = channel->data;

	return data->downLink;
}*/
//
// A Política deve obter os links por meio de um método no channel para atualiza-los.


// FLUCTUATION Management

static TDataFluctuation *initDataFluctuation(TGeneralFluctuationPolicy *policy){

	TDataFluctuation *data = malloc(sizeof(TDataFluctuation));


	data->policy = policy;
	data->averageIFT = 0.0;

	return data;
}
TFluctuation *createDataFluctuation(void *policy){

	TFluctuation *fluctuation = malloc(sizeof(TFluctuation));
	fluctuation->data = initDataFluctuation(policy);
	fluctuation->update = updateFluctuation;
	fluctuation->getCurrentIFT = getCurrentTimeForFluctuation;
	//fluctuation->getlmTimeForFluctuation = getLmTimeForFluctuation;
	fluctuation->getAverageIFT = getAverageTimeForFluctuation;


	return fluctuation;
}

//Metodos fluctuation
/*static int getLmTimeForFluctuation(TFluctuation *fluctuation){
	int QtdTimesForFluctuations;
	TDataFluctuation *data = fluctuation->data;
	TGeneralFluctuationPolicy *policy = data->policy;

	QtdTimesForFluctuations = policy->FM->getLmTimeForFluctuation(fluctuation);

	return QtdTimesForFluctuations;
}*/

static float getCurrentTimeForFluctuation(TFluctuation *fluctuation){

	TDataFluctuation *data=fluctuation->data;

	return data->currentIFT;
}

static float getAverageTimeForFluctuation(TFluctuation *fluctuation){

	TDataFluctuation *data=fluctuation->data;

	return data->averageIFT;
}


static short updateFluctuation(TFluctuation *fluctuation){
	short status;
	TDataFluctuation *data=fluctuation->data;
	TGeneralFluctuationPolicy *policy = data->policy;

	status = policy->FM->Update(fluctuation);

	return status;
}




//####################	 Fluctuation MANAGER

// Fluctuation Policy
typedef struct FMFROMFILEPolicy TFMFROMFILEPolicy;
struct FMFROMFILEPolicy{
	//

	TFMUpdateGeneralPolicy Update; //
	//TFMGetLmTimeForFluctuationGeneralPolicy getLmIFT;
	//TFMGetNextTimeForFluctuationGeneralPolicy getNextIFT;
	TFMSetAverageTimeForFluctuationGeneralPolicy setAverageIFT;
};

struct _data_FLUCTUATIONFROMFILEPolicy{
	//TlmTimeForFluctuationFROMFILEPolicy lmIFT;
	TFpTimeForFluctuationFROMFILEPolicy fpSourceIFT;
};
//#
struct FLUCTUATIONFROMFILEPolicy{
	TFMFROMFILEPolicy *FM;
	TDATAFLUCTUATIONFROMFILEPolicy *data;

};

void *createFluctuationFROMFILEPolicy(void *entry){

	TFLUCTUATIONFROMFILEPolicy *policy = (TFLUCTUATIONFROMFILEPolicy *) malloc(sizeof( TFLUCTUATIONFROMFILEPolicy ) );
	policy->FM = (TFMFROMFILEPolicy *) malloc(sizeof( TFMFROMFILEPolicy ) );
	policy->data =(TDATAFLUCTUATIONFROMFILEPolicy *) malloc(sizeof(TDATAFLUCTUATIONFROMFILEPolicy));;
	// init dynamics

	policy->FM->Update = updateFluctuationFROMFILEPolicy;
	//policy->FM->getLmIFT = getLmIntervalFluctuationTimeFROMFILEPolicy;
	//policy->FM->getNextIFT = getNextIntervalFluctuationTimeFROMFILEPolicy;
	policy->FM->setAverageIFT = setAverageIntervalFluctuationTimeFROMFILEPolicy;

	char *fileName;

	TParameters *lp = createParameters(entry, PARAMETERS_SEPARATOR);

	lp->iterator(lp);
//
//	policy->data->lmRates = (TlmFluctuationRatesFROMFILEPolicy)atoi(lp->next(lp));


	fileName=lp->next(lp);

	policy->data->fpSourceIFT = fopen(fileName, "r");
	if (policy->data->fpSourceIFT == NULL){
		printf("PANIC: ERROR! FILE WITH TIMES FOR FLUCTUATION NOT FOUND\n");
		printf("channel.c, line 622 \n");
		exit(0);
	}

	lp->dispose(lp);



	return policy;
}


//Returns a status that points out whether or not
short updateFluctuationFROMFILEPolicy(TFluctuation *fluctuation){

	short status = 0;
	float lastTimeForFluctuation,nextTimeForFlow;
	TDataFluctuation *data = fluctuation->data;
	TFLUCTUATIONFROMFILEPolicy *policy = data->policy;

	lastTimeForFluctuation = data->currentIFT;

		FILE *fp = policy->data->fpSourceIFT;

		if( !feof(fp) )
			fscanf(fp,"%f",&nextTimeForFlow);
		else{
			fseek(fp, 0L, SEEK_SET);
			fscanf(fp,"%f",&nextTimeForFlow);
		}

		data->currentIFT = nextTimeForFlow;

	policy->FM->setAverageIFT(fluctuation, lastTimeForFluctuation, data->currentIFT);

	status = 1;


	return status;
}

///#
/*int getLmIntervalFluctuationTimeFROMFILEPolicy(TFluctuation *fluctuation){
	int lmRates;
	TDataFluctuation *data = fluctuation->data;
	TFLUCTUATIONFROMFILEPolicy *policy = data->policy;

	lmRates = policy->data->lmIFT;

	return lmRates;
}*/

/*///# NEXT RATE FROMFILE POLICY
float getNextIntervalFluctuationTimeFROMFILEPolicy(TFluctuation *fluctuation){
	float nextTimeForFlow;
	TDataFluctuation *data = fluctuation->data;
	TFLUCTUATIONFROMFILEPolicy *policy = data->policy;
	FILE *fp = policy->data->fpSourceIFT;

	if( !feof(fp) )
		fscanf(fp,"%f",&nextTimeForFlow);
	else{
		fseek(fp, 0L, SEEK_SET);
		fscanf(fp,"%f",&nextTimeForFlow);
	}

	return nextTimeForFlow;
}*/

void setAverageIntervalFluctuationTimeFROMFILEPolicy(TFluctuation *fluctuation, float lastTimeForFluctuation, float newTimeForFluctuation){

	TDataFluctuation *data = fluctuation->data;

	// calcular media ponderada;

	if (data->averageIFT > 0) {
		data->averageIFT = (0.25 * lastTimeForFluctuation + ((1 - 0.25) * newTimeForFluctuation));
	}else{

		data->averageIFT=newTimeForFluctuation;

	}

}


