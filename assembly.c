#include <stdarg.h>
#include "assembly.h"

typedef enum{
    Divide_By_Zero,
    Dispatch_On_Void,
    Case_On_Void,
    Case_Without_Branch
}Errorcode;

typedef struct{
    char* id;
    char* data;
}Astring;

Astring stringlist[10000];
int stringi;

FILE* goutf;

//Using registers as variables, so that I could do some potential register allocation in the future
char* rax = "%rax";
char* rdx = "%rdx";
char* rbx = "%rbx";
char* rbp = "%rbp";
char* rsp = "%rsp";
char* rself = "%r12";
char* rretv = "%r13";
const int intoffset = 24;
char* rdi = "%rdi";
char* rsi = "%rsi";

int selfflag = 1;

//classtagmap holds which class gets which tag number
Map* classtagmap;

void asm_print(const char *fmt, ...);
void assembly_convert_tac(TAC* tac, int start, int end, Map* impmap, Map* classmap, char* currclass, char* currmethod, FILE* outf);
void assembly_new(char* class);

//Sets up the default assembly strings
void assembly_stringlist_init(){
    memset(stringlist, 0, sizeof(stringlist));
    stringi = 3;
    stringlist[0].id = "the.empty.string";
    stringlist[0].data = "";
    stringlist[1].id = "percent.d";
    stringlist[1].data = "%ld";
    stringlist[2].id = "percent.ld";
    stringlist[2].data = " %ld";
    stringlist[3].id = "string1";
    stringlist[3].data = "abort\\n";
    stringlist[4].id = "string2";
    stringlist[4].data = "ERROR: 0: Exception: String.substr out of range\\n";
    //stringlist[4].id = "string3";
    //stringlist[4].data = "ERROR: 0: Exception: String.substr out of range\\n";

}

//Used to add an assembly string, returns the label that references that string
char* assembly_stringlist_new(char* str){
    int n = stringi+2;
    stringlist[n].data = malloc(sizeof(char)*BUFMAX);
    stringlist[n].id = malloc(sizeof(char)*BUFMAX);


    strcpy(stringlist[n].data, str);

    //char buffer[32];
    //memset(buffer, 0, sizeof(buffer));
    //itoa(stringi, buffer, 10);
    sprintf(stringlist[n].id, "string%i", stringi);
    //strcpy(stringlist[n].id, "string");
    //strcat(stringlist[n].id, buffer);

    stringi++;
    return stringlist[n].id;
}

