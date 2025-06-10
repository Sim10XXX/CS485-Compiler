#include "optimizer.h"
#include <ctype.h>
int optimizer_tac_local_dead_analysis(CFG* cfg);

Livevars* livevars_new(){
    Livevars* livevars = malloc(sizeof(Livevars));
    livevars->length = 1;
    livevars->vars = malloc(sizeof(char*) * 1);
    memset(livevars->vars, 0, sizeof(char*) * 1);
    return livevars;
}

void livevars_free(Livevars* livevars){
    free(livevars->vars);
    free(livevars);
}

int livevars_is_live(Livevars* livevars, char* var){
    int i;
    char** vars = livevars->vars;
    //if (strcmp(var, "t$5") == 0){
    //    printf("var: %s\n", var);
    //    livevars_print(stdout, livevars);
    //}

    for (i = 0; vars[i] != NULL; i++){
        if (strcmp(vars[i], var) == 0){
            return 1;
        }
    }
    return 0;
}

void livevars_add_var(Livevars* livevars, char* var){
    int i;
    if (livevars_is_live(livevars, var)){
        return;
    }
    for (i = 0; livevars->vars[i] != NULL; i++){
    }
    if (i >= livevars->length-1){
        char** tempvars = malloc(sizeof(char*) * livevars->length * 2);
        memset(tempvars, 0, sizeof(char*) * livevars->length * 2);
        memcpy(tempvars, livevars->vars, sizeof(char*) * livevars->length);
        free(livevars->vars);
        livevars->vars = tempvars;
        livevars->length = livevars->length*2;
    }
    livevars->vars[i] = var;
}

void livevars_remove_var(Livevars* livevars, char* var){
    int i;
    char** vars = livevars->vars;
    int flag = 0;
    for (i = 0; vars[i] != NULL; i++){
        if (flag){
            //strcpy(vars[i], vars[i+1]);
            vars[i] = vars[i+1];
        }
        else{
            if (strcmp(vars[i], var) == 0){
                flag = 1;
                i--;
            }
        }
        
    }
}

void livevars_append(Livevars* livevars, Livevars* newvars){
    int i;
    char** vars = newvars->vars;
    for (i = 0; vars[i] != NULL; i++){
        livevars_add_var(livevars, vars[i]);
    }
}

int livevars_isequal(Livevars* livevars1, Livevars* livevars2){
    int i, j;
    for (i = 0; livevars1->vars[i] != NULL; i++){
    }
    for (j = 0; livevars2->vars[j] != NULL; j++){
    }
    if (j != i) return 0;
    for (i = 0; livevars1->vars[i] != NULL; i++){
        if (strcmp(livevars1->vars[i], livevars2->vars[i]) != 0){
            return 0;
        }
    }
    return 1;
}


void optimizer_eliminate_dead_tac(CFG_Methods* cfgmethods){
    TAC* tac;

    //initialize everything to dead
    int i, j, k;
    for (i = 0; i < cfgmethods->nummethods; i++){
        for (j = 0; j < cfgmethods->cfglistarray[i].length; j++){
            tac = cfgmethods->cfglistarray[i].cfgarray[j].taclist->tac;
            for (k = 0; tac[k].a1 != NULL; k++){
                tac[k].dead = 1;
            }
        }
    }

    //printf("d1\n");
    //first universally eliminate classes / attributes / methods that are dead / unreachable
    //maybe this can be handled on the assembly level... unless some other optimization could benefit from this


    //second, eliminate global/regional/local dead stuff

    for (i = 0; i < cfgmethods->nummethods; i++){
        int changeflag = 1;
        while (changeflag){ //while something has changed
            changeflag = 0;
            for (j = 0; j < cfgmethods->cfglistarray[i].length; j++){
                //do local/regional analysis on each basic cfg block
                //printf("d2\n");
                if (optimizer_tac_local_dead_analysis(&cfgmethods->cfglistarray[i].cfgarray[j])){
                    changeflag = 1;
                }
                //printf("d3\n");
            }
        }
    }
    

    //remove dead tac and compress
    //cfgmethods_print(stdout, cfgmethods);

    if (0) return;
    for (i = 0; i < cfgmethods->nummethods; i++){
        for (j = 0; j < cfgmethods->cfglistarray[i].length; j++){
            tac = cfgmethods->cfglistarray[i].cfgarray[j].taclist->tac;
            for (k = 0; tac[k].a1 != NULL; k++){
                if (tac[k].dead){
                    memset(&tac[k], 0, sizeof(TAC));
                }
            }
            tac_compress(cfgmethods->cfglistarray[i].cfgarray[j].taclist);
        }
    }

    //printf("done\n");
}

