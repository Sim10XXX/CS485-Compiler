#ifndef __ENVIRONMENT_H__
#define __ENVIRONMENT_H__
#include "main.h"

typedef struct {
    char id[BUFMAX];
    char type[BUFMAX];
    char* store; //name of temporary that holds the data
} IdMap; //maps an id to a store

typedef struct {
    int numelements;
    IdMap* idmap;
} Environment; // An environment is a list of idmaps


// Allocate an environment for numelements number of elements
Environment* environment_new(int numelements);

// Creates a NEW environment as the concatenation of e1 + e2
Environment* environment_cat(Environment* e1, Environment* e2);

// Search environment for id, returns type
char* environment_get_type(Environment* e, char* id);

// Search environment for id, returns store
char* environment_get_store(Environment* e, char* id);

// Free an environment pointer
void environment_free(Environment* e);

#endif