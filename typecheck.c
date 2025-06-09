//#include "main.h"
//#include "map.h"
#include "typecheck.h"
#include "typeEnvironment.h"

void typecheck_data_for_duplicates(Map* map){
    int i, j, k;
    Attribute* att;
    ImpMethod* met;
    if (map->maptype == class_map){
        att = (Attribute*) map->data;
    }
    else if (map->maptype == method_map){
        met = (ImpMethod*) map->data;
    }
    for (i = 0; i < map->numchild; i++){
        for (j = i+1; j < map->numchild; j++){
            if (map->maptype == class_map){
                if (strcmp(att[i].id, att[j].id) == 0){
                    throw_error(att[j].linenum, "duplicate attribute name");
                }
            }
            else if (map->maptype == method_map){
                if (strcmp(met[i].id, met[j].id) == 0){
                    throw_error(met[j].linenum, "duplicate method name");
                }
            }
        }
        if (map->maptype == method_map){
            for (j = 0; j < met[i].numformals; j++){
                for (k = j+1; k < met[i].numformals; k++){
                    if (strcmp(met[i].formals[j]->id, met[i].formals[k]->id) == 0){
                        throw_error(met[i].linenum,"method has duplicate formal names");
                    }
                }
            }
        }
        
    }
}

void typecheck_data_for_unknown_types_and_for_self(Map* fullmap, Map* currmap){
    int i, j;
    Attribute* att;
    ImpMethod* met;
    if (currmap->maptype == class_map){
        att = (Attribute*) currmap->data;
    }
    else if (currmap->maptype == method_map){
        met = (ImpMethod*) currmap->data;
    }

    for (i = 0; i < currmap->numchild; i++){
        if (currmap->maptype == class_map){
            if (!map_get_map_by_id(fullmap, att[i].type)){
                if (strcmp(att[i].type, "SELF_TYPE") != 0){
                    throw_error(att[i].linenum, "Attribute of unknown type");
                }
            }
            if (strcmp(att[i].id, "self") == 0){
                throw_error(att[i].linenum, "Attribute named self");
            }
        }
        else if (currmap->maptype == method_map){
            if (!map_get_map_by_id(fullmap, met[i].returntype)){
                if (strcmp(met[i].returntype, "SELF_TYPE") != 0){
                    throw_error(met[i].linenum, "Unknown return type for method");
                }
            }
            for (j = 0; j < met[i].numformals; j++){
                if (!map_get_map_by_id(fullmap, met[i].formals[j]->children[0]->id)){
                    throw_error(met[i].formals[j]->linenum, "Unknown type for formal");
                }
                if (strcmp(met[i].formals[j]->id, "self") == 0){
                    throw_error(met[i].formals[j]->linenum, "Formal named self");
                }
            }
        }
    }

    return;
}

void typecheck_map(Map* map){
    int i, j;
    for (i = 0; map[i].id != NULL; i++){
        //check class names
        typecheck_data_for_duplicates(&map[i]);

        //check map data
        typecheck_data_for_unknown_types_and_for_self(map, &map[i]);
    }
}

void typecheck_main_method_map(Map* impmap){
    Map* main = map_get_map_by_id(impmap, "Main");
    if (!main){
        throw_error(0, "No class 'Main' defined");
    }
    ImpMethod* metmain = map_get_data_from_map_by_id(main, "main");
    if (!metmain){
        throw_error(0, "No method 'main' defined");
    }
    if (metmain->numformals != 0){
        throw_error(0, "method 'main' must have 0 formals");
    }
}

// Helper function for typecheck_parent_map_cycle
int is_in_bank(char** bank, int bankc, char* id){
    int i;
    for (i = 0; i < bankc; i++){
        if (strcmp(bank[i], id) == 0){
            return 1;
        }
    }
    return 0;
}

char** typecheck_parent_map_cycle(Map* parentmap){
    int len = 0;
    int i;
    for (i = 0; parentmap[i].id != NULL; i++){
        len++;
    }
    char** bank = malloc(sizeof(char*)*len+1);
    bank[0] = "Object";
    int bankc = 1;

    int loopflag = 1;
    int cycleflag = 0;
    while(loopflag){
        loopflag = 0;
        for (i = 0; parentmap[i].id != NULL; i++){
            if (!is_in_bank(bank, bankc, parentmap[i].id)){
                cycleflag = 1;
                
                if (is_in_bank(bank, bankc, (char*) parentmap[i].data)){
                    loopflag = 1;
                    cycleflag = 0;
                    bank[bankc] = parentmap[i].id;
                    bankc++;
                    break;
                }
            }
        }
        if (cycleflag){
            throw_error(0, "Inheritance cycle");
        }
    }


    return bank;
}

