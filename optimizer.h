#ifndef __OPTIMIZER_H__
#define __OPTIMIZER_H__
#include "main.h"
#include "tac.h"
#include "assembly.h"

void optimizer_eliminate_dead_tac(CFG_Methods* cfgmethods);

ASM_List* optimizer_eliminate_unreachable_asm(ASM_List* asmlist, Map* pmap);

#endif