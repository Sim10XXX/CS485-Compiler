//#include "main.h"
#include "map.h"
//#include "typecheck.h"
#include "tac.h"
#include "assembly.h"
#include "optimizer.h"

// Output file
FILE* outf;

int main(int argc, char* argv[]) {
    //printf("start\n");
    if (argc != 2) return 1;
    FILE *fptr;
    AST* ast = malloc(sizeof(AST)); //root of the ast
    //Open input file
    if (strcmp(&argv[1][strlen(argv[1])-4], "type") != 0){
        return 1;
    }
    fptr = fopen(argv[1], "r"); 
    if (!fptr){
        printf("error opening file\n");
        return 1;
    }
    char namebuffer[BUFMAX]; //name of the output file
    //changing the extention from .cl-type to .cl-tac
    memset(namebuffer, 0, sizeof(char)*BUFMAX);
    memcpy(namebuffer, argv[1], strlen(argv[1])-4);
    strcat(namebuffer, "tac");
    int i;
    char buffer[BUFMAX];
    // Initialize each map that we need
    //Map* classMap = map_initialize(ast->numchild, class_map);
    //Map* implementationMap = map_initialize(ast->numchild, implementation_map);
    //Map* parentMap = map_initialize(ast->numchild, parent_map);
    //printf("r1\n");
    while (strcmp("class_map\n", buffer) != 0){
        fgets(buffer, BUFMAX, fptr);
    }
    //printf("i1\n");
    Map* classMap = map_read_class_map(fptr);
    //printf("r2\n");
    while (strcmp("implementation_map\n", buffer) != 0){
        fgets(buffer, BUFMAX, fptr);
    }
    //printf("i1\n");
    Map* implementationMap = map_read_implementation_map(fptr);
    //printf("i2\n");
    //printf("r3\n");
    while (strcmp("parent_map\n", buffer) != 0){
        fgets(buffer, BUFMAX, fptr);
    }
    Map* parentMap = map_read_parent_map(fptr);
    //printf("r4\n");
    /*while (fgets(buffer, BUFMAX, fptr)){
        printf("%s", buffer);
    }
    return 0;*/
    fgets(buffer, BUFMAX, fptr); //Read first line of ast
    ast->linenum = 0; //root is on line 0
    ast->numchild = atoi(buffer); // First line of input gives number of classes, each class is a child of Root
    ast->nodetype = Root;
    ast->id = NULL;
    ast->children = malloc(sizeof(AST*) * ast->numchild); // Allocate space for each child ast (each class)
    for (i = 0 ; i < ast->numchild ; i++){
        ast->children[i] = malloc(sizeof(AST));
        //printf("r1\n");
        ast_recursive_read(ast->children[i], fptr, Class); // Recursively read each class
        //printf("r2\n");
    }
    //tac_init(parentMap);
    //printf("t1\n");
    //tac_convert_root(ast);
    //tac_convert_classmap(classMap);
    TAC_List* taclist = tac_convert_ast(classMap, parentMap, ast);
    //printf("t2\n");
    // Open outfile and print everything
    //for (i = 0; i < taclist->length; i++){
    //    printf("i: %i, type: %i, linenum: %i, num: %i\n", i, taclist->tac[i].type, taclist->tac[i].linenum, taclist->tac[i].numparams);
    //}
    CFG_Methods* cfgmethods = tac_to_cfg(taclist, classMap);

    if (DeadTacElimination){
        optimizer_eliminate_dead_tac(cfgmethods);
    }
    
    

    taclist = cfg_to_tac(cfgmethods);
    if (1){
        if (PA4c1){
            outf = stdout;
        }
        else{
            outf = fopen(namebuffer, "w");
        }
        tac_print_tac(outf, taclist, 0);
        fclose(outf);
        if (PA4c1){
            return 0;
        }
    }
   
    int dostaticexec = 0;
    if (CheckStaticExec){
        dostaticexec = !taclist_has_input(taclist);
    }

    memset(namebuffer, 0, sizeof(char)*BUFMAX);
    memcpy(namebuffer, argv[1], strlen(argv[1])-8);
    strcat(namebuffer, ".stemp");
    outf = fopen(namebuffer, "w");
    //printf("asmprint\n");
    assembly_print(taclist, implementationMap, classMap, outf);
    //printf("asmprintdone\n");
    fclose(outf);
    fclose(fptr);

    fptr = fopen(namebuffer, "r");
    ASM_List* asmlist = assembly_parse(fptr);
    //printf("parsed\n");
    fclose(fptr);
    if (UnreachableAsmElim){
        asmlist = optimizer_eliminate_unreachable_asm(asmlist, parentMap);
        //optimizer_eliminate_unreachable_asm(asmlist, parentMap);
    }

    memset(namebuffer, 0, sizeof(char)*BUFMAX);
    memcpy(namebuffer, argv[1], strlen(argv[1])-8);
    strcat(namebuffer, ".s");
    outf = fopen(namebuffer, "w");
    //printf("printing\n");
    asmlist_print(asmlist, outf);
    //printf("printingdone\n");
    fclose(outf);
    /*map_print(classMap, outf);
    map_print(implementationMap, outf);
    map_print(parentMap, outf);
    ast_print(ast);*/
    //printf("b\n");
    if (dostaticexec){
        char command[BUFMAX*3];
        memset(namebuffer, 0, sizeof(char)*BUFMAX);
        memcpy(namebuffer, argv[1], strlen(argv[1])-8);
        sprintf(command, "gcc -no-pie -static \"%s.s\" -o \"%s.out\"", namebuffer, namebuffer);
        int status = system(command);
        //printf("status: %i\n", status);
        if (!status){
            sprintf(command, "./\"%s.out\" > \"%s.stdoutput\"", namebuffer, namebuffer);
            status = system(command);
            //printf("status: %i\n", status);
            if (!status){
                //printf("yay\n");
                memset(namebuffer, 0, sizeof(char)*BUFMAX);
                memcpy(namebuffer, argv[1], strlen(argv[1])-8);
                strcat(namebuffer, ".stdoutput");
                FILE* outputf = fopen(namebuffer, "r");
                char c;
                int counter = 0;
                while ((c = fgetc(outputf)) != EOF){
                    counter++;
                }
                fclose(outputf);
                if (counter < 10000){
                    char buffer[BUFMAX];

                    memset(namebuffer, 0, sizeof(char)*BUFMAX);
                    memcpy(namebuffer, argv[1], strlen(argv[1])-8);
                    strcat(namebuffer, ".s");
                    outf = fopen(namebuffer, "w");

                    memset(namebuffer, 0, sizeof(char)*BUFMAX);
                    memcpy(namebuffer, argv[1], strlen(argv[1])-8);
                    strcat(namebuffer, ".stdoutput");
                    FILE* outputf = fopen(namebuffer, "r");

                    fptr = fopen("internaldata", "r");

                    while (strcmp("hello_world\n", buffer) != 0){
                        fgets(buffer, BUFMAX, fptr);
                    }

                    while (1){
                        fgets(buffer, BUFMAX, fptr);
                        counter = 0;
                        if (strcmp("helper_functions\n", buffer) != 0){
                            if (strcmp(buffer, "\t.ascii\n") == 0){
                                fprintf(outf, "\t.ascii \"");
                                while ((c = fgetc(outputf)) != EOF){

                                    if (c == '\n'){
                                        fputc('\\', outf);
                                        fputc('n', outf);
                                    }
                                    else if (c == '\\' || c == '\"'){
                                        fputc('\\', outf);
                                        fputc(c, outf);
                                    }
                                    //else if (c == '%'){
                                    //    fputc('%', outf);
                                    //    fputc('%', outf);
                                    //}
                                    else{
                                        fputc(c, outf);
                                    }
                                    counter++;
                                    if (counter > 59){
                                        fputc('\"', outf);
                                        fputc('\n', outf);
                                        fprintf(outf, "\t.ascii \"");
                                        counter = 0;
                                    }
                                }
                                fputc('\\', outf);
                                fputc('0', outf);
                                fputc('\"', outf);
                                fputc('\n', outf);
                            }
                            else{
                                fprintf(outf, "%s", buffer);
                            }
                        }
                        else{
                            break;
                        }
                        
                    }
                    
                    
                    fclose(outf);
                    fclose(outputf);
                    fclose(fptr);
                    return 0;
                }

                
            }
        }
    }
    
    return 0;
}