//Called by main, the tac has all methods already converted, so this function will have to convert attributes to tac on its own
void assembly_print(TAC* tac, Map* impmap, Map* classmap, FILE* outf){
    int i, j, k;
    ImpMethod* met;
    Attribute* att;
    Map* cmap;
    FILE* fptr;
    char* str;
    char buffer[BUFMAX];
    char match[32];
    int* intp;
    int currtag = 20;
    goutf = outf;
    assembly_stringlist_init();
    classtagmap = malloc(sizeof(Map) * map_len(classmap));
    memset(classtagmap, 0, sizeof(Map) * map_len(classmap));
    //setup classtagmap
    for (i = 0; classmap[i].id != NULL; i++){
        classtagmap[i].id = classmap[i].id;
        intp = malloc(sizeof(int));
        //builtin class tags
        if (strcmp(classmap[i].id, "Object") == 0){
            *intp = 12;
        }
        else if (strcmp(classmap[i].id, "Int") == 0){
            *intp = 1;
        }
        else if (strcmp(classmap[i].id, "IO") == 0){
            *intp = 10;
        }
        else if (strcmp(classmap[i].id, "Bool") == 0){
            *intp = 0;
        }
        else if (strcmp(classmap[i].id, "String") == 0){
            *intp = 3;
        }
        else{ //for non builtin classes
            *intp = currtag++;
        }

        classtagmap[i].data = intp;
    }

    //print all vtables in assembly
    for (i = 0; impmap[i].id != NULL; i++){
        fprintf(outf, "                        ## ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n");
        fprintf(outf, ".globl %s..vtable\n", impmap[i].id);
        fprintf(outf, "%s..vtable:           ## virtual function table for %s\n", impmap[i].id, impmap[i].id);
        //printf("vtable for %s\n",impmap[i].id);
        met = (ImpMethod*) impmap[i].data;
        str = assembly_stringlist_new(impmap[i].id);
        fprintf(outf, "                        .quad %s\n", str);
        for (j = 0; j < impmap[i].numchild; j++){
            fprintf(outf, "                        .quad %s.%s\n", met[j].class, met[j].id);
        }
    }


    fptr = fopen("internaldata", "r");
    if (!fptr){
        printf("error opening file\n");
        return;
    }

    //print all methods in assembly
    for (i = 0; impmap[i].id != NULL; i++){
        met = (ImpMethod*) impmap[i].data;
        for (j = 0; j < impmap[i].numchild; j++){
            if (strcmp(met[j].class, impmap[i].id) == 0){
                if (strcmp(met[j].id, ".new") == 0 && 
                    strcmp(impmap[i].id, "Object") != 0 &&
                    strcmp(impmap[i].id, "IO") != 0 &&
                    strcmp(impmap[i].id, "Int") != 0 &&
                    strcmp(impmap[i].id, "Bool") != 0 &&
                    strcmp(impmap[i].id, "String") != 0 
                ){ //build the new method for non-internal classes
                    cmap = map_get_map_by_id(classmap, impmap[i].id);

                    fprintf(outf, ".globl %s.%s\n", impmap[i].id, met[j].id);
                    fprintf(outf, "%s.%s:              ## constructor for %s\n", impmap[i].id, met[j].id, impmap[i].id);
                    asm_print("pushq %s", rbp);
                    asm_print("movq %s, %s", rsp, rbp);
                    asm_print("## stack room for temporaries: 2");
                    asm_print("movq $16, %s", rax);
                    asm_print("subq %s, %s", rax, rsp);

                    asm_print("andq $0xFFFFFFFFFFFFFFF0, %s", rsp);
                    asm_print("movq $%i, %s", cmap->numchild+3, rdi); //calloc param for number of x
                    asm_print("movq $8, %s", rsi); //calloc param for size of x
                    asm_print("call calloc");
                    asm_print("movq %s, %s", rax, rself);

                    asm_print("## store class tag, object size and vtable pointer");
                    asm_print("movq $%i, %s", *(int*)(map_get_map_by_id(classtagmap, impmap[i].id)->data), rax); //set to specific class tag
                    asm_print("movq %s, 0(%s)", rax, rself);
                    asm_print("movq $%i, %s", cmap->numchild+3, rax); // object size
                    asm_print("movq %s, 8(%s)", rax, rself);
                    asm_print("movq $%s..vtable, %s", impmap[i].id, rax); //vtable for the object
                    asm_print("movq %s, 16(%s)", rax, rself );

                    asm_print("## initialize attributes");
                    //set attributes to default values
                    att = (Attribute*)cmap->data;
                    for (k = 0; k < cmap->numchild; k++){
                        //printf("%s\n", att[k].id);
                        //printf("%s\n", att[k].type);
                        if (strcmp(att[k].type, "Int") == 0 ||
                            strcmp(att[k].type, "Bool") == 0 ||
                            strcmp(att[k].type, "String") == 0 ){
                            assembly_new(att[k].type);
                        }
                        else{
                            asm_print("## init %s", att[k].type);
                            asm_print("movq $0, %s", rretv);
                        }
                        asm_print("movq %s, %i(%s)", rretv, (k+3)*8, rself );
                    }
                    //convert initializer expressions into tac, then into assembly
                    for (k = 0; k < cmap->numchild; k++){
                        if (att[k].init == Attribute_no_init){
                            
                        }
                        else{
                            asm_print("## initializer expr for %s", att[k].id);
                            int start = tac_convert_expr(att[k].expr, att[k].id); 
                            
                            int end;
                            for (end = start; tac[end].a1 != NULL; end++){
                                //printf("line: %i, type: %i\n", tac[end].linenum, tac[end].type);
                            }
                            //printf("%s: \n", att[k].id);
                            //printf("start: %i\n", start);
                            //printf("end: %i\n", end);
                            asm_print("pushq %s", rbp);
                            asm_print("pushq %s", rself);
                            //asm_print("pushq %s", rbp);
                            selfflag = 0;
                            assembly_convert_tac(tac, start, end, impmap, classmap, impmap[i].id, NULL, outf);
                            selfflag = 1;
                            asm_print("movq %s, %s", rbp, rsp);
                            asm_print("popq %s", rbp);
                            //asm_print("popq %s", rbp);
                            asm_print("popq %s", rself);
                            asm_print("popq %s", rbp);

                        }
                        
                    }
                    asm_print("movq %s, %s", rself, rretv ); //return the new object
                    asm_print("## return address handling");
                    asm_print("movq %s, %s", rbp, rsp);
                    asm_print("popq %s", rbp);
                    asm_print("ret");
                }
                else if (met[j].expr->nodetype == Internal){ //if internal, find in internaldata
                    //goto start of internaldata
                    rewind(fptr);
                    
                    
                    sprintf(match, ".globl %s.%s\n", impmap[i].id, met[j].id);
                    while (strcmp(match, buffer) != 0){
                        fgets(buffer, BUFMAX, fptr); //call fgets until you reach the desired method
                    }
                    fprintf(outf, "%s", buffer);
                    fgets(buffer, BUFMAX, fptr);
                    if (strcmp(buffer, ".globl Bool..new\n") == 0){ //sometimes it just goes to the first line idk why, it make no sense, so this specifically checks to see if it went to the first line and then just does it again
                        while (strcmp(match, buffer) != 0){
                            fgets(buffer, BUFMAX, fptr);
                        }
                        fgets(buffer, BUFMAX, fptr);
                    }
                    

                    while (strncmp(".globl ", buffer, 7) != 0 || strncmp(".globl l", buffer, 8) == 0){
                        fprintf(outf, "%s", buffer);
                        fgets(buffer, BUFMAX, fptr);
                    }
                    //printf("%s", match);

                    //fclose(fptr);
                }
                else{
                    //find in tac, convert tac to x86
                    int start = -1, end = -1;
                    sprintf(match, "%s_%s_0", impmap[i].id, met[j].id);
                    for (k = 0; tac[k].a1 != NULL; k++){
                        if (start == -1 && tac[k].type == t_Label && (strcmp(match, tac[k].a1) == 0)){
                            start = k;
                        }
                        else if (end == -1 && start != -1 && tac[k].type == t_Return /*&& (strncmp(match, tac[k].a1, strlen(match)-1) != 0)*/){
                            end = k;
                        }
                    }
                    //printf("match: %s\n", match);
                    //printf("start: %i\n", start+1);
                    //printf("end: %i\n", end);
                    if (end == -1){
                        end = k;
                    }
                    //printf("non internal: %s.%s\n", impmap[i].id, met[j].id);
                    //printf("start: %i, end: %i\n", start, end);

                    fprintf(outf, ".globl %s.%s\n", impmap[i].id, met[j].id);
                    fprintf(outf, "%s.%s:              ## method definition\n", impmap[i].id, met[j].id);

                    assembly_convert_tac(tac, start+1, end, impmap, classmap, impmap[i].id, met[j].id, outf);
                }

                /*  .globl Object.copy.end
                    Object.copy.end:        ## method body ends
                                            ## return address handling
                                            movq %rbp, %rsp
                                            popq %rbp
                                            ret
                                            ## ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;*/
                //add the function ending for not .new functions
                if (strcmp(met[j].id , ".new") != 0){
                    fprintf(outf, ".globl %s.%s.end\n", impmap[i].id, met[j].id);
                    fprintf(outf, "%s.%s.end:        ## method body ends\n", impmap[i].id, met[j].id);
                    fprintf(outf, "                        ## return address handling\n");
                    fprintf(outf, "                        movq %%rbp, %%rsp\n");
                    fprintf(outf, "                        popq %%rbp\n");
                    fprintf(outf, "                        ret\n");
                    fprintf(outf, "                        ## ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n");
                }
            }
        }
    }
    fclose(fptr);




    for (i = 0; stringlist[i].id != NULL; i++){
        fprintf(outf, ".globl %s\n", stringlist[i].id);
        fprintf(outf, "%s:\t\t# \"%s\"\n", stringlist[i].id, stringlist[i].data);
        for (j = 0; stringlist[i].data[j] != '\0'; j++){
            fprintf(outf, ".byte %*i # '%c'\n", 3, stringlist[i].data[j], stringlist[i].data[j]);
        }
        fprintf(outf, ".byte 0\n");
        fprintf(outf, "\n");

    }

    
    fptr = fopen("internaldata", "r");
    if (!fptr){
        printf("error opening file\n");
        return;
    }

    
    while (strcmp("helper_functions\n", buffer) != 0){
        fgets(buffer, BUFMAX, fptr);
    }
    while (fgets(buffer, BUFMAX, fptr)){
        fprintf(outf, "%s", buffer);
    }
    fclose(fptr);
    //printf("done\n");
}

