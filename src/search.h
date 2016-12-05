/*
 * search.h
 *
 *  Created on: Jan 15, 2013
 *      Author: cesar
 */

#ifndef SEARCH_H_
#define SEARCH_H_

typedef struct search TSearch;

typedef void* (*TRunSearch)(TSearch *search, void *peer, void *object, unsigned int clientId, float prefetchFraction);
typedef void (*TUFreeSearch)(TSearch *search);
struct search{
	void *data;
	TRunSearch run;
	TUFreeSearch ufree;
};

TSearch *createRandomWalkSearch(char *entry);
TSearch *createFloadingSearch(char *entry);
TSearch *createNeighborhoodSearch(char *entry);



#endif /* SEARCH_H_ */
