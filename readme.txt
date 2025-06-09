In the source code, each header file is well commented, and each c file has some important comments
Brief for each file:

-- main.c/h 
Contains the runner code as well as everything to do with ASTs
The structure of the AST:
typedef struct AST_S{
    int linenum;            // Current linenumber
    ast_type nodetype;      // Type of the node
    char* id;               // Id holds things like string/int constants or identifier names
    int numchild;           // Number of children
    struct AST_S** children;// List of pointers to each child
    char* outtype;          // For emitting types
}AST;

Main has the code to recursively read the AST from file, and to print AST to file

-- map.c/h
Maps are stored as a null-terminated array of type Map, where each map is a mapping of class id to some data
typedef struct {
    char* id;       // class id
    int linenum;    // line number for the class
    MapType maptype;// map type
    int numchild;   // number of elements in data
    void* data;     // type of data depends on map type
                        // if attribute_map || class_map
                            // data = Attribute*
                        // if method_map || implementation_map
                            // data = ImpMethod*
                        // if parent_map
                            // data = char*
} Map;
map.c/h has code for retrieving the information from these maps by id,
and defines each type of data

There are 5 different map types:
typedef enum {
    attribute_map,      // map of class id -> defined attributes
    class_map,          // map of class id -> defined/inherited attributes
    method_map,         // map of class id -> defined methods
    implementation_map, // map of class id -> defined/inherited methods
    parent_map          // map of class id -> parent class id
} MapType;


-- typeEnvironment.c/h
defines the type environments used when typechecking expressions.
Since expression typechecking is recursive, defining a new scope is as easy as

temp = type_environment_new();
newgamma = type_environment_cat(temp, oldgamma);
typecheck_expr(newgamma ...);
free temp, newgamma;

When retrieving data from a type environment, it always returns the first match,
so concatenating new environments onto old ones will match the new bindings first


-- typecheck.c/h
Contains all kinds of typecheck code, such as inheritance cycle, duplicate class names
duplicate method/attribute names, etc.
Also contains typecheck_expr, which basically defines the Type Check Rules section of the CRM
and also sets the outtype for each expression to be emitted in the annotated AST



Test cases:
good.cl: showcases shadowing in Cool by redefining x a bunch of times in interesting ways but still typechecking

bad1.cl: shows that B does not conform to SELF_TYPE(B), also made to check correct line number
bad2.cl: checks conformance of types when using static dispatch
bad3.cl: checks that both the LUB and type environments work for case statement