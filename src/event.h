
typedef enum EVENTS {REQUEST=1,JOIN=2,LEAVE=3,REPLICATE=4,STORE=5,TOPOLOGY=6, SHOW_TOPOLOGY=7, SHOW_MAPQUERY=8,BUFFERING=9,PLAYBACK=10, FINISHED_VIEWING, SHOW_CHANNELS} TTypeEvent;

//typedef unsigned int TTimeEvent;
typedef float TTimeEvent;
typedef unsigned int TOwnerEvent;

typedef struct event TEvent;
typedef TTimeEvent (*TGetTimeEvent)(TEvent* event);
typedef TTypeEvent (*TGetTypeEvent)(TEvent* event);
typedef TOwnerEvent (*TGetOwnerEvent)(TEvent* event);
typedef void (*TUFreeEvent)(TEvent* event);

struct event{
	void *data;
	TGetTimeEvent getTime;
	TGetTypeEvent getType;
	TGetOwnerEvent getOwner;
	TUFreeEvent ufree;
};

TEvent *createEvent(TTimeEvent time, TTypeEvent type, TOwnerEvent owner);


