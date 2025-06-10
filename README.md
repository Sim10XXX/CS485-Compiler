# CS485-Compiler

## Assignment Description

The main goal is to take the PA2 output, and generate assembly code that can be executed, following the operational semantics of Cool such that your compiler's output program behaves the same way as the reference compiler's output (from the outside)

Full details can be found here: https://kelloggm.github.io/martinjkellogg.com/teaching/cs485-sp25/projects/pa3.html

## Project Description

The overall design of my compiler is to convert the input ast into three address code (TAC), then translate that TAC into executable assembly.
I have made heavy use of the reference compiler, and copied all of its builtin functions. Everything I copied can be found in the "internaldata" file.

"tac.c" features the conversion of the AST into TAC, which is done recursively because of the recursive nature of the AST. Any non trivial AST statement to convert (so If, While, Let, Case) is generally handled by inserting some prologue TAC statements, then recursively converts the inner expression, and then adds epilogue TAC statements, utilizing return values and the parameter "returnvar" to map temporary variables correctly

I have decided to create some TAC statements to be able to handle Case, so that the translation to assembly can be made 1:1 without any extra shenanigans. More specifically, I create one "caseon t$x" to mark the start of a case, an "esac" to mark the end, and in-between statements for each type that needs to be considered (following Cool's case semantics) and where the code should jump if there is a hit "case t$2 : Main jmp Main_main_2". In assembly, "caseon" gets translated into the code that checks for void, each individual "case" is the code to check for the typetags, jmp if hit and fallthrough if miss (the jmps goto AFTER the "esac", and each sub-expression is set to jmp to a label that is placed after all sub-expressions), and eventually if the code falls through all cases,"esac" contains the "case without matching branch" error.

For ease of implementation (which is the PA3 mantra), my assembly code allocates stack space for each temporary found in the TAC (for that particular method/attribute), and so an instance of t$4 is basically translated to "get data from the location rbp - 4*8"

I've also decided to abstract away the specific register certian operations are using, anticipating that I would want some kind of register allocation in the future, so my "assembly.c" features the specific registers I am using as global variables.


These test cases are focused on covering aspects of Cool that are not present in your average Cool program

test1.cl:	Shows how a case on "Object" can be used as a default case, and how the case always picks the closest ancestor

test2.cl:	Division by zero

test3.cl:	Dispatch on void

test4.cl:	Specifically tests static dispatch
