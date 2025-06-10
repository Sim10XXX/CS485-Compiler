#include "tac.h"
#include "environment.h"

TAC out[50000];
int outn = 0;
int tempc = 0;
int labelc = 0;
char* currclass = NULL;
char* currmethod = NULL;
char* currattribute = NULL;
Map* parentmap;

void tac_init(Map* pmap){
    memset(out, 0, sizeof(out));
    parentmap = pmap;
}

void tac_add_tac(TAC_List* taclist, TAC tac);
char* tac_fresh_label();
char* tac_fresh_temp(char* retvar);
tac_type tac_ast_to_tac_type(ast_type a);

char* tac_convert_rec(TAC_List* taclist, AST* expr, char* returnvar, Environment* env);
char** tac_evaluate_params(TAC_List* taclist, AST* args, Environment* env);

TAC_List* tac_convert_ast(Map* cmap, Map* pmap, AST* root){
    tac_init(pmap);
    TAC_List* taclist = malloc(sizeof(TAC_List));
    if (!taclist) return NULL;
    taclist->length = 16;
    taclist->tac = malloc(sizeof(TAC) * taclist->length);
    memset(taclist->tac, 0, sizeof(TAC) * taclist->length);

    tac_convert_root(taclist, root);
    tac_convert_classmap(taclist, cmap);

    return taclist;
}

void tac_convert_classmap(TAC_List* taclist, Map* cmap){
    int i, j;
    Attribute* att;
    //printf("con\n");
    for (i = 0; cmap[i].id != NULL; i++){
        att = (Attribute*)cmap[i].data;
        currclass = cmap[i].id;
        //printf("%s:%i\n", cmap[i].id, cmap[i].numchild);
        for (j = 0; j < cmap[i].numchild; j++){
            if (att[j].init == Attribute_init){
                currattribute = att[j].id;
                //printf("p\n");
                tac_convert_expr(taclist, att[j].expr, currattribute);
                currattribute = NULL;
            }
        }
        currclass = NULL;
    }

}

void tac_convert_root(TAC_List* taclist, AST* ast){
    int i;
    if (ast->nodetype == Class){
        currclass = ast->id;
    }
    if (ast->nodetype == Method){
        currmethod = ast->id;
        tac_convert_method(taclist, ast);
        currmethod = NULL;
        //return 1;
    }
    /*if (ast->nodetype == Attribute_init){
        //printf("id: %s\n", ast->id);
        currattribute = ast->id;
        tac_convert_expr(ast->children[1], ast->id);
        //printf("done\n");
        currattribute = NULL;
    }*/
    for (i = 0; i < ast->numchild; i++){
        tac_convert_root(taclist, ast->children[i]);
        //if (tac_convert_root(ast->children[i])){
        //    return 1;
        //}
    }
    if (ast->nodetype == Class){
        currclass = NULL;
    }
    return;
}

int tac_convert_expr(TAC_List* taclist, AST* expr, char* ret){
    TAC currtac;
    labelc = 0;
    tempc = 0;
    int start = outn;
    memset(&currtac, 0, sizeof(TAC));
    currtac.type = t_Label;
    currtac.a1 = tac_fresh_label();
    tac_add_tac(taclist, currtac);

    currtac.type = t_Return;
    AST* e = expr;//->children[expr->numchild-1];
    if (!ret){
        currtac.a1 = tac_fresh_temp(NULL);
    }
    else{
        currtac.a1 = ret;
    }
    
    Environment* env = environment_new(0);
    tac_convert_rec(taclist, e, currtac.a1, env);
    tac_add_tac(taclist, currtac);
    return start;
}

int tac_convert_method(TAC_List* taclist, AST* method){
    TAC currtac;
    labelc = 0;
    tempc = 0;
    int start = outn;
    memset(&currtac, 0, sizeof(TAC));
    currtac.type = t_Label;
    currtac.a1 = tac_fresh_label();
    tac_add_tac(taclist, currtac);

    currtac.type = t_Return;
    AST* expr = method->children[method->numchild-1];
    currtac.a1 = tac_fresh_temp(NULL);
    Environment* env = environment_new(0);
    tac_convert_rec(taclist, expr, currtac.a1, env);
    tac_add_tac(taclist, currtac);
    return start;
}

