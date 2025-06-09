#ifndef __TYPECHECK_H__
#define __TYPECHECK_H__

#include "main.h"
#include "map.h"

// Checks for duplicate class names and attribute/method names
void typecheck_map(Map* map);

// Checks if Main exists and has a parameterless method main()
void typecheck_main_method_map(Map* impmap);

// Checks for a cycle in the parentmap. Since maps are sorted, it just so happens to also return the topological sort of the classes
char** typecheck_parent_map_cycle(Map* parentmap);

// Checks for inheritance from Int, String, etc
void typecheck_parent_map_illegal_inherits(Map* parentmap, Map* classmap);

// Checks if the two input methods have the same name, then typechecks the override
// returns true if the method is an override
int typecheck_is_method_override(ImpMethod* parentmethod, ImpMethod* childmethod);

// Given the root and the maps, recursively typechecks the expressions for each class
void typecheck_expr_from_root(Map* classMap, Map* implementationMap, Map* parentMap, AST* root);

#endif