void typecheck_parent_map_illegal_inherits(Map* parentmap, Map* classmap){
    int i, j, k;
    char* parent;
    Attribute* patt, *catt;
    Map* pmap, *cmap;

    //Check for each illegal class
    for (i = 0; parentmap[i].id != NULL; i++){
        if (strcmp(parentmap[i].id, "SELF_TYPE") == 0){
            throw_error(parentmap[i].linenum, "Class named SELF_TYPE");
        }
        parent = (char*)parentmap[i].data;
        if (strcmp(parent, "Int") == 0){
            throw_error(parentmap[i].linenum, "Cannot inherit from Int");
        }
        else if (strcmp(parent, "String") == 0){
            throw_error(parentmap[i].linenum, "Cannot inherit from String");
        }
        else if (strcmp(parent, "Bool") == 0){
            throw_error(parentmap[i].linenum, "Cannot inherit from Bool");
        }
        if (map_get_map_by_id(classmap, parent) == 0){
            throw_error(parentmap[i].linenum, "Cannot inherit from undefined class");
        }
    }
}

int typecheck_is_method_override(ImpMethod* parentmethod, ImpMethod* childmethod){
    if (strcmp(parentmethod->id, childmethod->id) != 0) return 0; //return false
    if (parentmethod->numformals != childmethod->numformals) throw_error(childmethod->linenum, "Cannot redefine inherited method with a different number of formals");
    if (strcmp(parentmethod->returntype, childmethod->returntype) != 0) throw_error(childmethod->linenum, "Cannot redefine inherited method with a different return type");
    int i;
    for (i = 0; i < parentmethod->numformals; i++){
        if (strcmp(parentmethod->formals[i]->children[0]->id, childmethod->formals[i]->children[0]->id) != 0){
            throw_error(childmethod->linenum, "Cannot redefine inherited method and change type of formals");
        }
    }
    return 1; //return true
}

// Checks if subtype conforms to supertype given a parent mapping, uses currclass to decipher SELF_TYPE
int typecheck_conforms(Map* parentMap, char* currclass, char* subtype, char* supertype){
    int i;

    //Base case: if subtype == supertype, subtype conforms to supertype
    if (strcmp(subtype, supertype) == 0) return 1; //return true

    //convert SELF_TYPE to currclass
    if (strcmp(subtype, "SELF_TYPE") == 0){
        return typecheck_conforms(parentMap, currclass, currclass, supertype);
    }
    if (strcmp(supertype, "SELF_TYPE") == 0){
        return typecheck_conforms(parentMap, currclass, subtype, currclass);
    }

    //Get the parent class for subtype
    Map* pmap = map_get_map_by_id(parentMap, subtype);
    if (!pmap) return 0; //If there is no parent, we have reached subtype == "Object' and should return false

    //Check conformance between subtype's parent and the supertype
    return typecheck_conforms(parentMap, currclass, (char*)pmap->data, supertype);
}

// Returns the Least Upper Bound for two types
char* typecheck_get_lub(Map* parentMap, char* currclass, char* type1, char* type2){
    // If type1 conforms to type2, then LUB = type2
    if (typecheck_conforms(parentMap, currclass, type1, type2)){
        return type2;
    }

    // Get the parent class of type2
    Map* pmap = map_get_map_by_id(parentMap, type2);
    if (!pmap) return NULL;

    // Get the LUB of type1 and type2's parent
    return typecheck_get_lub(parentMap, currclass, type1, (char*)pmap->data);
}

// forward declarations
char* typecheck_args_to_signature(TypeEnvironment* gamma, Map* implementationMap, Map* parentMap, char* currclass, AST* args, ImpMethod* signature);
char* typecheck_dispatch(TypeEnvironment* gamma, Map* implementationMap, Map* parentMap, char* currclass, char* methodId, char* dispatchClass, char* callerClass, AST* args, int dispatchLineNum, int methodLineNum);