char* tac_convert_rec(TAC_List* taclist, AST* expr, char* returnvar, Environment* env){
    TAC currtac;
    TAC multitac[10];
    Environment* tempenv;
    Environment* currenv, *newenv;
    int i, j;
    char* temp;
    char* templabel, *templabel2;
    char** labellist, **typelist;
    char* casevar;
    //printf("n\n");
    memset(&currtac, 0, sizeof(TAC));
    memset(multitac, 0, sizeof(multitac));
    currtac.linenum = expr->linenum;
    //printf("linenum: %i, type: %i\n", expr->linenum, expr->nodetype);
    //printf("type: %i\n", expr->nodetype);
    switch(expr->nodetype){
    case Plus:
    case Minus:
    case Times:
    case Divide:
    case Lt:
    case Le:
    case Eq:
        currtac.type = tac_ast_to_tac_type(expr->nodetype);
        currtac.a1 = tac_fresh_temp(returnvar);
        currtac.a2 = tac_convert_rec(taclist, expr->children[0], NULL, env);
        currtac.a3 = tac_convert_rec(taclist, expr->children[1], NULL, env);
        if (UnboxInt){
            if (strcmp(expr->children[0]->outtype, "Int") == 0 &&
            strcmp(expr->children[1]->outtype, "Int") == 0){
                switch (currtac.type){
                    case t_Eq:
                        currtac.type = t_ptEq;
                        break;
                    case t_Le:
                        currtac.type = t_ptLe;
                        break;
                    case t_Lt:
                        currtac.type = t_ptLt;
                        break;
                }
            }
        }
        break;
    case Integer:
    case String:
        currtac.type = tac_ast_to_tac_type(expr->nodetype);
        currtac.a2 = expr->id;
        currtac.a1 = tac_fresh_temp(returnvar);
        break;
    case True:
        currtac.type = t_Bool;
        currtac.a2 = "true";
        currtac.a1 = tac_fresh_temp(returnvar);
        break;
    case False:
        currtac.type = t_Bool;
        currtac.a2 = "false";
        currtac.a1 = tac_fresh_temp(returnvar);
        break;
    case Not:
    case Negate:
    case Isvoid:
        currtac.type = tac_ast_to_tac_type(expr->nodetype);
        currtac.a1 = tac_fresh_temp(returnvar);
        currtac.a2 = tac_convert_rec(taclist, expr->children[0], NULL, env);
        break;
    case New:
        currtac.type = tac_ast_to_tac_type(expr->nodetype);
        currtac.a1 = tac_fresh_temp(returnvar);
        currtac.a2 = expr->children[0]->id;
        break;
    case If:
        returnvar = tac_fresh_temp(returnvar);

        multitac[0].type = t_Not;
        multitac[0].a2 = tac_convert_rec(taclist, expr->children[0], NULL, env);
        multitac[0].a1 = tac_fresh_temp(NULL);

        multitac[3].type = t_Label; // Then branch
        multitac[3].a1 = tac_fresh_label();
        multitac[5].type = t_Label; // Else branch
        multitac[5].a1 = tac_fresh_label();
        multitac[7].type = t_Label; // Join branch
        multitac[7].a1 = tac_fresh_label();

        multitac[4].type = t_Jmp;
        multitac[4].a1 = multitac[7].a1; // jmp to join branch
        multitac[6].type = t_Jmp;
        multitac[6].a1 = multitac[7].a1; // jmp to join branch

        multitac[1].type = t_Bt;
        multitac[1].a1 = multitac[0].a1; //if false
        multitac[1].a2 = multitac[5].a1;
        multitac[2].type = t_Bt; 
        multitac[2].a1 = multitac[0].a2; //if true
        multitac[2].a2 = multitac[3].a1;

        
        for (i = 0; i < 8; i++){
            //printf("bu\n");
            tac_add_tac(taclist, multitac[i]);
            if (i == 3){
                tac_convert_rec(taclist, expr->children[1], returnvar, env);
            }
            if (i == 5){
                tac_convert_rec(taclist, expr->children[2], returnvar, env);
            }
        }
        return returnvar;
    case While:
        returnvar = tac_fresh_temp(returnvar);

        multitac[0].type = t_Label; // Predicate
        multitac[0].a1 = tac_fresh_label();

        currtac.type = t_Jmp;
        currtac.a1 = multitac[0].a1;
        tac_add_tac(taclist, currtac);

        tac_add_tac(taclist, multitac[0]);
        
        multitac[1].type = t_Not;
        multitac[1].a2 = tac_convert_rec(taclist, expr->children[0], NULL, env);
        multitac[1].a1 = tac_fresh_temp(NULL);
        
        multitac[4].type = t_Label; // Loop branch
        multitac[4].a1 = tac_fresh_label();
        multitac[6].type = t_Label; // Join branch
        multitac[6].a1 = tac_fresh_label();

        //multitac[4].type = t_Jmp;
        //multitac[4].a1 = multitac[7].a1; // jmp to join branch
        multitac[5].type = t_Jmp;
        multitac[5].a1 = multitac[0].a1; // jmp to predicate

        multitac[2].type = t_Bt;
        multitac[2].a1 = multitac[1].a1; //if false
        multitac[2].a2 = multitac[6].a1;
        multitac[3].type = t_Bt;
        multitac[3].a1 = multitac[1].a2; //if true
        multitac[3].a2 = multitac[4].a1;

        multitac[7].type = t_Default;
        multitac[7].a1 = returnvar;
        multitac[7].a2 = "Object";
        
        for (i = 1; i < 8; i++){
            //printf("bu\n");
            tac_add_tac(taclist, multitac[i]);
            if (i == 4){
                tac_convert_rec(taclist, expr->children[1], NULL, env);
            }
            //if (i == 5){ // change this, while needs to return object
            //    tac_convert_rec(expr->children[2], returnvar, env);
            //}
        }

        return returnvar;
    case Self_dispatch:
        currtac.numparams = expr->children[1]->numchild;
        currtac.params = tac_evaluate_params(taclist, expr->children[1], env);
        currtac.type = tac_ast_to_tac_type(expr->nodetype);
        currtac.a1 = tac_fresh_temp(returnvar);
        currtac.a2 = expr->children[0]->id;
        break;
    case Dynamic_dispatch:
        currtac.numparams = expr->children[2]->numchild;
        currtac.params = tac_evaluate_params(taclist, expr->children[2], env);
        tac_convert_rec(taclist, expr->children[0], NULL, env);
        currtac.type = tac_ast_to_tac_type(expr->nodetype);
        currtac.a1 = tac_fresh_temp(returnvar);
        currtac.a2 = expr->children[1]->id; 

        currtac.a3 = expr->children[0]->outtype; //experimental
        //printf("plz: %s\n", expr->children[0]->outtype);
        break;
    case Static_dispatch:
        currtac.numparams = expr->children[3]->numchild;
        currtac.params = tac_evaluate_params(taclist, expr->children[3], env);
        tac_convert_rec(taclist, expr->children[0], NULL, env);
        currtac.type = tac_ast_to_tac_type(expr->nodetype);
        currtac.a1 = tac_fresh_temp(returnvar);
        currtac.a2 = expr->children[2]->id;
        currtac.a3 = expr->children[1]->id;
        break;
    case Identifier:
        currtac.type = t_Identifier;
        currtac.a2 = environment_get_store(env, expr->children[0]->id);
        
        currtac.a1 = tac_fresh_temp(returnvar);
        break;
    case Assign:
        currtac.type = t_Identifier;
        currtac.a2 = tac_convert_rec(taclist, expr->children[1], environment_get_store(env, expr->children[0]->id), env);
        currtac.a1 = tac_fresh_temp(returnvar);
        break;
    case Let:
        //Let creates a new environment for each binded variable
        Environment** envarray = malloc(sizeof(Environment*) * (expr->numchild - 1));
        
        for (i = 0; i < expr->numchild - 1; i++){
            envarray[i] = environment_new(1);
            strcpy(envarray[i]->idmap[0].id, expr->children[i]->children[0]->id);
            strcpy(envarray[i]->idmap[0].type, expr->children[i]->children[1]->id);
            //printf("%s\n",expr->children[i]->children[0]->id);
        }
        //Concat the first binding to the original gamma, then each gamma after contains one more binded variable
        
        for (i = 0; i < expr->numchild - 1; i++){
            if (i == 0){
                currenv = env;
            }
            else{
                currenv = envarray[i-1];
            }
            envarray[i]->idmap->store = tac_convert_rec(taclist, expr->children[i], NULL, currenv);

            tempenv = environment_cat(envarray[i], currenv);

            environment_free(envarray[i]);
            envarray[i] = tempenv;
        }

        returnvar = tac_convert_rec(taclist, expr->children[expr->numchild - 1], returnvar, tempenv);

        for (i = 0; i < expr->numchild - 1; i++){
            environment_free(envarray[i]);
        }
        free(envarray);
        return returnvar;
    case Let_binding_init:
        returnvar = tac_fresh_temp(returnvar);
        tac_convert_rec(taclist, expr->children[2], returnvar, env);
        return returnvar;
    case Let_binding_no_init:
        returnvar = tac_fresh_temp(returnvar);
        currtac.type = t_Default;
        currtac.a1 = returnvar;
        currtac.a2 = expr->children[1]->id;
        //return returnvar;
        break;
    case Case:
        returnvar = tac_fresh_temp(returnvar);
        casevar = tac_fresh_temp(NULL);
        templabel = tac_fresh_label();
        labelc--; //We'll edit this templabel later to be a bigger label# (to make cfg implementation easier)
        labellist = malloc(sizeof(char*)* expr->children[1]->numchild);
        typelist = malloc(sizeof(char*)* (expr->children[1]->numchild + 1));
        memset(typelist, 0, sizeof(char*)* (expr->children[1]->numchild + 1));

        currtac.type = t_Case;
        //currtac.expr = expr;
        currtac.a1 = tac_convert_rec(taclist, expr->children[0], casevar, env);
        tac_add_tac(taclist, currtac);
        for (i = 0; i < expr->children[1]->numchild; i++){
            labellist[i] = tac_fresh_label();
            typelist[i] = expr->children[1]->children[i]->children[1]->id;

            currtac.type = t_Caseinner;
            currtac.a1 = casevar;
            currtac.a2 = expr->children[1]->children[i]->children[1]->id;
            currtac.a3 = labellist[i];
            tac_add_tac(taclist, currtac);
        }
        int index;
        int flag;
        for (i = 0; parentmap[i].id != NULL; i++){
            flag = 0;
            for (j = 0; typelist[j] != NULL; j++){
                if (strcmp(parentmap[i].id, typelist[j]) == 0){
                    flag = 1;
                }
            }
            if (flag) continue;
            index = map_nearest_ancestor(parentmap, typelist, parentmap[i].id);
            if (index != -1){
                currtac.type = t_Caseinner;
                currtac.a1 = casevar;
                currtac.a2 = parentmap[i].id;
                currtac.a3 = labellist[index];
                tac_add_tac(taclist, currtac);
            }
            
        }
        currtac.type = t_Esac;
        currtac.a1 = templabel;
        currtac.a2 = NULL;
        currtac.a3 = NULL;
        tac_add_tac(taclist, currtac);

        for (i = 0; i < expr->children[1]->numchild; i++){

            newenv = environment_new(1);
            strcpy(newenv->idmap->id, expr->children[1]->children[i]->children[0]->id);
            //strcpy(newenv->idmap->type, expr->children[1]->children[i]->children[1]->id);
            newenv->idmap->store = casevar;
            currenv = environment_cat(newenv, env);
            
            currtac.type = t_Label;
            currtac.a1 = labellist[i];
            currtac.a2 = NULL;
            currtac.a3 = NULL;
            //strcpy(currtac.a1, newenv->idmap->type);
            tac_add_tac(taclist, currtac);

            tac_convert_rec(taclist, expr->children[1]->children[i], returnvar, currenv);
            
            environment_free(newenv);
            environment_free(currenv);
            currtac.type = t_Jmp;
            currtac.a1 = templabel;
            currtac.a2 = NULL;
            currtac.a3 = NULL;
            tac_add_tac(taclist, currtac);
        }
        currtac.type = t_Label;
        currtac.a1 = templabel;
        currtac.a2 = NULL;
        currtac.a3 = NULL;
        tac_add_tac(taclist, currtac);
        //break;
        char* tl = tac_fresh_label();
        strcpy(templabel, tl);
        free(tl);

        return returnvar;
    case Case_inner:
        return tac_convert_rec(taclist, expr->children[2], returnvar, env);
    case Expr:
        expr->children[0]->linenum = expr->linenum;
        //printf("expr line: %i\n", expr->linenum);
    default: //case: block || expr
        for (i = 0; i < expr->numchild-1; i++){
            tac_convert_rec(taclist, expr->children[i], NULL, env);
        }

        return tac_convert_rec(taclist, expr->children[expr->numchild-1], returnvar, env);
    }
    tac_add_tac(taclist, currtac);
    return currtac.a1;
}

