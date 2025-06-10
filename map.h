#ifndef __MAP_H__
#define __MAP_H__
#include "main.h"

typedef enum {
    attribute_map,      // map of class id -> defined attributes
    class_map,          // map of class id -> defined/inherited attributes
    method_map,         // map of class id -> defined methods
    implementation_map, // map of class id -> defined/inherited methods
    parent_map          // map of class id -> parent class id
} MapType;

// Maps are stored as a null-terminated array of type Map
typedef struct {
    char* id;       // class id
    int linenum;    // line number for the class
    MapType maptype;// map type
    int numchild;   // number of elements in data
    void* data;     // type of data depends on map type
                        // if attribute_map || class_map
                            // data = Attribute*
                        // if method_map || implementation_map
                            // data = ImpMethod*
                        // if parent_map
                            // data = char*
} Map;

typedef struct {
    char* id;       // identifier
    char* type;     // type 
    int linenum;    // line number
    ast_type init;  // either Attribute_no_init or Attribute_init
    AST* expr;      // the initializing expression
} Attribute;

typedef struct {
    char* id;       // identifier
    int linenum;    // line number
    int numformals; // number of parameters
    AST** formals;  // the parameters (ast_type == Formal)
    char* class;    // class in which this method is defined
    char* returntype;//return type
    AST* expr;      // method body

} ImpMethod;

// Called by map_insert_ast(), insert the mapping of the class to it's data while preserving alphabetical order
void map_insert_sorted(Map* map, char* id, int numchild, int linenum, void* d);

// Takes a map and the ast of a class, and creates a mapping depending on maptype
void map_insert_ast(Map* map, AST* class);

// Allocates space for a map given the number of classes that it needs to hold,
// Also initializes the predefined classes (Bool, Int, etc) and their methods
Map* map_initialize(int numchild, MapType maptype);

// Prints a map to the out file
void map_print(Map* map, FILE* outf);

// Counts the length of a map
int map_len(Map* map);

// Get the mapping for a class (Ex. give me the class map for class "A")
Map* map_get_map_by_id(Map* map, char* id);

// After you map_get_map_by_id(), search the data of the mapping (Ex. give me the attribute "x", given the class map for "A")
void* map_get_data_from_map_by_id(Map* map, char* id);

Map* map_read_parent_map(FILE* fptr);
Map* map_read_implementation_map(FILE* fptr);
Map* map_read_class_map(FILE* fptr);

int map_get_offset(Map* map, char* class, char* id);
int map_get_param_offset(Map* impmap, char* currmethod, char* id);
char* map_get_type(Map* impmap, Map* classmap, char* currmethod, char* id);

int map_nearest_ancestor(Map* pmap, char** alist, char* id);

#endif