void livevars_reset_attributes(Livevars* livevars, Map* cmap){ //could be more optimal, by being able to choose which attributes
    Attribute* att = (Attribute*)cmap->data;
    int i;
    for (i = 0; i < cmap->numchild; i++){
        livevars_add_var(livevars, att[i].id);
    }
}

int optimizer_tac_local_dead_analysis(CFG* cfg){
    int i, j;
    //printf("o0\n");
    if (cfg == NULL){
        printf("cfg is null, kinda ggs ngl\n");
    }
    TAC* tac = cfg->taclist->tac;
    //if (!livevars){
    Livevars* livevars = livevars_new();
    //}
    livevars_reset_attributes(livevars, cfg->cmap);

    int changeflag = 0;
    //printf("o1\n");
    //init livevars (get livetop from child branches)
    if (cfg->branch1){
        if (cfg->branch1->livetop){
            livevars_append(livevars, cfg->branch1->livetop);
        }
    }
    if (cfg->branch2){
        if (cfg->branch2->livetop){
            livevars_append(livevars, cfg->branch2->livetop);
        }
    }
    if (cfg->casebranchlist){
        for (i = 0; i < cfg->casebranches; i++){
            if (cfg->casebranchlist[i]->livetop){
                livevars_append(livevars, cfg->casebranchlist[i]->livetop);
            }
        }
    }
    
    //printf("o2\n");
    for(i = 0; tac[i].a1 != NULL; i++){}
    for(i = i-1; i >= 0; i--){
        if (tac[i].type == t_Label ||
            tac[i].type == t_Jmp ||
            tac[i].type == t_Esac ||
            tac[i].type == t_Caseinner){
            if (tac[i].dead){
                tac[i].dead = 0;
                changeflag = 1;
            }
        }
        else if (tac[i].type == t_Return ||
            tac[i].type == t_Case ||
            tac[i].type == t_Bt){
            livevars_add_var(livevars, tac[i].a1);
            if (tac[i].dead){
                tac[i].dead = 0;
                changeflag = 1;
            }
        }
        else if (tac[i].type == t_Bool ||
            tac[i].type == t_Integer ||
            tac[i].type == t_String){
            if (livevars_is_live(livevars, tac[i].a1)){
                if (tac[i].dead){
                    tac[i].dead = 0;
                    changeflag = 1;
                }
                livevars_remove_var(livevars, tac[i].a1);
            }
        }
        else if (tac[i].type == t_Dynamic_dispatch ||
            tac[i].type == t_Static_dispatch ||
            tac[i].type == t_Self_dispatch ||
            tac[i].type == t_New){
            livevars_reset_attributes(livevars, cfg->cmap); //very pessimistic reset
            if (tac[i].type == t_Dynamic_dispatch ||
                tac[i].type == t_Static_dispatch){
                livevars_add_var(livevars, tac[i-1].a1);
            }
            livevars_remove_var(livevars, tac[i].a1);
            if (tac[i].dead){
                tac[i].dead = 0;
                changeflag = 1;
            }
            for (j = 0; j < tac[i].numparams; j++){
                livevars_add_var(livevars, tac[i].params[j]);
            }
            
        }
        else{
            //printf("o5\n");
            //printf("tactype: %i\n", tac[i].type);
            if (tac[i].type == t_Identifier){
                if (strcmp(tac[i].a1, tac[i].a2) == 0){
                    tac[i].dead = 1;
                    continue;
                }
            }
            if (livevars_is_live(livevars, tac[i].a1)){
                //printf("o5.1\n");
                if (tac[i].dead){
                    tac[i].dead = 0;
                    changeflag = 1;
                }
                //printf("o5.2\n");
                livevars_add_var(livevars, tac[i].a2);
                //printf("o5.3\n");
                if (tac[i].a3){
                    livevars_add_var(livevars, tac[i].a3);
                }
                //printf("o5.4\n");
                livevars_remove_var(livevars, tac[i].a1);
            }
        }
        //printf("o6\n");
    }
    //printf("o7\n");
    //update livetop
    if (cfg->livetop){
        if (!livevars_isequal(cfg->livetop, livevars)){
            changeflag = 1;
        }
        livevars_free(cfg->livetop);
    }
    cfg->livetop = livevars;

    return changeflag;
}