void tac_add_tac(TAC_List* taclist, TAC tac){
    //memcpy(&out[outn], &tac, sizeof(TAC));
    //printf("outn: %i, type: %i\n",outn, tac.type);
    //outn++;
    
    //if (outn >= 50000){
    //    printf("over 50k\n");
    //}
    int i, j;
    for (i = 0; taclist->tac[i].a1 != NULL; i++){
    }
    if (i >= taclist->length-1){
        //expand
        //printf("expandering\n");
        //for (j = 0; j < taclist->length; j++){
        //    printf("expand i: %i, type: %i, linenum: %i, num: %i\n", j, taclist->tac[j].type, taclist->tac[j].linenum, taclist->tac[j].numparams);
        //}
        TAC* temptac = malloc(sizeof(TAC) * taclist->length * 2);
        memset(temptac, 0, sizeof(TAC) * taclist->length * 2);
        memcpy(temptac, taclist->tac, sizeof(TAC) * taclist->length);
        free(taclist->tac);
        taclist->tac = temptac;
        taclist->length = taclist->length*2;
    }
    memcpy(&taclist->tac[i], &tac, sizeof(TAC));
    
    //printf("i : %i, type: %i, typpe: %i\n", i, taclist->tac[i].type, tac.type);
    

}

char* tac_fresh_temp(char* retvar){
    if (retvar) return retvar;
    char* ret = malloc(sizeof(char)* BUFMAX);
    memset(ret, 0, sizeof(char)* BUFMAX);
    sprintf(ret, "t$%i", tempc);
    tempc++;
    return ret;
}