/*List* list_new(){
    List* l = malloc(sizeof(List));
    memset(l, 0, sizeof(List));
    return l;
}

void list_append(List* list, int d){
    if (!list) return;
    while (list->next != NULL){
        list = list->next;
    }
    list->next = list_new();
    list->next->data = d;
}*/


// Helper function to strip off \n when reading ids
char* read_id(FILE* fptr){
    char* c = malloc(sizeof(char)*BUFMAX);
    fgets(c, BUFMAX, fptr);
    int i;
    for (i=0; c[i]!=0; i++){
        if (c[i] == '\n'){
            c[i] = 0;
            break;
        }
    }
    return c;
}


void ast_recursive_read(AST* ast, FILE* fptr, ast_type t){
    //printf("type: %i\n", t);
    char buffer[BUFMAX], buffer2[BUFMAX];
    int i;
    AST* temp, *temp2, *temp3;
    ast_type typearray[8];
    memset(typearray, 0, sizeof(typearray));
    memset(ast, 0, sizeof(AST));
    ast->nodetype = t;
    
    // BIG switch statement on ast type, to handle every case
    // Definitely has some redundancy
    switch (t){
    case Class:
        fgets(buffer, BUFMAX, fptr);
        ast->linenum = atoi(buffer);
        ast->id = read_id(fptr);
        temp = malloc(sizeof(AST));
        
        ast_recursive_read(temp, fptr, Inherits);

        fgets(buffer, BUFMAX, fptr);
        ast->numchild = atoi(buffer) + 1;
        ast->children = malloc(sizeof(AST*) * ast->numchild);
        ast->children[0] = temp;
        for (i = 1; i < ast->numchild; i++){
            ast->children[i] = malloc(sizeof(AST));
            ast_recursive_read(ast->children[i], fptr, Feature);
        }
        
        break;
    case Inherits:
        fgets(buffer, BUFMAX, fptr);
        if (strcmp(buffer, "no_inherits\n") == 0){
            ast->nodetype = No_inherits;
        }
        else{
            ast->nodetype = Inherits;
            fgets(buffer, BUFMAX, fptr);
            ast->linenum = atoi(buffer);
            ast->id = read_id(fptr);
        }
        break;
    case Feature:
        fgets(buffer2, BUFMAX, fptr);
        fgets(buffer, BUFMAX, fptr);
        ast->linenum = atoi(buffer);
        ast->id = read_id(fptr);

        if (strcmp(buffer2, "attribute_no_init\n") == 0){
            ast->nodetype = Attribute_no_init;

            typearray[0] = Type;
            ast_read_children_of_ast_type(ast, fptr, 1, typearray);
        }
        else if (strcmp(buffer2, "attribute_init\n") == 0){
            ast->nodetype = Attribute_init;
            
            typearray[0] = Type;
            typearray[1] = Expr;
            ast_read_children_of_ast_type(ast, fptr, 2, typearray);
            
        }
        else /*if (strcmp(buffer2, "method\n") == 0)*/{
            ast->nodetype = Method;
            fgets(buffer, BUFMAX, fptr);
            ast->numchild = atoi(buffer) + 2;
            ast->children = malloc(sizeof(AST*) * ast->numchild);
            for (i = 0; i < ast->numchild; i++){
                ast->children[i] = malloc(sizeof(AST));
            }
            for (i = 0; i < ast->numchild - 2; i++){
                ast_recursive_read(ast->children[i], fptr, Formal);
            }
            ast_recursive_read(ast->children[ast->numchild-2], fptr, Type);
            ast_recursive_read(ast->children[ast->numchild-1], fptr, Expr);
            
        }
        break;
    
    case Type:
        fgets(buffer, BUFMAX, fptr);
        ast->linenum = atoi(buffer);
        ast->id = read_id(fptr);
        break;
    case Formal:
        fgets(buffer, BUFMAX, fptr);
        ast->linenum = atoi(buffer);
        ast->id = read_id(fptr);

        typearray[0] = Type;
        ast_read_children_of_ast_type(ast, fptr, 1, typearray);
        break;
    case Expr:
        fgets(buffer, BUFMAX, fptr);
        ast->linenum = atoi(buffer);
        
        ast->outtype = read_id(fptr);

        fgets(buffer, BUFMAX, fptr);
        typearray[0] = ast_type_from_buffer(buffer);
        ast_read_children_of_ast_type(ast, fptr, 1, typearray);
        break;
    case Integer:
    case String:
        ast->id = read_id(fptr);
        break;
    case True:
    case False:
        break;
    case Identifier:
        typearray[0] = ID;
        ast_read_children_of_ast_type(ast, fptr, 1, typearray);
        break;
    case ID:
        fgets(buffer, BUFMAX, fptr);
        ast->linenum = atoi(buffer);
        ast->id = read_id(fptr);
        break;
    case Isvoid:
    case Negate:
    case Not:
        typearray[0] = Expr;
        ast_read_children_of_ast_type(ast, fptr, 1, typearray);
        break;
    case New:
        typearray[0] = Type;
        ast_read_children_of_ast_type(ast, fptr, 1, typearray);
        break;
    case While:
    case Plus:
    case Minus:
    case Times:
    case Divide:
    case Lt:
    case Le:
    case Eq:
        typearray[0] = Expr;
        typearray[1] = Expr;
        ast_read_children_of_ast_type(ast, fptr, 2, typearray);
        break;
    case If:
        typearray[0] = Expr;
        typearray[1] = Expr;
        typearray[2] = Expr;
        ast_read_children_of_ast_type(ast, fptr, 3, typearray);
        break;
    case Block:
    case Args:
        fgets(buffer, BUFMAX, fptr);
        ast->numchild = atoi(buffer);
        //List* typelist = newList();
        //for (i = 0; i < ast->numchild; i++){
        //    typearray[i] = Expr;
        //}
        //ast_read_children_of_ast_type(ast, fptr, ast->numchild, typearray);
        ast->children = malloc(sizeof(AST*) * ast->numchild);
        for (i = 0; i < ast->numchild; i++){
            //allocate each child ast
            ast->children[i] = malloc(sizeof(AST));
            //read each child
            ast_recursive_read(ast->children[i], fptr, Expr);
        }
        break;
    case Let:
        fgets(buffer, BUFMAX, fptr);
        ast->numchild = atoi(buffer)+1;
        ast->children = malloc(sizeof(AST*) * ast->numchild);
        for (i = 0; i < ast->numchild-1; i++){
            ast->children[i] = malloc(sizeof(AST));
            fgets(buffer, BUFMAX, fptr);
            ast_recursive_read(ast->children[i], fptr, ast_type_from_buffer(buffer));
        }
        ast->children[ast->numchild-1] = malloc(sizeof(AST));
        ast_recursive_read(ast->children[ast->numchild-1], fptr, Expr);
        break;
    case Let_binding_no_init:
        typearray[0] = ID;
        typearray[1] = Type;
        ast_read_children_of_ast_type(ast, fptr, 2, typearray);
        break;
    case Let_binding_init:
        typearray[0] = ID;
        typearray[1] = Type;
        typearray[2] = Expr;
        ast_read_children_of_ast_type(ast, fptr, 3, typearray);
        break;
    case Self_dispatch:
        typearray[0] = ID;
        typearray[1] = Args;
        ast_read_children_of_ast_type(ast, fptr, 2, typearray);
        break;
    case Dynamic_dispatch:
        typearray[0] = Expr;
        typearray[1] = ID;
        typearray[2] = Args;
        ast_read_children_of_ast_type(ast, fptr, 3, typearray);
        break;
    case Static_dispatch:
        typearray[0] = Expr;
        typearray[1] = Type;
        typearray[2] = ID;
        typearray[3] = Args;
        ast_read_children_of_ast_type(ast, fptr, 4, typearray);
        break;
    case Assign:
        typearray[0] = ID;
        typearray[1] = Expr;
        ast_read_children_of_ast_type(ast, fptr, 2, typearray);
        break;
    case Case:
        typearray[0] = Expr;
        typearray[1] = Case_block;
        ast_read_children_of_ast_type(ast, fptr, 2, typearray);
        break;
    case Case_block:
        fgets(buffer, BUFMAX, fptr);
        ast->numchild = atoi(buffer);
        ast->children = malloc(sizeof(AST*) * ast->numchild);
        for (i = 0; i < ast->numchild; i++){
            ast->children[i] = malloc(sizeof(AST));
            ast_recursive_read(ast->children[i], fptr, Case_inner);
        }
        break;
    case Case_inner:
        typearray[0] = ID;
        typearray[1] = Type;
        typearray[2] = Expr;
        ast_read_children_of_ast_type(ast, fptr, 3, typearray);
        break;
    case Internal:
        ast->id = read_id(fptr);
        break;
    }
    //printf("d: %i\n", t);
    return;
}

