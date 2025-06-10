#include "environment.h"
#include <string.h>

Environment* environment_new(int numelements){
    Environment* e = malloc(sizeof(Environment));
    e->numelements = numelements;
    e->idmap = malloc(sizeof(IdMap)*numelements);
    memset(e->idmap, 0, sizeof(IdMap)*numelements);
    return e;
}

Environment* environment_cat(Environment* e1, Environment* e2){
    Environment* e = malloc(sizeof(Environment));
    e->numelements = e1->numelements + e2->numelements;
    e->idmap = malloc(sizeof(IdMap)* e->numelements);
    memcpy(e->idmap, e1->idmap, sizeof(IdMap)*e1->numelements);
    memcpy(e->idmap+e1->numelements, e2->idmap, sizeof(IdMap)*e2->numelements);
    return e;
}

char* environment_get_type(Environment* e, char* id){
    int i;
    char* ret;
    for (i = 0; i < e->numelements; i++){
        if (strcmp(e->idmap[i].id, id) == 0){
            ret = malloc(sizeof(char)*128);
            strcpy(ret, e->idmap[i].type);
            return ret;
        }
    }
    return NULL;
}

char* environment_get_store(Environment* e, char* id){
    int i;
    //char* ret;
    for (i = 0; i < e->numelements; i++){
        if (strcmp(e->idmap[i].id, id) == 0){
            //ret = malloc(sizeof(char)*128);
            //strcpy(ret, e->idmap[i].type);
            return e->idmap[i].store;
        }
    }
    return id; //if id is not in the environment, it must be an attribute (which exists because we already typechecked)
}

void environment_free(Environment* e){
    free(e->idmap);
    free(e);
}