char* tac_fresh_label(){
    char* ret = malloc(sizeof(char)* BUFMAX);
    memset(ret, 0, sizeof(char)* BUFMAX);
    if (currclass && currmethod){
        sprintf(ret, "%s_%s_%i", currclass, currmethod, labelc);
    }
    else if (currclass && currattribute){
        sprintf(ret, "%s$%s$%i", currclass, currattribute, labelc);
    }
    else{
        sprintf(ret, "label%i", labelc);
    }
    labelc++;
    return ret;
}

char** tac_evaluate_params(TAC_List* taclist, AST* args, Environment* env){
    if (!args) return NULL;
    int n = args->numchild;
    if (!n) return NULL;
    char** params = malloc(sizeof(char*) * n);
    int i;
    for (i = 0; i < n; i++){
        params[i] = tac_fresh_temp(NULL);
        tac_convert_rec(taclist, args->children[i], params[i], env);
    }
    return params;
}

tac_type tac_ast_to_tac_type(ast_type a){
    switch(a){
    case Dynamic_dispatch:
        return t_Dynamic_dispatch;
    case Self_dispatch:
        return t_Self_dispatch;
    case Static_dispatch:
        return t_Static_dispatch;
    case New:
        return t_New;
    case Isvoid:
        return t_Isvoid;
    case Plus:
        return t_Plus;
    case Minus:
        return t_Minus;
    case Times:
        return t_Times;
    case Divide:
        return t_Divide;
    case Negate:
        return t_Negate;
    case Lt:
        return t_Lt;
    case Le:
        return t_Le;
    case Eq:
        return t_Eq;
    case Not:
        return t_Not;
    case Integer:
        return t_Integer;
    case String:
        return t_String;
    default:
        //printf("uhoh\n");
        return(t_Label);
    }
}