void optimizer_eliminate_asm_rec(ASM_List* asmlist, int start, Map* pmap){
    ASM* assembly = asmlist->assembly;
    int i, j;
    int index, index2;
    char templabel[BUFMAX];
    memset(templabel, 0, sizeof(templabel));
    if (start == -1){
        printf("negative start index\n");
    }
    for (i = start; assembly[i].instruction != A_NULL; i++){
        if (assembly[i].unreachable == 0) return;
        assembly[i].unreachable = 0;
        //printf("un\n");
        if (assembly[i].instruction == A_jmp){
            i = asmlist_find_label(asmlist, assembly[i].source) - 1;
        }
        else if (assembly[i].instruction == A_je ||
            assembly[i].instruction == A_jle ||
            assembly[i].instruction == A_jne ||
            assembly[i].instruction == A_js ||
            assembly[i].instruction == A_jl){
            optimizer_eliminate_asm_rec(asmlist, asmlist_find_label(asmlist, assembly[i].source), pmap);
        }
        else if (assembly[i].instruction == A_call){
            //printf("A_call\n");
            if (strncmp(assembly[i].source, "*%r", 3) == 0){
                if (assembly[i].dest){
                    //printf("lalala\n");
                    //assembly[i].dest is of the form: ##2.Main.7
                    //printf("%s\n", assembly[i].dest);
                    char class[BUFMAX];
                    char method[BUFMAX];
                    
                    memset(class, 0, sizeof(class));
                    memset(method, 0, sizeof(class));
                    tac_type ttype = assembly[i].dest[2] - '0';
                    //printf("%s\n", assembly[i].dest);
                    
                    for (index = 4; assembly[i].dest[index] != '.'; index++){}
                    strncpy(class, &assembly[i].dest[4], index-4);

                    for (index2 = index+1; index2 == index+1 || assembly[i].dest[index2] != '.'; index2++){
                        //printf("c: %c\n", assembly[i].dest[index2]);
                    }
                    strncpy(method, &assembly[i].dest[index+1], index2-(index+1));
                    //printf("%s\n", method);

                    int offset = atoi(&assembly[i].dest[index2+1]);
                    if (ttype == t_Dynamic_dispatch ||
                        ttype == t_Self_dispatch){ 
                        //printf("p0\n");
                        List* classes = map_get_all_subclasses(pmap, class);
                        //printf("p1\n");
                        for (j = 0; classes->data[j] != NULL; j++){
                            //printf("%s:%s\n", class, classes->data[j]);
                            strcpy(class, classes->data[j]);
                            strcat(class, "..vtable");
                            index = asmlist_find_label(asmlist, class) + offset + 1;
                            //printf("%s\n", assembly[index].source);
                            index = asmlist_find_label(asmlist, assembly[index].source);
                            //printf("ind: %i\n", index);
                            optimizer_eliminate_asm_rec(asmlist, index, pmap);
                            //printf("__%s\n", classes->data[j]);
                            if (strcmp(method, "type_name") == 0){ //type_name also makes the string of the class available
                                index = asmlist_find_label(asmlist, class) + 1;
                                index = asmlist_find_label(asmlist, assembly[index].source);
                                optimizer_eliminate_asm_rec(asmlist, index, pmap);
                            }
                        }
                        //printf("p2\n");
                        list_free(classes);
                    }
                    else{// static dispatch
                        strcat(class, "..vtable");
                        index = asmlist_find_label(asmlist, class) + offset + 1;
                        //printf("%s\n", assembly[index].source);
                        optimizer_eliminate_asm_rec(asmlist, asmlist_find_label(asmlist, assembly[index].source), pmap);

                        if (strcmp(method, "type_name") == 0){ //type_name also makes the string of the class available
                            index = asmlist_find_label(asmlist, class) + 1;
                            index = asmlist_find_label(asmlist, assembly[index].source);
                            optimizer_eliminate_asm_rec(asmlist, index, pmap);
                        }
                    }
                }
                else{
                    for (j = i; j > 0; j--){
                        if (!assembly[j].dest) continue;
                        if (strcmp(&assembly[i].source[1], assembly[j].dest) == 0){
                            break;
                        }
                    }
                    optimizer_eliminate_asm_rec(asmlist, asmlist_find_label(asmlist, assembly[j].source), pmap);
                }
            }
            else if (strcmp(assembly[i].source, "exit") == 0){
                return;
            }
            else if (strcmp(assembly[i].source, "calloc") == 0 || //external functions
                    strcmp(assembly[i].source, "strcmp") == 0 ||
                    strcmp(assembly[i].source, "printf") == 0 ||
                    strcmp(assembly[i].source, "fputc@PLT") == 0 ||
                    strcmp(assembly[i].source, "fgetc@PLT") == 0 ||
                    strcmp(assembly[i].source, "fflush@PLT") == 0 ||
                    strcmp(assembly[i].source, "calloc@PLT") == 0 ||
                    strcmp(assembly[i].source, "snprintf@PLT") == 0 ||
                    strcmp(assembly[i].source, "strndup@PLT") == 0 ||
                    strcmp(assembly[i].source, "fgets") == 0 ||
                    strcmp(assembly[i].source, "sscanf") == 0){
                continue;
            }
            else{
                optimizer_eliminate_asm_rec(asmlist, asmlist_find_label(asmlist, assembly[i].source), pmap);
            }
        }
        else if (assembly[i].instruction == A_ret){
            if (strcmp(assembly[i+1].buffer, ".cfi_endproc") == 0){
                assembly[i+1].unreachable = 0;
            }
            return;
        }
        else if (assembly[i].instruction == A_movq){
            if (assembly[i].source[0] == '$' &&
                !isdigit(assembly[i].source[1])){
                //printf("n: %s\n", assembly[i].source);
                index = asmlist_find_label(asmlist, assembly[i].source);
                if (index != -1){
                    optimizer_eliminate_asm_rec(asmlist, index, pmap);
                }
            }
            
        }
        else if (assembly[i].instruction == A_byte){
            if (assembly[i].source[0] == '0'){
                return;
            }
        }
        else if (assembly[i].instruction == A_leaq &&
                assembly[i].source[0] == '.'){
            for (index = 0; assembly[i].source[index] != '\0'; index++){
                if (assembly[i].source[index] == '('){
                    strncpy(templabel, assembly[i].source, index);
                    //printf(":clueless:\n");
                    optimizer_eliminate_asm_rec(asmlist, asmlist_find_label(asmlist, templabel), pmap);
                    break;
                }
            }
            

        }
        else if (assembly[i].instruction == A_Other){
            //printf("other: %s\n",assembly[i].buffer);
        }
    }
}