void ast_read_children_of_ast_type(AST* ast, FILE* fptr, int numchild, ast_type* type){
    int i;
    ast->numchild = numchild;
    //allocates the pointer list
    ast->children = malloc(sizeof(AST*) * numchild);
    for (i = 0; i < numchild; i++){
        //allocate each child ast
        ast->children[i] = malloc(sizeof(AST));
        //read each child
        ast_recursive_read(ast->children[i], fptr, type[i]);
    }
}


void ast_print(AST* ast){
    int i;
    // Prints to file case by case depending on the ast_type
    // Probably has some redundancies
    if (!ast) return;
    if (ast->nodetype == Internal){
        fprintf(outf, "%i\n", ast->linenum);
        fprintf(outf, "%s\n", ast->outtype);
        fprintf(outf, "internal\n");
        fprintf(outf, "%s\n", ast->id);
        return;
    }
    if (ast->nodetype == Expr){
        fprintf(outf, "%i\n", ast->linenum);
        ast_print(ast->children[0]);
        return;
    }
    if (ast->outtype && 
        ast->nodetype != Type && 
        ast->nodetype != Case_inner &&
        ast->nodetype != Case_block){
        fprintf(outf, "%s\n", ast->outtype);
    }
    

    switch(ast->nodetype){
        case Root:
        case Block:
        case Args:
        case Case_block:
            if (ast->nodetype == Block) fprintf(outf, "block\n");
            fprintf(outf, "%i\n", ast->numchild);
            for (i = 0; i < ast->numchild; i++){
                ast_print(ast->children[i]);
            }
            return;
        case Class:
            i = 0;
            fprintf(outf, "%i\n", ast->linenum);
            fprintf(outf, "%s\n", ast->id);
            fprintf(outf, "%s\n", string_from_ast_type(ast->children[i]->nodetype));
            if (ast->children[i]->nodetype == Inherits){
                fprintf(outf, "%i\n", ast->children[i]->linenum);
                fprintf(outf, "%s\n", ast->children[i]->id);
            }

            i++;
            fprintf(outf, "%i\n", ast->numchild-1);
            for (; i < ast->numchild; i++){
                ast_print(ast->children[i]);
            }
            return;
        case ID:
        case Type:
            fprintf(outf, "%i\n", ast->linenum);
            fprintf(outf, "%s\n", ast->id);
            return;
        case Case_inner:
            for (i = 0; i < ast->numchild; i++){
                ast_print(ast->children[i]);
            }
            return;
        case Formal:
            fprintf(outf, "%i\n", ast->linenum);
            fprintf(outf, "%s\n", ast->id);
            ast_print(ast->children[0]);
            return;
        case Let:
            fprintf(outf, "let\n");
            fprintf(outf, "%i\n", ast->numchild-1);
            for (i = 0; i < ast->numchild; i++){
                ast_print(ast->children[i]);
            }
            return;
    }

    fprintf(outf, "%s\n", string_from_ast_type(ast->nodetype));
    switch(ast->nodetype){
    
    case Integer:
    case String:
        fprintf(outf, "%s\n", ast->id);
        break;
    case Attribute_init:
    case Attribute_no_init:
    case Method:
        fprintf(outf, "%i\n", ast->linenum);
        fprintf(outf, "%s\n", ast->id);
        if (ast->nodetype == Method){
            fprintf(outf, "%i\n", ast->numchild - 2);
        }
    default:
        for (i = 0; i < ast->numchild; i++){
            ast_print(ast->children[i]);
        }
    }
}