char* tac_op(tac_type t){
    switch (t){
    case t_New:
        return "new";
    case t_Isvoid:
        return "isvoid";
    case t_Plus:
        return "+";
    case t_Minus:
        return "-";
    case t_Times:
        return "*";
    case t_Divide:
        return "/";
    case t_Negate:
        return "~";
    case t_Lt:
        return "<";
    case t_Le:
        return "<=";
    case t_Eq:
        return "=";
    case t_ptLt:
        return "pt<";
    case t_ptLe:
        return "pt<=";
    case t_ptEq:
        return "pt=";
    case t_Not:
        return "not";
    case t_Integer:
        return "int";
    case t_String:
        return "string";
    case t_Bool:
        return "bool";
    case t_Return:
        return "return";
    case t_Label:
        return "label";
    case t_Bt:
        return "bt";
    case t_Jmp:
        return "jmp";
    case t_Dynamic_dispatch:
    case t_Self_dispatch:
    case t_Static_dispatch:
        return "call";
    case t_Identifier:
        return "";
    case t_Default:
        return "default";
    case t_Case:
        return "caseon";
    case t_Caseinner:
        return "case";
    case t_Esac:
        return "esac";
    default:
        //printf("oops\n");
        return "";
    }
}

void tac_print_tac(FILE* outf, TAC_List* taclist, int start){
    int i, j;
    TAC* tac = taclist->tac;
    //printf("%i\n", outn);
    for (i = start; tac[i].a1 != NULL; i++){
        //printf("i: %i, tactype: %i\n", i, tac[i].type);
        //printf("%i\n", i);
        //fprintf(outf, "class_map\n");
        //fprintf(outf, "dead: %i |", tac[i].dead);
        //fprintf(outf, " type: %i |", tac[i].type);
        if (tac[i].type == 0){
            fprintf(outf, "NULL type (not good)\n");
        }
        else if (tac[i].type == t_Return ||
            tac[i].type == t_Label ||
            tac[i].type == t_Jmp ||
            tac[i].type == t_Case){
            //printf("a\n");
            fprintf(outf, "%s %s\n", tac_op(tac[i].type), tac[i].a1);
            //printf("aa\n");
        }
        else if (tac[i].type == t_Bt){
            //printf("b\n");
            fprintf(outf, "bt %s %s\n", tac[i].a1, tac[i].a2);
            //printf("bb\n");
        }
        else if (tac[i].type == t_Identifier){
            fprintf(outf, "%s <- %s\n", tac[i].a1, tac[i].a2);
        }
        else if (tac[i].type == t_Caseinner){
            fprintf(outf, "case %s : %s jmp %s\n", tac[i].a1, tac[i].a2, tac[i].a3);
        }
        else if (tac[i].type == t_Esac){
            fprintf(outf, "%s\n", tac_op(tac[i].type));
        }
        else if (tac[i].type == t_String){
            fprintf(outf, "%s <- %s \n%s\n", tac[i].a1, tac_op(tac[i].type), tac[i].a2);
        }
        else{
            //printf("c %i\n", tac[i].type);
            fprintf(outf, "%s <- %s %s", tac[i].a1, tac_op(tac[i].type), tac[i].a2);
            if (tac[i].a3 && tac[i].type != t_Static_dispatch && tac[i].type != t_Dynamic_dispatch && tac[i].type != t_Self_dispatch){
                fprintf(outf, " %s", tac[i].a3);
            }
            if (tac[i].params){
                for (j = 0; j < tac[i].numparams; j++){
                    fprintf(outf, " %s", tac[i].params[j]);
                }
            }
            fprintf(outf, "\n");
            //printf("cc\n");
        }
    }
}