// Typechecks expressions
void typecheck_expr(TypeEnvironment* gamma, Map* implementationMap, Map* parentMap, char* currclass, AST* expr){
    int i;
    char* m;
    ImpMethod* signature;
    TypeEnvironment* temp;

    // BIG switch statement
    // Depending on the nodetype, it typechecks the expression and sets expr->outtype to the correct type
    switch(expr->nodetype){
        case True:
        case False:
            expr->outtype = "Bool";
            break;
        case Integer:
            expr->outtype = "Int";
            break;
        case String:
            expr->outtype = "String";
            break;
        case Identifier:
            expr->outtype = type_environment_get_type(gamma, expr->children[0]->id);
            if (!expr->outtype){
                char message[128];
                sprintf(message, "unbound identifier %s", expr->children[0]->id);
                throw_error(expr->children[0]->linenum, message);
            }
            break;
        case Assign:
            expr->outtype = type_environment_get_type(gamma, expr->children[0]->id);
            if (!expr->outtype){
                char message[128];
                sprintf(message, "unbound identifier %s", expr->children[0]->id);
                throw_error(expr->children[0]->linenum, message);
            }
            typecheck_expr(gamma, implementationMap, parentMap, currclass, expr->children[1]);
            if (!typecheck_conforms(parentMap, currclass, expr->children[1]->outtype, expr->outtype)){
                throw_error(expr->children[1]->linenum, "expr does not conform to the identifier's type");
            }
            expr->outtype = expr->children[1]->outtype; //Assign's type is the type of the Expr not the ID
            break;
        case Type:
            if (strcmp(expr->id, "SELF_TYPE") != 0 &&
                !map_get_map_by_id(implementationMap, expr->id)){
                throw_error(expr->linenum, "Undefined class/type");
            }
            expr->outtype = expr->id; //A type's type is itself
            break;
        case Self_dispatch:
            expr->outtype = typecheck_dispatch(gamma, implementationMap, parentMap, currclass, expr->children[0]->id, currclass, "SELF_TYPE", expr->children[1], expr->children[0]->linenum, expr->children[0]->linenum);
            break;
        case Dynamic_dispatch:
            char* dispatchClass;
            typecheck_expr(gamma, implementationMap, parentMap, currclass, expr->children[0]);
            if (strcmp(expr->children[0]->outtype, "SELF_TYPE") == 0){
                dispatchClass = currclass;
            }
            else 
                dispatchClass = expr->children[0]->outtype;
            expr->outtype = typecheck_dispatch(gamma, implementationMap, parentMap, currclass, expr->children[1]->id, dispatchClass, expr->children[0]->outtype, expr->children[2], expr->children[0]->linenum, expr->children[1]->linenum);
            break;
        case Static_dispatch:
            typecheck_expr(gamma, implementationMap, parentMap, currclass, expr->children[0]);
            typecheck_expr(gamma, implementationMap, parentMap, currclass, expr->children[1]);
            if (strcmp(expr->children[1]->outtype, "SELF_TYPE") == 0){
                throw_error(expr->children[0]->linenum, "cannot perform static_dispatch on SELF_TYPE");
            }
            if (!typecheck_conforms(parentMap, currclass, expr->children[0]->outtype, expr->children[1]->outtype)){
                throw_error(expr->children[0]->linenum, "type does not conform in static_dispatch");
            }
            expr->outtype = typecheck_dispatch(gamma, implementationMap, parentMap, currclass, expr->children[2]->id, expr->children[1]->outtype, expr->children[0]->outtype, expr->children[3], expr->children[0]->linenum, expr->children[2]->linenum);
            break;
        case Let:
            //Let creates a new type environment for each binded variable
            TypeEnvironment** garray = malloc(sizeof(TypeEnvironment*) * (expr->numchild - 1));
            for (i = 0; i < expr->numchild - 1; i++){
                garray[i] = type_environment_new(1);
                strcpy(garray[i]->typemap[0].id, expr->children[i]->children[0]->id);
                strcpy(garray[i]->typemap[0].type, expr->children[i]->children[1]->id);
                if (strcmp(garray[i]->typemap[0].id, "self") == 0){
                    throw_error(expr->children[i]->children[0]->linenum, "Cannot bind self in a let");
                }
            }
            //Concat the first binding to the original gamma, then each gamma after contains one more binded variable
            for (i = 0; i < expr->numchild - 1; i++){
                if (i == 0)
                    temp = type_environment_cat(garray[i],gamma);
                else
                    temp = type_environment_cat(garray[i],garray[i-1]);
                type_environment_free(garray[i]);
                garray[i] = temp;
            }
            //Typecheck the first binding with the original gamma
            typecheck_expr(gamma, implementationMap, parentMap, currclass, expr->children[0]);
            //Then typecheck with the next gamma
            for (i = 1; i < expr->numchild; i++){
                typecheck_expr(garray[i-1], implementationMap, parentMap, currclass, expr->children[i]);
            }
            //Free all the new gammas
            for (i = 0; i < expr->numchild - 1; i++){
                type_environment_free(garray[i]);
            }
            free(garray);
            expr->outtype = expr->children[expr->numchild - 1]->outtype;
            break;
        case Let_binding_init:
            typecheck_expr(gamma, implementationMap, parentMap, currclass, expr->children[1]);
            typecheck_expr(gamma, implementationMap, parentMap, currclass, expr->children[2]);
            if (!typecheck_conforms(parentMap, currclass, expr->children[2]->outtype, expr->children[1]->outtype)){
                char message[128];
                sprintf(message, "Let initialization (%s) does not conform to (%s)", expr->children[2]->outtype, expr->children[1]->outtype);
                throw_error(expr->children[1]->linenum, message);
            }
            
            break;
        case Case_block:
            for (i = 0; i < expr->numchild; i++){
                typecheck_expr(gamma, implementationMap, parentMap, currclass, expr->children[i]);
            }
            expr->outtype = expr->children[0]->outtype;
            for (i = 1; i < expr->numchild; i++){
                //A case's type is the lub of each case_inner
                expr->outtype = typecheck_get_lub(parentMap, currclass, expr->outtype, expr->children[i]->outtype);
            }
            break;
        case Case_inner:
            //Each case statement binds a variable for its expression
            TypeEnvironment* newg = type_environment_new(1);
            strcpy(newg->typemap[0].id, expr->children[0]->id);
            strcpy(newg->typemap[0].type, expr->children[1]->id);
            if (strcmp(newg->typemap[0].type, "SELF_TYPE") == 0){
                throw_error(expr->children[1]->linenum, "using SELF_TYPE as a case branch type is not allowed");
            }
            temp = type_environment_cat(newg, gamma);
            type_environment_free(newg);
            newg = temp;
            typecheck_expr(newg, implementationMap, parentMap, currclass, expr->children[2]);
            type_environment_free(newg);
            expr->outtype = expr->children[2]->outtype;
            break;
        case Method:
            TypeEnvironment* newgamma = NULL, *g = NULL;
            g = type_environment_new(expr->numchild - 2);
            for (i = 0; i < expr->numchild - 2; i++){
                strcpy(g->typemap[i].id, expr->children[i]->id);
                strcpy(g->typemap[i].type, expr->children[i]->children[0]->id);
            }
            newgamma = type_environment_cat(g, gamma);
            expr->outtype = expr->children[expr->numchild-2]->id;
            typecheck_expr(newgamma, implementationMap, parentMap, currclass, expr->children[expr->numchild-1]);
            if (!typecheck_conforms(parentMap, currclass, expr->children[expr->numchild-1]->outtype, expr->outtype) ||
                (strcmp(expr->outtype, "SELF_TYPE") == 0 && strcmp(expr->children[expr->numchild-1]->outtype, "SELF_TYPE") != 0)){
                char message[128];
                sprintf(message, "Method body (%s) does not conform to return type (%s)", expr->children[expr->numchild-1]->outtype, expr->outtype);
                throw_error(expr->linenum, message);
            }
            type_environment_free(g);
            type_environment_free(newgamma);
            expr->outtype = NULL;
            break;
        default:
            // By default, just call typecheck_expr on every child
            for (i = 0; i < expr->numchild; i++){
                typecheck_expr(gamma, implementationMap, parentMap, currclass, expr->children[i]);
            }
            // Switch statement to determine expr->outtype
            switch(expr->nodetype){
                case If:
                    if (strcmp(expr->children[0]->outtype, "Bool") != 0){
                        throw_error(expr->children[0]->linenum, "if condition must be Bool");
                    }
                    expr->outtype = typecheck_get_lub(parentMap, currclass, expr->children[1]->outtype, expr->children[2]->outtype);
                    break;
                case While:
                    if (strcmp(expr->children[0]->outtype, "Bool") != 0){
                        throw_error(expr->children[0]->linenum, "loop condition must be Bool");
                    }
                    expr->outtype = "Object";
                    break;
                case Isvoid:
                    expr->outtype = "Bool";
                    break;
                case Not:
                    if (strcmp(expr->children[0]->outtype, "Bool") != 0){
                        throw_error(expr->children[0]->linenum, "'Not' can only be performed on Bool");
                    }
                    expr->outtype = "Bool";
                    break;
                case Negate:
                case Plus:
                case Minus:
                case Times:
                case Divide:
                    //Must be Int
                    for (i = 0; i < expr->numchild; i++){
                        if (strcmp(expr->children[i]->outtype, "Int") != 0){
                            throw_error(expr->children[i]->linenum, "Arithmetic operators can only be performed on Int");
                        }
                    }
                    expr->outtype = "Int";
                    break;
                case Eq:
                case Lt:
                case Le:
                    //Typchecks comparisons for the default types
                    if (strcmp(expr->children[0]->outtype, "Int") == 0 ||
                        strcmp(expr->children[0]->outtype, "String") == 0 ||
                        strcmp(expr->children[0]->outtype, "Bool") == 0 ||
                        strcmp(expr->children[1]->outtype, "Int") == 0 ||
                        strcmp(expr->children[1]->outtype, "String") == 0 ||
                        strcmp(expr->children[1]->outtype, "Bool") == 0){
                            if (strcmp(expr->children[0]->outtype, expr->children[1]->outtype) != 0){
                                char message[128];
                                sprintf(message, "Cannot compare %s with %s", expr->children[0]->outtype, expr->children[1]->outtype);
                                throw_error(expr->children[1]->linenum, message);
                            }
                        }
                    expr->outtype = "Bool";
                    break;
                case Attribute_init:
                    if (!typecheck_conforms(parentMap, currclass, expr->children[1]->outtype, expr->children[0]->outtype)){
                        throw_error(expr->children[0]->linenum, "expr does not conform to the attribute's type");
                    }
                    break;
                default:
                    // By default, your outtype is the outtype of your last child
                    if (expr->numchild &&
                        expr->nodetype != Attribute_no_init &&
                        expr->nodetype != Let_binding_no_init
                        /*expr->nodetype != Class &&
                        expr->nodetype != Attribute_init*/){
                        expr->outtype = expr->children[expr->numchild-1]->outtype;

                    }
            }  
    }
}
// Called on the root node
void typecheck_expr_from_root(Map* classMap, Map* implementationMap, Map* parentMap, AST* root){
    TypeEnvironment* gamma;
    Map* currmap;
    int i;
    // For each class
    for (i = 0; i < root->numchild; i++){
        //printf("doing class %s\n", root->children[i]->id);
        currmap = map_get_map_by_id(classMap, root->children[i]->id);

        //get the base type environment for the class
        gamma = type_environment_from_classmap(currmap);

        //typecheck the expressions of the class
        typecheck_expr(gamma, implementationMap, parentMap, root->children[i]->id, root->children[i]);
        type_environment_free(gamma);
    }
}

