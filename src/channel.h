typedef struct channel TChannel;
typedef struct link TLink;

//LINK

//FROMFILE
// Policy related data/function definition
typedef struct _data_FROMFILEPolicy TDATAFROMFILEPolicy;
typedef int TlmLinkRatesFROMFILEPolicy;
typedef FILE TFpRatesFROMFILEPolicy;

typedef struct FROMFILEPolicy TFROMFILEPolicy;
void *createFROMFILEPolicy(void *entry);


short updateFROMFILEPolicy(TLink *link);
int getLmRatesFROMFILEPolicy(TLink *link);
float getNextRateFROMFILEPolicy(TLink *link);
void setAverageThroughputFROMFILEPolicy(TLink *link, float lastRate, float newRate);

//TChannel *createDataChannel(float capacity, float rate_upload);
TChannel *createDataChannel(float capacity, void *upLink, void *downLink);
TLink *createDataLink(float maxRate, void *policy);

typedef  short (* TCanStreamDataChannel )(TChannel *, float rate);
typedef  float (* TGetULRate )(TChannel *);
typedef  float (* TGetDLRate )(TChannel *);
typedef  TLink* (* TGetUpLinkChannel )(TChannel *);
typedef  TLink* (* TGetDownLinkChannel )(TChannel *);
typedef  void (* TUpdateRateLink )(TChannel *channel);
typedef  short (* TOpenDLDataChannel )(TChannel *, int idPeerSrc, int idPeerDst, float rate);
typedef  short (* TOpenULDataChannel )(TChannel *, int idPeerSrc, int idPeerDst, float rate);
typedef  void (* TCloseDLDataChannel )(TChannel *, unsigned int idPeerDst);
typedef  void (* TCloseULDataChannel )(TChannel *, unsigned int idPeerDst);
typedef  short (* THasDownlinkChannel )(TChannel *, float bitRate);
typedef  float (* TAverageThroughputChannel )(TChannel *);

struct channel{
	void *data;
	TGetULRate getULRate;
	TGetDLRate getDLRate;
	TGetUpLinkChannel getUpLink;
	TGetDownLinkChannel getDownLink;
	TUpdateRateLink updateRates;
	TCanStreamDataChannel canStream;
	TOpenDLDataChannel openDL; // open a DownLink data Channel
	TOpenULDataChannel openUL;
	TCloseDLDataChannel closeDL; // close a DownLink data Channel
	TCloseULDataChannel closeUL;
	THasDownlinkChannel hasDownlink;
	TAverageThroughputChannel averageThroughput;
};


typedef  short (* TUpdateLink )(TLink *);
typedef  float (* TGetCurrentRateLink )(TLink *);
typedef  float (* TGetMaxRateLink )(TLink *);
typedef  float (* TGetNextRateLink )(TLink *);
typedef  int (* TGetlmRatesLink )(TLink *);
typedef  float (* TGetAverageThroughputLink )(TLink *);
typedef void (* TsetAverageThroughputLink)(TLink *link, float lastRate, float newRate);

//
struct link{
	void *data;
	TUpdateLink update;
	TGetCurrentRateLink getCurrentRate;
	TGetMaxRateLink getMaxRate;
	TGetNextRateLink getNextRate;
	TGetlmRatesLink getlmRates;
	TGetAverageThroughputLink getAverageThroughput;
	TsetAverageThroughputLink setAverageThroughput;


};

//Prototipos de funcoes LM

// Policy related declaration

typedef void TLMPolicy;

typedef struct GeneralLinkPolicy TGeneralLinkPolicy;

typedef short (* TLMUpdateGeneralPolicy)(TLink *link);
typedef int (* TLMGetLmRatesGeneralPolicy)(TLink *link);
typedef float (* TLMGetNextRateGeneralPolicy)(TLink *link);
typedef void (* TLMSetAverageThroughputGeneralPolicy)(TLink *link, float lastRate, float newRate);



typedef struct LMGeneralPolicy TLMGeneralPolicy;
struct LMGeneralPolicy{

	TLMUpdateGeneralPolicy Update; // Object Management Policy Update
	TLMGetLmRatesGeneralPolicy getLmLinkRates;
	TLMGetNextRateGeneralPolicy getNextRate;
};

typedef void TDATAGeneralLinkPolicy;

struct GeneralLinkPolicy{
	TLMGeneralPolicy *LM;
	TDATAGeneralLinkPolicy *data;
};

//#


//#### Fluctuation Policy #################

typedef struct fluctuation TFluctuation;

//LINK

//FROMFILE
// Policy related data/function definition
typedef struct _data_FLUCTUATIONFROMFILEPolicy TDATAFLUCTUATIONFROMFILEPolicy;
typedef int TlmTimeForFluctuationFROMFILEPolicy;
typedef FILE* TFpTimeForFluctuationFROMFILEPolicy;

typedef struct FLUCTUATIONFROMFILEPolicy TFLUCTUATIONFROMFILEPolicy;

void *createFluctuationFROMFILEPolicy(void *entry);


short updateFluctuationFROMFILEPolicy(TFluctuation *fluctuation);
int getLmIntervalFluctuationTimeFROMFILEPolicy(TFluctuation *fluctuation);
float getNextIntervalFluctuationTimeFROMFILEPolicy(TFluctuation *fluctuation);
void setAverageIntervalFluctuationTimeFROMFILEPolicy(TFluctuation *fluctuation, float lastRate, float newRate);


TFluctuation *createDataFluctuation(void *policy);



typedef  short (* TUpdateFluctuation )(TFluctuation *);
typedef  float (* TGetCurrentTimeForFluctuation )(TFluctuation *);
typedef  float (* TGetNextTimeForFluctuation )(TFluctuation *);
typedef  int (* TGetlmTimeForFluctuation )(TFluctuation *);
typedef  float (* TGetAverageTimeForFluctuation )(TFluctuation *);
typedef void (* TsetAverageTimeForFluctuation)(TFluctuation *, float lastRate, float newRate);

//
struct fluctuation{
	void *data;
	TUpdateFluctuation update;
	TGetCurrentTimeForFluctuation getCurrentIFT;
	TGetNextTimeForFluctuation getNextTimeForFluctuation;
	TGetlmTimeForFluctuation getlmTimeForFluctuation;
	TGetAverageTimeForFluctuation getAverageIFT;
	TsetAverageTimeForFluctuation setAverageTimeForFluctuation;


};

//Prototipos de funcoes FM ( Fluctuation Manager )

// Policy related declaration

typedef void TFMPolicy;

typedef struct GeneralFluctuationPolicy TGeneralFluctuationPolicy;

typedef short (* TFMUpdateGeneralPolicy)(TFluctuation *fluctuation);
typedef int (* TFMGetLmTimeForFluctuationGeneralPolicy)(TFluctuation *fluctuation);
typedef float (* TFMGetNextTimeForFluctuationGeneralPolicy)(TFluctuation *fluctuation);
typedef void (* TFMSetAverageTimeForFluctuationGeneralPolicy)(TFluctuation *fluctuation, float lastRate, float newRate);



typedef struct FMGeneralPolicy TFMGeneralPolicy;
struct FMGeneralPolicy{

	TFMUpdateGeneralPolicy Update; // Object Management Policy Update
	TFMGetLmTimeForFluctuationGeneralPolicy getLmTimeForFluctuation;
	TFMGetNextTimeForFluctuationGeneralPolicy getNextTimeForFluctuation;
};

typedef void TDATAGeneralFluctuationPolicy;

struct GeneralFluctuationPolicy{
	TFMGeneralPolicy *FM;
	TDATAGeneralFluctuationPolicy *data;
};



























