TAC* tac_get_tac(){
    return out;
}

TAC_List* taclist_get_segment(TAC_List* taclist, int start, int end){
    TAC_List* retlist = malloc(sizeof(TAC_List));
    retlist->length = end-start + 1;
    //printf("taclist len: %i\n", retlist->length);
    retlist->tac = malloc(sizeof(TAC)*retlist->length);
    memset(&retlist->tac[retlist->length-1], 0, sizeof(TAC)); //set last element to NULL
    memcpy(retlist->tac, &taclist->tac[start], sizeof(TAC)*(end-start)); //copy from taclist

    return retlist;
}

CFG* cfg_from_label(CFG_List* cfglist, char* label){
    int i;
    for (i = 0; i < cfglist->length; i++){
        if (strcmp(cfglist->cfgarray[i].label, label) == 0){
            return &cfglist->cfgarray[i];
        }
    }
    return NULL;
}

int is_backedge(char* currlabel, char* label){
    int i;
    int c = 0;
    if (!currlabel){
        printf("currlabel is NULL (bad)\n");
    }
    if (!label){
        printf("label is NULL (bad)\n");
    }
    for (i = 0; c < 2; i++){
        if (currlabel[i] == '_' || currlabel[i] == '$'){
            c++;
        }
    }
    i++;
    return atoi(&label[i]) <= atoi(&currlabel[i]);
}

void tac_compress(TAC_List* taclist){
    int i, j;
    TAC* tac = taclist->tac;
    TAC* newtac = malloc(sizeof(TAC)*taclist->length);
    
    /*for (i = 0; i < taclist->length; i++){
        if (tac[i].a1 == NULL){
            for (j = i+1; j < taclist->length; j++){
                memcpy(&tac[j-1], &tac[j], sizeof(TAC));
            }
        }
    }*/
    for (i = 0, j = 0; i < taclist->length; i++){
        if (tac[i].a1 != NULL){
            memcpy(&newtac[j], &tac[i], sizeof(TAC));
            j++;
        }
    }
    for (;j < taclist->length; j++){
        memset(&newtac[j], 0, sizeof(TAC));
    }
    free(tac);
    taclist->tac = newtac;

    /*for (i = 0, j = 0; i < taclist->length; i++){
        if (tac[i].a1 != NULL){
            if (j != i){
                memcpy(&tac[j], &tac[i], sizeof(TAC));
            }
            j++;

        }
    }
    j++;
    for (;j < taclist->length; j++){
        memset(&tac[j], 0, sizeof(TAC));
    }*/
}

