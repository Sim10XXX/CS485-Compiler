#ifndef __TAC_H__
#define __TAC_H__
#include "main.h"
#include "map.h"
//#include "optimizer.h"

typedef enum {
    t_Dynamic_dispatch = 1,
    t_Self_dispatch,
    t_Static_dispatch,
    t_New,
    t_Isvoid,
    t_Plus,
    t_Minus,
    t_Times,
    t_Divide,
    t_Negate,
    t_Lt,
    t_Le,
    t_Eq,
    t_ptLt,
    t_ptLe,
    t_ptEq,
    t_Not,
    t_Integer,
    t_String,
    t_Bool,
    t_Return,
    t_Label,
    t_Bt,
    t_Jmp,
    t_Identifier,
    t_ID,
    t_Default,
    t_Case,
    t_Caseinner,
    t_Esac
}tac_type;

typedef struct {
    tac_type type;
    char* a1;
    char* a2;
    char* a3;
    int numparams;
    int linenum;
    char** params; // for dispatch
    AST* expr; // used to store case statements
    int dead;
}TAC;

typedef struct {
    int length;
    TAC* tac;
    //CFG* taccfg;
}TAC_List;

typedef struct{
    int length;
    char** vars;
}Livevars; //try not to implement a generic expanding array challenge (impossible)


typedef struct CFG_S{
    char* label;
    TAC_List* taclist;
    struct CFG_S* branch1;
    struct CFG_S* branch2;
    int backedge;
    int casebranches;              //reserved for case statements
    struct CFG_S** casebranchlist; //reserved for case statements
    Livevars* livetop; //dataflow for dead variables
    Map* cmap; //For attribute knowledge
}CFG;

typedef struct{
    int length;
    CFG* cfgarray;
}CFG_List;

typedef struct{
    int nummethods;
    CFG_List* cfglistarray;
}CFG_Methods;

void tac_init(Map* pmap);

int tac_convert_method(TAC_List* taclist, AST* method);

void tac_convert_root(TAC_List* taclist, AST* ast);
void tac_convert_classmap(TAC_List* taclist, Map* cmap);

void tac_print_tac(FILE* outf, TAC_List* taclist, int start);
void cfgmethods_print(FILE* outf, CFG_Methods* cfgmethods);
void livevars_print(FILE* outf, Livevars* livevars);

TAC* tac_get_tac();

char* tac_fresh_label();
int tac_convert_expr(TAC_List* taclist, AST* expr, char* ret);

TAC_List* tac_convert_ast(Map* cmap, Map* pmap, AST* root);

CFG_Methods* tac_to_cfg(TAC_List* taclist, Map* cmap);

TAC_List* cfg_to_tac(CFG_Methods* cfgmethods);

void tac_compress(TAC_List* taclist);

void taclist_free(TAC_List* taclist);
void cfgmethods_free(CFG_Methods* cfgmethods);

int taclist_has_input(TAC_List* taclist);

#endif