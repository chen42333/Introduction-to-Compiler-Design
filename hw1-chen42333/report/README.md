# hw1 report

|Field|Value|
|-:|:-|
|Name|陳芷萱|
|ID|110550029|

## How much time did you spend on this project

About 5-6 (code) + 4 (report) hours.

## Project overview

This project generate a scanner by using Lex to convert the file ```scanner.l``` to the C file ```scanner.c``` and then compile it to a binary file. There are also some testcases to test the correctness of this scanner.

The file ```scanner.l``` can be divided into three parts: definitions, transition rules and user subroutines. I didn't modify the user subroutines part from the provided template but just added some definitions of the tokens and corresponding transition rules. 
- Delimiters, Arithmetic/Relational/Logical Operators: 
    - Definition: OR those characters or words listed in the spec.
    - Transition Rules: Print out the characters or words without any attribute.
- Reserved Words: 
    - Definition: OR those words listed in the spec.
    - Transition Rules: Because it has to add ```KW``` before the reserved words when prints out the token, but do not have to add anything when prints out the source code, so I created a string ```tmp``` to save the reseved words with the ```KW``` prefix and then called ```LIST_TOKEN(tmp)``` to print out the token in specific format without modifying ```yytext```.
- Identifiers: 
    - Definition: The identifier must begin with a letter => ```[a-zA-Z]```, and be followed some by letters or digits or nothing => ```[a-zA-Z][a-zA-Z0-9]*```
    - Transition Rules: Print out the characters or words with ```id``` attribute.
- Integer Constants:
    - Definition: 
        - Decimal Numbers: Because a positive decimal number cannot begin with 0 and the following degits can be any numbers between 0~9 => ```decimal_pos = [1-9][0-9]*```. A decimal number can be positive or 0 => ```0|{decimal_pos}```.
        - Octal Numbers: When a number begins with 0 and all of its digits are between 0~7, it is an octal number => ```0[0-7]*```.
    - Transition Rules: To make sure ```0``` will be recognize as a decimal integer, the transition rule of decimal digits must be placed before octal digits'. Both of them just print out the numbers with ```integer``` or ```oct_integer``` attributes.
- Floating-Point Constants:
    - Definition: A floating-point number must begin with a decimal number as its integer part => ```{decimal}```, and be followed by a point and some numbers between 0~9 without redundant 0s => ```{decimal}\.[0-9]*[1-9]```. However, it can be ended with a 0 if it only has one decimal place => ```({decimal}\.[0-9]*[1-9])|({decimal}\.0)```.
    - Transition Rules: Print out the numbers with ```float``` attributes.
- Scientific Notations:
    - Definition: The coefficient of a scientific notation must be a nonzero real number, which means that it can be a positive integer with ```.0``` or not => ```{decimal_pos}(\.0)```, or a floating-point number whose last decimal place is not 0 => ```{decimal}\.[0-9]*[1-9]```. The coefficient will be followed by a character 'e' or 'E' => ```[e|E]```, and then the exponent, which is a decimal number with a sign or not =>```(({decimal}\.[0-9]*[1-9])|({decimal_pos}(\.0)?))[e|E][-+]?{decimal}```.
    - Transition Rules: Print out the numbers with ```scientific``` attributes.
- String Constants: 
    - Definition: 
        - ```string```: Strings without any double quote or newline. Any sequence of characters wrapped by two double quotes should be recognize as a string => ```\"[^\"]*\"```
        - ```string_2```: Strings with some double quotes. Because the double quotes will be pairs of consecutive double quotes except for the first and the last ones, it looks like some basic strings (```string```) concatenate together => ```{string}+```.
    - Transition Rules: 
        - ```string_2```: As the ways to deal with reserved words, create a new string ```tmp``` to save the string without the first and last double quotes and remove one double quotes in any two consecutive double quotes, and then print out ```tmp``` with ```string``` attribute.
        - ```string```: If a sequence of characters matches the format of ```string```, it will also matches the format of ```string_2```, and it can be processed in the same way as ```string_2```. So it doesn't need to create another transition rule for ```string```. 
- Whitespace: 
    - Definition: Blank spaces or tabs.
    - Transition Rules: These have to be discarded, so just call ```LIST_SOURCE``` to append them to the current line text buffer and doesn't have to do anything else.
- Comments: 
    - Definition:
        - C-style: No definition was added but create a state ```COMMENT``` in the definition section.
        - C++-style: A line of text starts with ```//``` => ```"//".*```.
    - Transition Rules: 
        - C-style: 
            - When there is a ```/*``` pattern, it means that the following text is a C-style comment, so change the state from ```INITIAL``` to ```COMMENT```.
            - Anything in a comment should be discarded, so if the current state is ```COMMENT```, just list the source code and doesn't have to do anything else unless there is a ```*/``` pattern.
            - When there is a ```*/``` pattern and the current state is ```COMMENT```, it means that the comment is ended, so change the state to ```INITIAL```.
        - C++-style: It has to be discarded, so just call ```LIST_SOURCE``` to append it to the current line text buffer and doesn't have to do anything else.
- Pseudocomments:
    - Definition: A line of text starts with ```//&S+```,  ```//&S-```, ```//&T+``` or ```//&T-``` => ```"//&S+".*```, ```"//&S-".*```, ```"//&T+".*``` and ```"//&T-".*```.
    - Transition Rules: Set the corresponding ```opt_src``` or ```opt_tok``` and append them to the current line text buffer.

## What is the hardest you think in this project

I think the hardest part is to recognize comments. Although I found that there is example code of comments part on the lecture ppt, I still took a little bit of time to review how to set the states and what are their functions. 

Besides, in the strings and identifiers part, the token and the source code that have to be print out have a few diferences: the source code have to contain ' " ' or "KW" while the tokens don't. I added a macro and modified ```yytext``` to fulfill the requirments at the beginning, but changed the method because I thought it's better to use the provided macro and not to modify ```yytext```. After I choosing the alternative way to implement, I got some weird output that made the process terminated, and I found that I forgot to put the character '\0' at the end of ```tmp``` , which caused the error.

## Feedback to T.A.s

> Good. ⭐️⭐️⭐️⭐️⭐️