CFG_Methods* tac_to_cfg(TAC_List* taclist, Map* cmap){
    int i, j, k, start, c, l;
    int count = 0;
    TAC* tac = taclist->tac;
    for (i = 0; tac[i].a1 != NULL; i++){
        if (tac[i].type == t_Return){
            count++;
        }
    }
    CFG_Methods* cfgmethods = malloc(sizeof(CFG_Methods));
    cfgmethods->nummethods = count;
    cfgmethods->cfglistarray = malloc(sizeof(CFG_List)*count);

    count = 0;
    j = 0;
    for (i = 0; tac[i].a1 != NULL; i++){
        if (tac[i].type == t_Label){
            count++;
        }
        if (tac[i].type == t_Return){
            cfgmethods->cfglistarray[j].length = count;
            cfgmethods->cfglistarray[j].cfgarray = malloc(sizeof(CFG) * count);
            memset(cfgmethods->cfglistarray[j].cfgarray, 0, sizeof(CFG) * count);
            count = 0;
            j++;
        }
    }

    k = -1;
    j = 0;
    for (i = 0; tac[i].a1 != NULL; i++){
        if (tac[i].type == t_Label){
            k++;
            cfgmethods->cfglistarray[j].cfgarray[k].label = tac[i].a1;
            //printf("ttc1\n");
            char* class = malloc(sizeof(char)*BUFMAX);
            memset(class, 0, sizeof(char)*BUFMAX);
            for (l = 0; tac[i].a1[l] != '\0'; l++){
                if (tac[i].a1[l] == '_' || tac[i].a1[l] == '$' ){
                    break;
                }
                class[l] = tac[i].a1[l];
                //printf("l: %i, c: %c\n", l, class[l]);
            }
            //printf("ttc2\n");
            //printf("tac.a1: %s\n", tac[i].a1);
            //printf("class: %s\n", class);
            cfgmethods->cfglistarray[j].cfgarray[k].cmap = map_get_map_by_id(cmap, class);
            //printf("ttc3\n");
            free(class);
            //printf("ttc4\n");

            //printf("j: %i, k: %i, Label: %s\n", j, k, tac[i].a1);
        }
        if (tac[i].type == t_Return){
            j++;
            k = -1;
        }
    }

    k = -1;
    j = 0;
    start = 0;
    CFG* currcfg;
    for (i = 0; tac[i].a1 != NULL; i++){
        //printf("i: %i\n", i);
        if (tac[i].type == t_Label){
            //if (i > 0 && tac[i-1].type == t_Label){

            //}
            k++;
            currcfg = &cfgmethods->cfglistarray[j].cfgarray[k];
            //printf(" j: %i, k: %i\n", j, k);
            start = i;            
        }
        if (tac[i].type == t_Bt){
            currcfg->branch1 = cfg_from_label(&cfgmethods->cfglistarray[j], tac[i].a2);
            i++;
            currcfg->branch2 = cfg_from_label(&cfgmethods->cfglistarray[j], tac[i].a2);
            currcfg->taclist = taclist_get_segment(taclist, start, i+1);
        }
        if (tac[i].type == t_Jmp){
            //printf("j1\n");
            currcfg->backedge = is_backedge(currcfg->label, tac[i].a1);
            //printf("j2\n");
            currcfg->branch1 = cfg_from_label(&cfgmethods->cfglistarray[j], tac[i].a1);
            //printf("j3\n");
            currcfg->taclist = taclist_get_segment(taclist, start, i+1);
            //printf("j4\n");
        }
        if (tac[i].type == t_Case){
            for (c = i+1; tac[c].type == t_Caseinner; c++){
                currcfg->casebranches++;
            }
            //printf("cc1: %i\n", currcfg->casebranches);

            currcfg->casebranchlist = malloc(sizeof(CFG*) * currcfg->casebranches);
            //printf("cc2\n");
            c = 0;
        }
        if (tac[i].type == t_Caseinner){
            currcfg->casebranchlist[c] = cfg_from_label(&cfgmethods->cfglistarray[j], tac[i].a3);
            c++;
        }
        if (tac[i].type == t_Esac){
            currcfg->taclist = taclist_get_segment(taclist, start, i+1);
        }
        if (tac[i].type == t_Return){
            currcfg->taclist = taclist_get_segment(taclist, start, i+1);
            j++;
            k = -1;
        }
    }
    free(taclist);
    //printf("ttc done\n");
    return cfgmethods;
}

