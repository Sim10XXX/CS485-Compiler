# CS485-Compiler

## Assignment Description

## Project Description

Some optimizations are more convienient when the program is in CFG form

To implement CFG's (Control Flow Graph), I decided to make this struct
```
typedef struct CFG_S{
    char* label;
    TAC_List* taclist;
    struct CFG_S* branch1;
    struct CFG_S* branch2;
    int backedge;
    int casebranches;              //reserved for case statements
    struct CFG_S** casebranchlist; //reserved for case statements
    Livevars* livetop; //dataflow for dead variables
    Map* cmap; //For attribute knowledge
}CFG;
```

`taclist` is the basic block in TAC form, and since non-case statements have at most 2 branches, each CFG has 2 possible branches `branch1` and `branch2`. `backedge` is a bool that indicates if the block is at the end of a loop body, which I thought would be important but I don't think I ended up using it, and `casebranches` is for a case statement since it can have an arbitrary number of branches. `cmap` is just a pointer to the classmap and `livetop` is used for liveness analysis. I implemented translation functions between pure TAC and CFG tac since liveness analysis requires the CFG form but my codegen works on pure TAC.

I have two liveness analyses in my optimizer: one for TAC and one for pure assembly. The one for TAC works on the CFG form, and initializes every line of TAC (from bottom to top) to be marked as dead. It works on the global scale (for entire methods) by keeping track of a list of live temporaries. Temporaries get added to this list if they are involved in dispatch/new, or are the return value for the method. Temporaries also get added if an already live temporary depends on its value, and temporaries get removed from livevars if their value is overwritten with a constant. When a block is done, the CFG's livetop is updated so that the live variables can propagate beyond the basic block, and the process is looped until no more changes are made. Every line marked as dead can be safely deleted.

For my pure assembly analysis, I only did an unreachable analysis because 
1) I already did dead code analysis in the TAC
2) That sounds horrible to implement for assembly
3) Unreachable analysis on its own brings a decent amount of binary size improvement

I made an IR to store lines of assembly and attach a variable to each line for if it is unreachable or not, initialize every line to unreachable, then "color" in everything that is reachable.
Starting at the "Start:" label, I color every line I come across. If there is a branch (conditional jmp or a method call), I recursively take the branch, then continue down. If there is a ret, I stop.
This sounds simple but was painful to implement because of the many cases I needed to handle (calls to libc, dynamic dispatch and vtable shenanigans), but it is able to remove a lot of fluff from programs. Like if no string is ever used, all the string built-ins and things like string..new all get removed.

I also added the unboxing int optimization, since ints are just not treated as objects anyways in Cool, since setting an int variable always creates a new int so no reference aliasing shenanigans happens anyways. If UnboxInt flag is checked in my compiler, it skips the dereferencing steps whenever working on Int operations, and uses the raw value of whatever that location was. I also had to modify all the Cool internal methods to stop dereferencing the ints (instant segfault) by adding flags in my internaldata file that indicated that a line after ##UnboxInt should be removed if UnboxInt flag is true, and ##!UnboxInt should be removed if UnboxInt flag is false, so that my compiler still works in both modes.

I also added static execution to my compiler. If the tac contains no user input, then that means that the Cool program is completely deterministic and will always yield the same output. So in that case, I generate the code as usual, then I use a system call to gcc to make the binary, then another sys call to execute and store output into a temporary file. Then I take the contents of that temporary file and generate assembly based on a "hello world" printing c program that literally just prints the output. I needed to handle some corner cases with escaping characters like '\\', '\"', and '\n', and used a call to printf a "%s" to avoid shenanigans with %'s 

I wanted to add register allocation at the time of writing this, but my WSL decided to just stop working for some reason and I can't run my code without putting in more effort than I can right now, so no register allocation only memory :P



benchmark1.cl:
the key feature here is that there is no user input, but a lot of different integer printing nonsense, which is perfect for static execution

benchmark2.cl:
This benchmark takes an input, and performs integer operations in a loop many times before printing, in a manor that is nontrivial to optimize (in loop body, attributes, using user input). Integer unboxing should save a lot of instructions in this program.