//Formats the assembly code (to make writing all the prints less tedious)
void asm_print(const char *fmt, ...)
{
    char buffer[512];
    memset (buffer, 0, sizeof(buffer));
    va_list args;
    strcat(buffer, "                        ");
    strcat(buffer, fmt);
    strcat(buffer, "\n");

    va_start(args, fmt);
    vfprintf(goutf, buffer, args);
    va_end(args);
    return;
}

//A function to handle all kinds of Cool error checking in assembly
void assembly_error_check(Errorcode e, char* r, int line);

//A function to abstract int operations, since they are similar
void assembly_int_op(tac_type op, int i1, int i2, int line);

//Take a piece of tac (from index 'start' to 'end') and print assembly code to a file
void assembly_convert_tac(TAC* tac, int start, int end, Map* impmap, Map* classmap, char* currclass, char* currmethod, FILE* outf){
    int i, j;
    //alloc space for temporaries
    int numtemps = 0;
    int currtemp;
    int offset = 0;
    char* str;
    char* label;
    //char buffer[BUFMAX];

    for (i = start; i < end; i++){
        currtemp = atoi(&tac[i].a1[2]); //get the # value of the temp
        if (currtemp > numtemps){
            numtemps = currtemp;
        }
    }
    numtemps++; //add 1 since t$0 counts as a temporary
    if (numtemps%2 == 1){
        numtemps++; //one more to align if necessary
    }

    /*                  pushq %rbp
                        movq %rsp, %rbp
                        movq 16(%rbp), %r12
                        ## stack room for temporaries: 2
                        movq $16, %r14
                        subq %r14, %rsp*/
    
    asm_print("pushq %s", rbp);
    asm_print("movq %s, %s", rsp, rbp);
    if (selfflag){
        
        asm_print("movq 16(%s), %s    ## self object", rbp, rself);
    }
    
    asm_print("## stack room for temporaries: %i", numtemps);
    asm_print("movq $%i, %s", numtemps*8, rax);
    asm_print("subq %s, %s", rax, rsp);
    //printf("start: %i, end: %i\n", start, end);
    for (i = start; i < end; i++){
        //printf("i: %i type: %i\n", i, tac[i].type);
        int temp1 = -1;   
        int temp2 = -1;
        int temp3 = -1;
        int temp4 = -1; // for dispatch parameters
        if (tac[i].a1){
            if (strncmp(tac[i].a1, "t$", 2) == 0){
                temp1 = atoi(&tac[i].a1[2])*8; //extract the stack offset from the tac temp var format
            }
        }
        if (tac[i].a2){
            if (strncmp(tac[i].a2, "t$", 2) == 0){
                temp2 = atoi(&tac[i].a2[2])*8;
            }
        }
        if (tac[i].a3){
            if (strncmp(tac[i].a3, "t$", 2) == 0){
                temp3 = atoi(&tac[i].a3[2])*8;
            }
        }
        //printf("i: %i, type: %i\n", i, tac[i].type);
        switch(tac[i].type){
            case t_Integer:
                assembly_new("Int");
                asm_print("movq $%i, %s", atoi(tac[i].a2), rax); //move immediate to rax
                asm_print("movq %s, 24(%s)", rax, rretv); //set Int object's value to rax
                break;
            case t_Plus:
            case t_Minus:
            case t_Times:
            case t_Divide:
                assembly_int_op(tac[i].type, -temp2, -temp3, tac[i].linenum);
                break;
            case t_Negate:
                assembly_int_op(t_Negate, -temp2, 0, tac[i].linenum);
                break;
            case t_Dynamic_dispatch: //rretv has our caller object
            case t_Static_dispatch:
            case t_Self_dispatch:
                asm_print("pushq %s", rself); //save our self object
                asm_print("pushq %s", rbp); //save our rbp, we are gonna make a function call
                for (j = 0; j < tac[i].numparams; j++){
                    if (strncmp(tac[i].params[j], "t$", 2) == 0){
                        temp4 = atoi(&tac[i].params[j][2])*8;
                        asm_print("movq %i(%s), %s", -temp4, rbp, rax);
                        asm_print("pushq %s", rax);  //push the necessary parameters
                    }
                }
                if (tac[i].type == t_Self_dispatch){
                    asm_print("pushq %s", rself); //push the self object (which is self for self dispatch)
                    asm_print("movq 16(%s), %s", rself, rax); //get vtable from self

                    tac[i].a3 = currclass; //experimental
                }
                else if (tac[i].type == t_Dynamic_dispatch){
                    assembly_error_check(Dispatch_On_Void, rretv, tac[i].linenum);
                    asm_print("pushq %s", rretv); //push the self object (caller for dynamic dispatch)
                    asm_print("movq 16(%s), %s", rretv, rax); //get vtable from caller
                    if (strcmp(tac[i].a3, "SELF_TYPE") == 0){
                        //printf("bingo\n");
                        tac[i].a3 = currclass;
                    }
                }
                else {//if (tac[i].type == t_Static_dispatch){
                    assembly_error_check(Dispatch_On_Void, rretv, tac[i].linenum);
                    asm_print("pushq %s", rretv); //push the self object (caller for dynamic/static dispatch)
                    asm_print("movq $%s..vtable, %s", tac[i].a3, rax); //get static vtable
                    //printf("a1: %s\n", tac[i].a1);
                    //printf("a2: %s\n", tac[i].a2);
                    //printf("a3: %s\n", tac[i].a3);
                }
                
                //printf("i: %i\n", i);
                temp4 = map_get_offset(impmap, tac[i].a3, tac[i].a2); //get offset for the method
                //printf("i: %i\n", i);
                if (temp4 == -1) {
                    printf("badabooey\n");
                    printf("%s\n", currclass);
                    printf("%s\n", tac[i].a2);
                }
                asm_print("movq %i(%s), %s", temp4, rax, rax); //goto the offset at the vtable
                asm_print("call *%s", rax); //call that method
                asm_print("addq $%i, %s", (tac[i].numparams+1)*8, rsp); //unallocate parameters
                asm_print("popq %s", rbp); //retrieve our rbp
                asm_print("popq %s", rself); //retrieve our self object
                break;
            case t_Bool:
                assembly_new("Bool");
                if (strcmp(tac[i].a2, "true") == 0){
                    temp2 = 1;
                }
                else{
                    temp2 = 0;
                }
                asm_print("movq $%i, %s", temp2, rax); //move immediate to rax
                asm_print("movq %s, 24(%s)", rax, rretv); //set Bool object's value to rax
                break;
            case t_String:
                assembly_new("String");
                //printf("str: %s\n", tac[i].a2);
                str = assembly_stringlist_new(tac[i].a2);
                asm_print("movq $%s, 24(%s)", str, rretv);
                break;
            case t_Bt:
                asm_print("movq %i(%s), %s", -temp1, rbp, rax); //grab Bool object 
                asm_print("movq %i(%s), %s", intoffset, rax, rax); //get the actual value    

                asm_print("cmpq $1, %s", rax);
                asm_print("je %s", tac[i].a2);
                continue;
            case t_Jmp:
                asm_print("jmp %s", tac[i].a1);
                break;
            case t_Label:
                /*  .globl l4
                    l4:*/
                asm_print(".globl %s", tac[i].a1);
                asm_print("%s:", tac[i].a1);
                break;
            case t_Not:
                assembly_new("Bool");
                asm_print("movq %i(%s), %s", -temp2, rbp, rax); //grab Bool object 1
                asm_print("movq %i(%s), %s", intoffset, rax, rax); //get the actual value

                asm_print("xor $1, %s", rax);

                asm_print("movq %s, 24(%s)", rax, rretv);
                break;
            case t_Eq:
            case t_Le:
            case t_Lt:
                asm_print("pushq %s", rself); //save our self object
                asm_print("pushq %s", rbp); //save our rbp, we are gonna make a function call

                asm_print("movq %i(%s), %s", -temp2, rbp, rax); //push param 1
                asm_print("pushq %s", rax);
                asm_print("movq %i(%s), %s", -temp3, rbp, rax); //push param 2
                asm_print("pushq %s", rax);
                asm_print("pushq %s", rself); //push self param (definitely not necessary, but the reference compiler does it)
                switch(tac[i].type){
                    case t_Eq:
                        asm_print("call eq_handler");
                        break;
                    case t_Le:
                        asm_print("call le_handler");
                        break;
                    case t_Lt:
                        asm_print("call lt_handler");
                        break;
                }
                asm_print("addq $24, %s", rsp);
                asm_print("popq %s", rbp);
                asm_print("popq %s", rself);
                break;
            case t_Default:
                if (strcmp(tac[i].a2, "Int") == 0 ||
                    strcmp(tac[i].a2, "Bool") == 0 ||
                    strcmp(tac[i].a2, "String") == 0){
                    assembly_new(tac[i].a2);
                }
                else{
                    //printf("Undefined behavior for default!!!\n");
                    asm_print("movq $0, %s", rretv); //Default value for objects is void
                }
                break;
            case t_Identifier:
                
                if (temp2 != -1){
                    asm_print("movq %i(%s), %s", -temp2, rbp, rretv);
                }
                else{
                    
                    //char* type = map_get_type(map_get_map_by_id(impmap, currclass), currmethod, tac[i].a2);
                    temp2 = map_get_param_offset(map_get_map_by_id(impmap, currclass), currmethod, tac[i].a2);
                    if (temp2 != -1){
                        //printf("hurray\n");
                        asm_print("movq %i(%s), %s", temp2, rbp, rretv);
                    }
                    else{
                        //printf("a2: %s\n", tac[i].a2);
                        temp2 = map_get_offset(classmap, currclass, tac[i].a2);
                        if (temp2 != -1){
                            asm_print("movq %i(%s), %s", temp2, rself, rretv);
                        }
                        else{//should be self here I think
                            if (strcmp(tac[i].a2, "self") != 0){
                                printf("not self here which is a big nono\n");
                                printf("a1: %s, a2: %s, i: %i\n", tac[i].a1, tac[i].a2, i);

                            }
                            asm_print("movq %s, %s", rself, rretv);
                        }

                    }
                    
                }
                break;
            case t_Return:
                //printf("skipping return\n");
                break;
            case t_New:
                assembly_new(tac[i].a2);
                break;
            case t_Isvoid:
                assembly_new("Bool");
                asm_print("movq %i(%s), %s", -temp2, rbp, rax);
                asm_print("cmpq $0, %s", rax);
                label = tac_fresh_label();
                asm_print("jne %s", label);
                asm_print("movq $1, 24(%s)", rretv);
                asm_print(".globl %s", label);
                asm_print("%s:", label);
                break;
            case t_Case:
                //printf("Case 'case' not handled\n");
                asm_print("## case expression begins");
                asm_print("movq %i(%s), %s", -temp1, rbp, rax);
                //asm_print("cmpq $0, %s", rax);
                assembly_error_check(Case_On_Void, rax, tac[i].linenum);
                asm_print("movq 0(%s), %s", rax, rax);
                //jump to void case

                //exit(1);
                break;
            case t_Caseinner:
                asm_print("## case expression: compare type tags");
                asm_print("cmpq $%i, %s", *(int*)(map_get_map_by_id(classtagmap, tac[i].a2)->data), rax);
                asm_print("je %s", tac[i].a3);
                break;
            case t_Esac:
                asm_print("## case expression: error case" );
                assembly_error_check(Case_Without_Branch, NULL, tac[i].linenum);
                break;
            default:
                printf("unhandled case: %i\n", tac[i].type);
                break;
        }

        //if of the form 't$0 <-', then:
        if (temp1 != -1){
            asm_print("movq %s, %i(%s)", rretv, -temp1, rbp); //save pointer to object as temporary
        }
        else{
            //printf("%s\n", tac[i].a1);
            //asm_print("movq %s, %i(%s)", rretv, -temp1, rbp); //save to attribute

            temp2 = map_get_param_offset(map_get_map_by_id(impmap, currclass), currmethod, tac[i].a1);
            if (temp2 != -1){
                //printf("hurray\n");
                asm_print("movq %s, %i(%s)", rretv, temp2, rbp );
            }
            else{
                temp2 = map_get_offset(classmap, currclass, tac[i].a1);
                if (temp2 != -1){
                    asm_print("movq %s, %i(%s)", rretv, temp2, rself);
                }
                
            }
        }
        
    }
}

