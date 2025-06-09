//#include "main.h"
#include "map.h"
#include "typecheck.h"

// Output file
FILE* outf;

int main(int argc, char* argv[]) {
    if (argc != 2) return 1;
    FILE *fptr;
    AST* ast = malloc(sizeof(AST)); //root of the ast
    //Open input file
    fptr = fopen(argv[1], "r"); 
    if (!fptr){
        printf("error opening file\n");
        return 1;
    }
    char namebuffer[BUFMAX]; //name of the output file
    //changing the extention from .cl-ast to .cl-type
    memset(namebuffer, 0, sizeof(char)*BUFMAX);
    memcpy(namebuffer, argv[1], strlen(argv[1])-3);
    strcat(namebuffer, "type");

    int i;
    char buffer[BUFMAX];
    fgets(buffer, BUFMAX, fptr); //Read first line of input
    ast->linenum = 0; //root is on line 0
    ast->numchild = atoi(buffer); // First line of input gives number of classes, each class is a child of Root
    ast->nodetype = Root;
    ast->id = NULL;
    ast->children = malloc(sizeof(AST*) * ast->numchild); // Allocate space for each child ast (each class)
    for (i = 0 ; i < ast->numchild ; i++){
        ast->children[i] = malloc(sizeof(AST));
        ast_recursive_read(ast->children[i], fptr, Class); // Recursively read each class
    }
    
    // Initialize each map that we need
    Map* attributeMap = map_initialize(ast->numchild, attribute_map);
    Map* classMap = map_initialize(ast->numchild, class_map);
    Map* methodMap = map_initialize(ast->numchild, method_map);
    Map* implementationMap = map_initialize(ast->numchild, implementation_map);
    Map* parentMap = map_initialize(ast->numchild, parent_map);

    // Insert each class into the maps.
    for (i = 0 ; i < ast->numchild ; i++){
        map_insert_ast(attributeMap, ast->children[i]);
        map_insert_ast(methodMap, ast->children[i]);
        map_insert_ast(parentMap, ast->children[i]);
    }

    // Bank will hold the result of typecheck_parent_map_cycle
    char** bank;
    int bankc = 1;
    for (i = 0; parentMap[i].id != NULL; i++){
        bankc++;
    }
    typecheck_parent_map_illegal_inherits(parentMap, attributeMap);
    bank = typecheck_parent_map_cycle(parentMap);

    // Build the classmap and implementationmap in topological order, so that inherited attributes/methods can be inherited
    for (i = 0; i < bankc; i++){
        map_build_class_map(classMap, attributeMap, parentMap, bank[i]);
        map_build_implementation_map(implementationMap, methodMap, parentMap, bank[i]);
    }

    typecheck_map(classMap);
    typecheck_map(methodMap);
    typecheck_main_method_map(methodMap);
    
    typecheck_expr_from_root(classMap, implementationMap, parentMap, ast);
    
    // Open outfile and print everything
    outf = fopen(namebuffer, "w");
    map_print(classMap, outf);
    map_print(implementationMap, outf);
    map_print(parentMap, outf);
    ast_print(ast);

    fclose(fptr);
    fclose(outf);
    return 0;
}

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
    char buffer[BUFMAX], buffer2[BUFMAX];
    int i;
    AST* temp, *temp2, *temp3;
    ast_type typearray[128] = { 0 };
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
        for (i = 0; i < ast->numchild; i++){
            typearray[i] = Expr;
        }
        ast_read_children_of_ast_type(ast, fptr, ast->numchild, typearray);
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
    }
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