// Checks if the Args of a method match its signature
// Also calls typecheck_expr for each Arg
char* typecheck_args_to_signature(TypeEnvironment* gamma, Map* implementationMap, Map* parentMap, char* currclass, AST* args, ImpMethod* signature){
    if (!args || !signature) return NULL;
    if (args->nodetype != Args) return NULL;
    if (args->numchild != signature->numformals){
        return "Wrong number of actual args";
    }
    int i;
    for (i = 0; i < args->numchild; i++){
        typecheck_expr(gamma, implementationMap, parentMap, currclass, args->children[i]);
        if (!typecheck_conforms(parentMap, currclass, args->children[i]->outtype, signature->formals[i]->children[0]->id)){
            char* message = malloc(sizeof(char)*128);
            sprintf(message, "Argument #%i type %s does not conform to formal type %s", i+1, args->children[i]->outtype, signature->formals[i]->children[0]->id);
            //throw_error(args->children[i]->linenum, message);
            return message;
        }
    }
    return NULL;
}

// General dispatch typecheck
char* typecheck_dispatch(TypeEnvironment* gamma, Map* implementationMap, Map* parentMap, char* currclass, char* methodId, char* dispatchClass, char* callerClass, AST* args, int dispatchLineNum, int methodLineNum){
    ImpMethod* signature;
    char* m;
    // Get signature
    signature = map_get_data_from_map_by_id(map_get_map_by_id(implementationMap, dispatchClass), methodId);
    if (!signature){
        char message[128];
        sprintf(message, "unknown method %s in dispatch on %s", methodId, dispatchClass);
        throw_error(methodLineNum, message);
    }
    // Typecheck the args
    m = typecheck_args_to_signature(gamma, implementationMap, parentMap, currclass, args, signature);
    if (m){
        throw_error(dispatchLineNum, m);
    }
    if (strcmp(signature->returntype, "SELF_TYPE") == 0){
        return callerClass; ///////////////////////////////////////////////////////////
    }
    // Return the output type
    return signature->returntype;
}