void assembly_new(char* class){
    
    asm_print("## new %s", class);
    asm_print("pushq %s", rbp); //save rbp
    asm_print("pushq %s", rself); //save self
    if (strcmp(class, "SELF_TYPE") != 0){
        asm_print("movq $%s..new, %s", class, rax); 
    }
    else{
        asm_print("movq 16(%s), %s", rself, rax); //get vtable of self object
        asm_print("movq 8(%s), %s", rax, rax); //get the .new function from vtable
    }
    
    
    asm_print("call *%s", rax);     //call new
    asm_print("popq %s", rself); //retrieve self
    asm_print("popq %s", rbp); //retrieve rbp
}

void assembly_int_op(tac_type op, int i1, int i2, int line){
    char* str, *label;
    char buffer[BUFMAX];
    memset(buffer, 0, sizeof(buffer));
    assembly_new("Int"); //rretv contains the pointer to this int
    if (op != t_Divide){
        asm_print("movq %i(%s), %s", i1, rbp, rax); //grab Int object 1
        asm_print("movq %i(%s), %s", intoffset, rax, rax); //get the actual value
        if (op != t_Negate){
            asm_print("movq %i(%s), %s", i2, rbp, rdx); //grab Int object 2
            asm_print("movq %i(%s), %s", intoffset, rdx, rdx); //get the actual value
        }
        switch(op){
            case t_Plus:
                asm_print("addq %s, %s", rdx, rax); //add
                break;
            case t_Minus:
                asm_print("subq %s, %s", rdx, rax); //subtract
                break;
            case t_Times:
                asm_print("imul %s, %s", rdx, rax); //multiply
                break;
            case t_Negate:
                asm_print("neg %s", rax); //negate
                break;
        }
        
    }
    else{
        asm_print("movq %i(%s), %s", i1, rbp, rax); //grab Int object 1
        asm_print("movq %i(%s), %s", intoffset, rax, rax); //get the actual value
        asm_print("movq %i(%s), %s", i2, rbp, rbx); //grab Int object 2
        asm_print("movq %i(%s), %s", intoffset, rbx, rbx); //get the actual value
        
        assembly_error_check(Divide_By_Zero, rbx, line);

        //do the division
        
        asm_print("movq $0, %s", rdx); //make sure rdx is zero
        asm_print("cdq"); //call before idiv
        //asm_print("idiv %s", rbx); //divide rdx:rax by rbx
        asm_print("idiv %%ebx");
    }
    
    //assembly_new("Int"); //rretv contains the pointer to this int
    asm_print("movq %s, 24(%s)", rax, rretv); //set Int object's value to rax
}