TAC_List* cfg_to_tac(CFG_Methods* cfgmethods){
    TAC_List* taclist;
    
    int i, j, len = 0;
    //printf("c1\n");
    for (i = 0; i < cfgmethods->nummethods; i++){
        //printf("i: %i\n", i);
        for (j = 0; j < cfgmethods->cfglistarray[i].length; j++){
            //printf("j: %i\n", j);
            len += cfgmethods->cfglistarray[i].cfgarray[j].taclist->length;
            //printf("tac: %i\n", cfgmethods->cfglistarray[i].cfgarray[j].taclist->tac[0].type);
            //tac_print_tac(stdout, cfgmethods->cfglistarray[i].cfgarray[j].taclist, 0);
        }
    }
    //printf("c1.1\n");
    taclist = malloc(sizeof(TAC_List));
    taclist->length = len+1;
    taclist->tac = malloc(sizeof(TAC)*taclist->length);
    //printf("c2\n");
    len = 0;
    for (i = 0; i < cfgmethods->nummethods; i++){
        for (j = 0; j < cfgmethods->cfglistarray[i].length; j++){
            cfgmethods->cfglistarray[i].cfgarray[j].taclist->length;
            memcpy(
                &taclist->tac[len], 
                cfgmethods->cfglistarray[i].cfgarray[j].taclist->tac, 
                sizeof(TAC)*cfgmethods->cfglistarray[i].cfgarray[j].taclist->length);

            len += cfgmethods->cfglistarray[i].cfgarray[j].taclist->length;
        }
        
    }
    //printf("c3\n");

    tac_compress(taclist);
    cfgmethods_free(cfgmethods);
    //printf("ctt done\n");
    return taclist;
}

void taclist_free(TAC_List* taclist){
    free(taclist->tac);
    free(taclist);
}

void cfgmethods_free(CFG_Methods* cfgmethods){
    int i, j;
    for (i = 0; i < cfgmethods->nummethods; i++){
        for (j = 0; j < cfgmethods->cfglistarray[i].length; j++){
            //printf("f1\n");
            free(cfgmethods->cfglistarray[i].cfgarray[j].taclist);
            if (cfgmethods->cfglistarray[i].cfgarray[j].casebranchlist){
                //printf("f1.5\n");
                free(cfgmethods->cfglistarray[i].cfgarray[j].casebranchlist);
                cfgmethods->cfglistarray[i].cfgarray[j].casebranchlist = NULL;
            }
            //printf("f2\n");
        }
        //printf("f3\n");
        free(cfgmethods->cfglistarray[i].cfgarray);
        //printf("f4\n");
    }
    //printf("f5\n");
    free(cfgmethods->cfglistarray);
    //printf("f6\n");
    free(cfgmethods);
    //printf("f7\n");
}

void livevars_print(FILE* outf, Livevars* livevars){
    int i;
    fprintf(outf, "Livevars:");
    for (i = 0; livevars->vars[i] != NULL; i++){
        fprintf(outf, " %s,", livevars->vars[i]);
    }
    fprintf(outf, "\n");
}

void cfgmethods_print(FILE* outf, CFG_Methods* cfgmethods){
    int i, j, k;
    CFG* cfg;
    for (i = 0; i < cfgmethods->nummethods; i++){
        for (j = 0; j < cfgmethods->cfglistarray[i].length; j++){
            cfg = &cfgmethods->cfglistarray[i].cfgarray[j];
            livevars_print(outf, cfg->livetop);

            tac_print_tac(outf, cfg->taclist, 0);
            if (cfg->branch1){
                printf("branch1: %s\n", cfg->branch1->label);
            }
            if (cfg->branch2){
                printf("branch2: %s\n", cfg->branch2->label);
            }
            printf("------------\n");
        }
    }
}

int taclist_has_input(TAC_List* taclist){
    int i;
    TAC* tac = taclist->tac;
    for (i = 0; tac[i].a1 != NULL; i++){
        if (tac[i].type == t_Self_dispatch ||
            tac[i].type == t_Dynamic_dispatch ||
            tac[i].type == t_Static_dispatch){
            if (strcmp(tac[i].a2, "in_int") == 0 ||
                strcmp(tac[i].a2, "in_string") == 0){
                return 1;
            }
        }
    }
    return 0;
}