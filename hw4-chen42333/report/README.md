# hw4 report

|||
|-:|:-|
|Name|陳芷萱|
|ID|110550029|

## How much time did you spend on this project

About 20 hours.

## Project overview

- To manage the symbol tables, SymbolTable.[ch]pp are added and the structure is refered to the "Symbol Table Construction" section in the spec, and the type of errors is also defined here. The symbol manager manages all the current tables (stored in a stack), and each table contains several symbol entries (stored in a vector). 
    - To find a symbol in a table, the table will do linear search in the vector, and return the entry once it finds the symbol. If the symbol do not exist, an entry object without initializing will be returned. We can check whether the name variable of the entry is NULL to check whether the symbol is in the table.
    - The symbol manager will find all the current existing tables to check whether a specified symbol has been declared. If the symbol doesn't exist in the symbol table at the top of the stack, the table will be popped and pushed into a buffer (implemented with a stack), and then find whether the symbol is in the next table. After the symbol is found or the all the tables have been checked, the tables in the buffer will be popped and pushed back to the stack. And thus the cloest declaration will be returned.
- To print the source code, the source file name will be given to the semantic analyzer when it is declared, and the action of its constructor is to read all the lines of source code and store them into an array. 
- When the scanner detects the pseudocomment ```//&D```, it will set the variable ```opt_table``` to the corresponding value, and when a symbol table is to be popped, it will check ```opt_table``` to determine whether it should print its information.
- Once a semantic error is detect, the corresponding error information will be stored in the symbol manager. And the action of the destructor of the semantic analyzer is to print all the error messsages depends on the error types.
- To determine the type of an expression: 
    - Binary Operator: Get the type of the left and right expresssions by recursion, and then check whether their types are valid to the operator. If their types are invalid, return "error"; if valid, return the corresponding type from the rule of the operator.
    - Unary Operator: Similar as binary operator, but only one expression should be checked.
    - Variable Reference: Get the type of the variable from symbol tables first and check the kind is valid. If it is valid, check how many times it is indexed. Every time it is indexed, the foremost index will be removed (ex: real [3][2] -> real [2]).
    - Function Invocation: Check if it is valid (same as the semantic analysis of function invocation). If it is invalid, return "error"; if it is valid, get and return the return type of the function from symbol tables.
    - Constant Value: Return the type information stored in the node.
- Use the variable ```default_kind``` to determine the kind of symbol, and the default is variable.
    - Before a for node traversing its children, it will set ```default_kind``` to loop_var. Since there will be only one loop variable, ```default_kind``` will be set back to variable right after the loop variable inserted to the table. If the insertion is failed due to redeclaration, ```default_kind``` will be set back to variable by the assignment node (the assignment node will check whether ```default_kind``` is loop_var).
    - Before a function node traversing its children, it will set ```default_kind``` to parameter and set back to variable after traversing (for function declaration). The compound statement node will also set ```default_kind``` back to variable (for function definition).
    - When a variable node inserts a symbol to the table, it will check whether it has a child (constant value node) to determine the symbol type (variable or constant).
- To check if it has to push a new scope, two variables ```enterFunc``` (default is false) and ```newTable``` (default is true) are used. Before a function node traversing its children, it will set ```enterFunc``` to true and set both of them back to the default value after traversing. When a compound statement node finds that both ```enterFunc``` and ```newTable``` are true, it means that it is the first level compound statement of a function, so it won't push a new symbol table, and it will set both of them to false; when a compound statement node finds that ```enterFunc``` is false, it means that it is not the first level compound statement of a function, so it will set ```newTable``` to true and push a new symbol table. ```newTable``` will be back up before traversing the children, and it will use this back up to determine whether it should pop the current symbol table after traversing the children.
- Because a loop variable cannot be redeclared, a vector of string will record every currently exist loop variables. Each time a symbol is to be inserted, it has to check the symbol name doesn't be the same as the name of any currently exist loop variables. The last element of the vector will be removed when a for node pops its symbol table.
## What is the hardest you think in this project

I think the hardest part is to determine the kind of symbols and how to let a node know whether it should do semantic analysis if there are already some errors. I took a lot of time to come up with the solution of the former and solve the latter by test again and again.

## Feedback to T.A.s

I'm still a little confused about whether a node should do semantic analysis if there are already some errors. The description in the spec can be more clear.