void throw_error(int linenum, const char* message){
    printf("ERROR: %i: Type-Check: %s\n", linenum, message);
    exit(2);
}


ast_type ast_type_from_buffer(const char* buffer){
    // just a long list of each type, probably would be better to use a hash map but those don't come with C
    if (strcmp(buffer, "assign\n") == 0){
        return Assign;
    }
    else if (strcmp(buffer, "dynamic_dispatch\n") == 0){
        return Dynamic_dispatch;
    }
    else if (strcmp(buffer, "static_dispatch\n") == 0){
        return Static_dispatch;
    }
    else if (strcmp(buffer, "self_dispatch\n") == 0){
        return Self_dispatch;
    }
    else if (strcmp(buffer, "if\n") == 0){
        return If;
    }
    else if (strcmp(buffer, "while\n") == 0){
        return While;
    }
    else if (strcmp(buffer, "block\n") == 0){
        return Block;
    }
    else if (strcmp(buffer, "let\n") == 0){
        return Let;
    }
    else if (strcmp(buffer, "let_binding_no_init\n") == 0){
        return Let_binding_no_init;
    }
    else if (strcmp(buffer, "let_binding_init\n") == 0){
        return Let_binding_init;
    }
    else if (strcmp(buffer, "case\n") == 0){
        return Case;
    }
    else if (strcmp(buffer, "new\n") == 0){
        return New;
    }
    else if (strcmp(buffer, "isvoid\n") == 0){
        return Isvoid;
    }
    else if (strcmp(buffer, "plus\n") == 0){
        return Plus;
    }
    else if (strcmp(buffer, "minus\n") == 0){
        return Minus;
    }
    else if (strcmp(buffer, "times\n") == 0){
        return Times;
    }
    else if (strcmp(buffer, "divide\n") == 0){
        return Divide;
    }
    else if (strcmp(buffer, "negate\n") == 0){
        return Negate;
    }
    else if (strcmp(buffer, "lt\n") == 0){
        return Lt;
    }
    else if (strcmp(buffer, "le\n") == 0){
        return Le;
    }
    else if (strcmp(buffer, "eq\n") == 0){
        return Eq;
    }
    else if (strcmp(buffer, "not\n") == 0){
        return Not;
    }
    else if (strcmp(buffer, "identifier\n") == 0){
        return Identifier;
    }
    else if (strcmp(buffer, "integer\n") == 0){
        return Integer;
    }
    else if (strcmp(buffer, "string\n") == 0){
        return String;
    }
    else if (strcmp(buffer, "true\n") == 0){
        return True;
    }
    else if (strcmp(buffer, "false\n") == 0){
        return False;
    }
    else if (strcmp(buffer, "internal\n") == 0){
        return Internal;
    }
}

