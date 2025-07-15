# hw2 report

|||
|-:|:-|
|Name|陳芷萱|
|ID|110550029|

## How much time did you spend on this project

About 4 hours.

## Project overview

First, modify the scanner to let it return every token except whitespace, comment and pseudocomment, and then define these token as terminal in the declaration part with ```%token```. 

Since a .p file is a program in P language, define ```Program``` as the start symbol, and write the grammars of functions, data declaration and statements from their definition in the spec.

- A program is either a program unit, a function declaration or a function definition.
- The functions part in a program unit is a combination of zero or some function declaration and function definition.
- The function declaration without the semicolon is equal to a procedure plus a scalar type.
- A constant can be either an integer, an octal integer, a floating number, a scientific notation, a string, true or false (boolean).
- The statements part of a compound statement is a combination of zero or some simple statements, conditional statements, while statements, for statements, return statements or function calls.
- A variable reference is an identifier with zero or some brackets that contain an expression in each.
- A conditional statement can have the else part or not.

As for the expression part, define the precedence of the operations in the definition part first. There is a terminal ```UMINUS``` which is defined last with ```%noassoc``` and represents negative (```-``` can represents negative and subtraction while they have different precedence). Use ```%prec UMINUS``` to mark the production in which ```-``` represents negative to give this operation the highest precedence.
 

## What is the hardest you think in this project

I think the expression part is hardest, and the grammar of the other part can be directly traslated from the description in the spec basically. Although there are example code in the slide, but I still spent some time to google the detail and realize it.

## Feedback to T.A.s

 Good practice. 