void assembly_error_check(Errorcode e, char* r, int line){
    char *str, *label;
    char buffer[BUFMAX];
    if (r != NULL){
        asm_print("cmpq $0, %s", r); //check if dividing by zero
        label = tac_fresh_label();
        asm_print("jne %s", label);
    }
    
    //asm_print("cmpq $0, %s", r);
    switch(e){
        case Divide_By_Zero:
        sprintf(buffer, "ERROR: %i: Exception: division by zero\\n", line);
        break;
        case Dispatch_On_Void:
        sprintf(buffer, "ERROR: %i: Exception: dispatch on void\\n", line);
        break;
        case Case_On_Void:
        sprintf(buffer, "ERROR: %i: Exception: case on void\\n", line);
        break;
        case Case_Without_Branch:
        sprintf(buffer, "ERROR: %i: Exception: case without matching branch\\n", line);
        break;

    }

    

    str = assembly_stringlist_new(buffer);
    asm_print("movq $%s, %s", str, rdi);
    asm_print("andq $0xFFFFFFFFFFFFFFF0, %s", rsp);
    asm_print("call cooloutstr");
    asm_print("andq $0xFFFFFFFFFFFFFFF0, %s", rsp);
    asm_print("movq $0, %s", rdi);
    asm_print("call exit");
    if (r != NULL){
        asm_print(".globl %s", label);
        asm_print("%s:", label);
    }
}