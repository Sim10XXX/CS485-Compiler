#ifndef __ASSEMBLY_H__
#define __ASSEMBLY_H__
#include "tac.h"
#include "map.h"

void assembly_print(TAC_List* taclist, Map* impmap, Map* classmap, FILE* outf);

typedef enum{
    A_NULL,
    A_movq,
    A_addq,
    A_subq,
    A_xor,
    A_andq,
    A_pushq,
    A_popq,
    A_imult,
    A_idiv,
    A_call,
    A_ret,
    A_jmp,
    A_je,
    A_jne,
    A_jle,
    A_jl,
    A_js,
    A_quad,
    A_byte,
    A_leaq,
    A_Other
}Asmtype;

typedef struct{
    Asmtype instruction;
    char* buffer;
    char* source;
    char* dest;
    char unreachable;
}ASM;

typedef struct{
    int length;
    ASM* assembly;
}ASM_List;

void asmlist_add(ASM_List* asmlist, ASM assembly);

ASM_List* assembly_parse(FILE* fptr);

void asmlist_print(ASM_List* asmlist, FILE* fptr);

int asmlist_find_label(ASM_List* asmlist, char* label);

#endif