const char* string_from_ast_type(ast_type t){
    // Long list
    switch(t){
        case Attribute_init:
            return "attribute_init";
        case Attribute_no_init:
            return "attribute_no_init";
        case Method:
            return "method";
        case Inherits:
            return "inherits";
        case No_inherits:
            return "no_inherits";
        case Assign:
            return "assign";
        case Dynamic_dispatch:
        return "dynamic_dispatch";   
        case Static_dispatch:
        return "static_dispatch";
        case Self_dispatch:
        return "self_dispatch";
        case If:
        return "if";
        case While:
        return "while";
        case Block:
        return "block";
        case Let:
        return "let";
        case Let_binding_no_init:
        return "let_binding_no_init";
        case Let_binding_init:
        return "let_binding_init";
        case Case:
        return "case";
        case New:
        return "new";
        case Isvoid:
        return "isvoid";
        case Plus:
        return "plus";
        case Minus:
        return "minus";
        case Times:
        return "times";
        case Divide:
        return "divide";
        case Negate:
        return "negate";
        case Lt:
        return "lt";
        case Le:
        return "le";
        case Eq:
        return "eq";
        case Not:
        return "not";
        case Identifier:
        return "identifier";
        case Integer:
        return "integer";
        case String:
        return "string";
        case True:
        return "true";
        case False:
        return "false";
    }
    return "null";
}

List* list_new(){
    List* list = malloc(sizeof(List));
    list->length = 1;
    list->data = malloc(sizeof(char*) * 1);
    memset(list->data, 0, sizeof(char*) * 1);
    return list;
}

int list_is_in(List* list, char* id){
    int i;
    char** data = list->data;

    for (i = 0; data[i] != NULL; i++){
        if (strcmp(data[i], id) == 0){
            return 1;
        }
    }
    return 0;
}

void list_add_id(List* list, char* id){
    int i;
    if (list_is_in(list, id)){
        return;
    }
    for (i = 0; list->data[i] != NULL; i++){
    }
    if (i >= list->length-1){
        char** tempdata = malloc(sizeof(char*) * list->length * 2);
        memset(tempdata, 0, sizeof(char*) * list->length * 2);
        memcpy(tempdata, list->data, sizeof(char*) * list->length);
        free(list->data);
        list->data = tempdata;
        list->length = list->length*2;
    }
    list->data[i] = id;
}

void list_free(List* list){
    free(list->data);
    free(list);
}

void list_append(List* list, List* newlist){
    int i;
    char** data = newlist->data;
    for (i = 0; data[i] != NULL; i++){
        list_add_id(list, data[i]);
    }
}