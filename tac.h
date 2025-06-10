#ifndef __TAC_H__
#define __TAC_H__
#include "main.h"
#include "map.h"

typedef enum {
    t_Dynamic_dispatch,
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
}TAC;

void tac_init(Map* pmap);

int tac_convert_method(AST* method);

int tac_convert_root(AST* ast);

void tac_print_tac(FILE* outf, int start);

TAC* tac_get_tac();

char* tac_fresh_label();
int tac_convert_expr(AST* expr, char* ret);

#endif