#ifndef __MAIN_H__
#define __MAIN_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFMAX 128 //buffer size for reading lines in the ast

// Each node in my internal AST has an ast_type
typedef enum {
    Root,
    Inherits,
    No_inherits,
    Class,
    Feature,
    Attribute_no_init,
    Attribute_init,
    Method,
    Type,
    Constant,
    Formal,
    Expr,
    Assign,
    Dynamic_dispatch,
    Static_dispatch,
    Self_dispatch,
    Args,
    If,
    While,
    Block,
    Let,
    Let_binding_no_init,
    Let_binding_init,
    Case,
    Case_block,
    Case_inner,
    New,
    Isvoid,
    Plus,
    Minus,
    Times,
    Divide,
    Negate,
    Lt,
    Le,
    Eq,
    Not,
    Identifier,
    ID,
    Integer,
    String,
    True,
    False,
    Internal
}ast_type;

// The structure of the internal AST
typedef struct AST_S{
    int linenum;            // Current linenumber
    ast_type nodetype;      // Type of the node
    char* id;               // Id holds things like string/int constants or identifier names
    int numchild;           // Number of children
    struct AST_S** children;// List of pointers to each child
    char* outtype;          // For emitting types
}AST;

// AST* ast: The current node to read into
// FILE* fptr: Passing around the file pointer
// ast_type t: The type of the current node
// If a node has children, it calls ast_recursive_read for each child
void ast_recursive_read(AST* ast, FILE* fptr, ast_type t);

// Helper function for ast_recursive_read
void ast_read_children_of_ast_type(AST* ast, FILE* fptr, int numchild, ast_type* type);

// Helper functions to convert between the string representations of node types and my internal enum ast_type
ast_type ast_type_from_buffer(const char* buffer);
const char* string_from_ast_type(ast_type t);

// Recursively prints the ast to the output file
void ast_print(AST* ast);

// Called whenever a typecheck error is detected and exits the program
void throw_error(int linenum, const char* message);
#endif