#include "typeEnvironment.h"
#include <string.h>

TypeEnvironment* type_environment_new(int numelements){
    TypeEnvironment* e = malloc(sizeof(TypeEnvironment));
    e->numelements = numelements;
    e->typemap = malloc(sizeof(TypeMap)*numelements);
    return e;
}

TypeEnvironment* type_environment_from_classmap(Map* classMap){
    TypeEnvironment* e = type_environment_new(classMap->numchild+1);
    int i;
    Attribute* att = (Attribute*) classMap->data;

    // bind each attribute
    for (i = 0; i < classMap->numchild; i++){
        strcpy(e->typemap[i].id, att[i].id);
        strcpy(e->typemap[i].type, att[i].type);
    }

    // bind self
    strcpy(e->typemap[e->numelements-1].id, "self");
    strcpy(e->typemap[e->numelements-1].type, "SELF_TYPE");

    return e;
}

TypeEnvironment* type_environment_cat(TypeEnvironment* e1, TypeEnvironment* e2){
    TypeEnvironment* e = malloc(sizeof(TypeEnvironment));
    e->numelements = e1->numelements + e2->numelements;
    e->typemap = malloc(sizeof(TypeMap)* e->numelements);
    memcpy(e->typemap, e1->typemap, sizeof(TypeMap)*e1->numelements);
    memcpy(e->typemap+e1->numelements, e2->typemap, sizeof(TypeMap)*e2->numelements);
    return e;
}

char* type_environment_get_type(TypeEnvironment* e, char* id){
    int i;
    char* ret;
    for (i = 0; i < e->numelements; i++){
        if (strcmp(e->typemap[i].id, id) == 0){
            ret = malloc(sizeof(char)*128);
            strcpy(ret, e->typemap[i].type);
            return ret;
        }
    }
    return NULL;
}

void type_environment_free(TypeEnvironment* e){
    free(e->typemap);
    free(e);
}