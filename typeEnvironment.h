#ifndef __TYPEENVIRONMENT_H__
#define __TYPEENVIRONMENT_H__
#include "map.h"

typedef struct {
    char id[128];
    char type[128];
} TypeMap; //maps an id to a type

typedef struct {
    int numelements;
    TypeMap* typemap;
} TypeEnvironment; // A type environment is a list of typemaps


// Allocate a type environment for numelements number of elements
TypeEnvironment* type_environment_new(int numelements);

// Allocates a type environment given a classMap, also binds "self"
TypeEnvironment* type_environment_from_classmap(Map* classMap);

// Creates a NEW type environment as the concatenation of e1 + e2
TypeEnvironment* type_environment_cat(TypeEnvironment* e1, TypeEnvironment* e2);

// Search environment for id, returns type
char* type_environment_get_type(TypeEnvironment* e, char* id);

// Free a type environment pointer
void type_environment_free(TypeEnvironment* e);

#endif