ASM_List* optimizer_eliminate_unreachable_asm(ASM_List* asmlist, Map* pmap){
    ASM* assembly = asmlist->assembly;
    ASM_List* newlist = malloc(sizeof(ASM_List));
    //init everything to unreachable (except the vtables)
    int i;
    for (i = 0; assembly[i].instruction != A_NULL; i++){
        if (strncmp(assembly[i].buffer, "## vtables done", 10) == 0){
            break;
        }
    }
    //printf("i: %i\n", i);
    //i--; //to catch the .globl Bool..new
    for (; assembly[i].instruction != A_NULL; i++){
        assembly[i].unreachable = 1;
    }
    optimizer_eliminate_asm_rec(asmlist, asmlist_find_label(asmlist, "start"), pmap);
    //printf("asm analysis done\n");

    newlist->length = 16;
    newlist->assembly = malloc(sizeof(ASM)*newlist->length);
    memset(newlist->assembly, 0, sizeof(ASM)*newlist->length);

    for (i = 0; assembly[i].instruction != A_NULL; i++){
        if (assembly[i].unreachable) continue;
        asmlist_add(newlist, assembly[i]);
    }
    assembly = newlist->assembly;
    for (i = 0; assembly[i].instruction != A_NULL; i++){
        if (assembly[i].instruction == A_quad){
            if (asmlist_find_label(newlist, assembly[i].source) == -1){
                assembly[i].source = "0";
            }
        }
    }
    return newlist;
}